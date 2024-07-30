/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "libwacomwrapper.h"

#include "logging.h"

#include <memory>

extern "C" {
#include <libwacom/libwacom.h>
}

namespace Wacom
{

static int skipWheelButtons(int button)
{
    // skip buttons 4-7, which correspond to vertical/horizontal wheel up/down events
    if (button > 3) {
        return button + 4;
    } else {
        return button;
    }
}
#ifndef LIBWACOM_EVDEV_MISSING
static int convertEvdevToXsetwacomButton(int evdevCode);
#endif // LIBWACOM_EVDEV_MISSING

libWacomWrapper::libWacomWrapper()
{
    db = libwacom_database_new();
}

libWacomWrapper &libWacomWrapper::instance()
{
    static libWacomWrapper instance;
    return instance;
}

libWacomWrapper::~libWacomWrapper()
{
    libwacom_database_destroy(db);
}

bool libWacomWrapper::lookupTabletInfo(int tabletId, int vendorId, TabletInformation &tabletInfo)
{
    qCDebug(COMMON) << "LibWacom lookup for" << tabletId << vendorId;
    auto errorDeleter = [](WacomError *e) {
        libwacom_error_free(&e);
    };
    std::unique_ptr<WacomError, decltype(errorDeleter)> error(libwacom_error_new(), errorDeleter);
    std::unique_ptr<WacomDevice, decltype(&libwacom_destroy)> device(libwacom_new_from_usbid(db, vendorId, tabletId, error.get()), &libwacom_destroy);

    if (!device) {
        qCInfo(COMMON) << "LibWacom lookup failed:" << libwacom_error_get_message(error.get());
        return false;
    }

    // TODO: libWacom returned button layouts don't make much sense (yet)
    // because they use letters instead of numbers
    const auto layoutFileName = libwacom_get_layout_filename(device.get());
    if (layoutFileName) {
        tabletInfo.set(TabletInfo::ButtonLayout, QString::fromLatin1(layoutFileName));
    }

    // Seems like there is no model or vendor names in libwacom
    // TODO: are these properties even used anywhere?
    tabletInfo.set(TabletInfo::CompanyName, QString());
    tabletInfo.set(TabletInfo::TabletModel, QString());
    // TODO: Seems like there's no info about the pad wheel in libwacom
    // Only a couple of tablets have the wheel anyway, so it probably can be hacked around
    tabletInfo.set(TabletInfo::HasWheel, false);
    // TODO: Returns more detailed information than we expect,
    // current LED code is broken anyway so this should be untangled later
    int numStatusLeds = 0;
    int numLedGroups = 0;

    // TODO: Following code might not work very well with Cintiq 24HD
    // because of how we currently handle StatusLEDs property (we expect 0, 4 or 8 leds)
    // and Cintiq 24HD supposedly has 3 + 3 leds
    const WacomStatusLEDs *ledGroups = libwacom_get_status_leds(device.get(), &numLedGroups);
    for (int i = 0; i < numLedGroups; i++) {
        int groupModes = 0;
        switch (ledGroups[i]) {
        case WACOM_STATUS_LED_RING:
            groupModes = libwacom_get_ring_num_modes(device.get());
            break;
        case WACOM_STATUS_LED_RING2:
            groupModes = libwacom_get_ring2_num_modes(device.get());
            break;
        case WACOM_STATUS_LED_TOUCHSTRIP:
            groupModes = libwacom_get_strips_num_modes(device.get());
            break;
        case WACOM_STATUS_LED_TOUCHSTRIP2:
            groupModes = libwacom_get_strips_num_modes(device.get());
            break;
        case WACOM_STATUS_LED_UNAVAILABLE:
            break;
        }
        numStatusLeds += groupModes;
    }
    tabletInfo.set(TabletInfo::StatusLEDs, numStatusLeds);

    tabletInfo.set(TabletInfo::TabletName, QString::fromLatin1(libwacom_get_name(device.get())));

    const int padButtonNumber = libwacom_get_num_buttons(device.get());
    tabletInfo.set(TabletInfo::NumPadButtons, padButtonNumber);

    // Convert button evdev codes to buttonMap
    if (libwacom_get_num_buttons(device.get()) > 0) {
        QMap<QString, QString> buttonMapping;
        for (char i = 1; i < padButtonNumber + 1; ++i) {
#ifdef LIBWACOM_EVDEV_MISSING
            // TODO: warn the user in the KCM too
            qCWarning(COMMON) << "Your libwacom version is too old. We will try and guess button mapping, "
                              << "but it's going to be broken for quirky tablets. Paired device detection is going to be broken as well."
                              << "Use kde_wacomtablet_finder to configure your device instead.";
            const int buttonIndex = skipWheelButtons(i);
            buttonMapping[QString::number(i)] = QString::number(buttonIndex);
#else
            const char buttonChar = 'A' + (i - 1); // libwacom marks buttons as 'A', 'B', 'C'...
            const int buttonEvdevCode = libwacom_get_button_evdev_code(device.get(), buttonChar);
            const int buttonIndex = convertEvdevToXsetwacomButton(buttonEvdevCode);
            buttonMapping[QString::number(i)] = QString::number(buttonIndex);

            if (buttonIndex < 1) {
                qCWarning(COMMON) << "Unrecognized evdev code. "
                                  << "Device:" << tabletId << "Vendor:" << vendorId << "Button:" << buttonChar << "EvdevCode:" << buttonEvdevCode;
                return false;
            }
#endif
        }
        tabletInfo.setButtonMap(buttonMapping);
    }

#ifndef LIBWACOM_EVDEV_MISSING
    const WacomMatch *paired_device_match = libwacom_get_paired_device(device.get());
    if (paired_device_match) {
        const std::uint32_t pairedTabletId = libwacom_match_get_product_id(paired_device_match);
        const std::uint32_t pairedVendorId = libwacom_match_get_vendor_id(paired_device_match);
        const auto pairedDeviceID = QString::number(pairedTabletId, 16);
        tabletInfo.set(TabletInfo::TouchSensorId, pairedDeviceID);

        qCDebug(COMMON) << "Libwacom reported a paired device" << pairedTabletId << pairedVendorId << pairedDeviceID;
    }
#endif

    const int numStrips = libwacom_get_num_strips(device.get());
    const bool hasLeftStrip = numStrips > 0;
    const bool hasRightStrip = numStrips > 1;
    const bool hasRing = libwacom_has_ring(device.get()) != 0;

    tabletInfo.set(TabletInfo::HasLeftTouchStrip, hasLeftStrip);
    tabletInfo.set(TabletInfo::HasRightTouchStrip, hasRightStrip);
    tabletInfo.set(TabletInfo::HasTouchRing, hasRing);
    return true;
}

#ifndef LIBWACOM_EVDEV_MISSING

static int convertMouseEvdevToXsetwacomButton(int evdevCode)
{
    // some quirky consumer tablets, e.g. Bamboo/Graphire, use mouse button events
    // instead of just numbered express keys. Translate them back to numbers
    constexpr int BTN_LEFT = 0x110;
    constexpr int BTN_RIGHT = 0x111;
    constexpr int BTN_MIDDLE = 0x112;
    constexpr int BTN_FORWARD = 0x115;
    constexpr int BTN_BACK = 0x116;

    switch (evdevCode) {
    case BTN_LEFT:
        return 1;
    case BTN_RIGHT:
        return 3;
    case BTN_MIDDLE:
        return 2;
    case BTN_FORWARD:
        return 9;
    case BTN_BACK:
        return 8;

    default:
        return 0;
    }
}

static int convertEvdevToXsetwacomButton(int evdevCode)
{
    // based on set_button_codes_from_heuristics from libwacom/libwacom-database.c
    // FIXME: this code might actually be wrong, see "Wacom Intuos BT M" (usb id 888 1386) for example:
    // It's actually 1 2 3 8
    // We report     1 3 8 9
    static const int BTN_MISC = 0x100;
    static const int BTN_MOUSE = 0x110;
    static const int BTN_BASE = 0x126;
    static const int BTN_GAMEPAD = 0x130;

    int translatedCode = 0;
    if (evdevCode >= BTN_GAMEPAD) {
        translatedCode = evdevCode - BTN_GAMEPAD + 10;
    } else if (evdevCode >= BTN_BASE) {
        translatedCode = evdevCode - BTN_BASE + 16;
    } else if (evdevCode >= BTN_MOUSE) {
        return convertMouseEvdevToXsetwacomButton(evdevCode);
    } else if (evdevCode >= BTN_MISC) {
        translatedCode = evdevCode - BTN_MISC;
    } else {
        return 0;
    }

    return skipWheelButtons(translatedCode + 1);
}
#endif // LIBWACOM_EVDEV_MISSING

} // namespace ends
