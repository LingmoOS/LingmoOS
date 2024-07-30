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

#include "x11input.h"

#include <QList>

#include "private/qtx11extras_p.h"

#include <xorg/wacom-properties.h>

#include <X11/extensions/XInput.h>

using namespace Wacom;

const QString X11Input::PROPERTY_DEVICE_PRODUCT_ID = QLatin1String("Device Product ID");
const QString X11Input::PROPERTY_DEVICE_NODE = QLatin1String("Device Node");
const QString X11Input::PROPERTY_TRANSFORM_MATRIX = QLatin1String("Coordinate Transformation Matrix");
const QString X11Input::PROPERTY_WACOM_SERIAL_IDS = QLatin1String(WACOM_PROP_SERIALIDS);
const QString X11Input::PROPERTY_WACOM_TABLET_AREA = QLatin1String(WACOM_PROP_TABLET_AREA);
const QString X11Input::PROPERTY_WACOM_TOOL_TYPE = QLatin1String(WACOM_PROP_TOOL_TYPE);

bool X11Input::findDevice(const QString &deviceName, X11InputDevice &device)
{
    if (deviceName.isEmpty()) {
        return false;
    }

    bool found = false;
    int ndevices = 0;
    Display *display = QX11Info::display();

    // Don't port this to xcb, not supported yet.
    XDeviceInfo *info = XListInputDevices(display, &ndevices);

    for (int i = 0; i < ndevices; ++i) {
        if (deviceName.compare(QLatin1String(info[i].name), Qt::CaseInsensitive) == 0) {
            found = device.open(info[i].id, QLatin1String(info[i].name));
            break;
        }
    }

    if (info) {
        XFreeDeviceList(info);
    }

    return found;
}

void X11Input::scanDevices(X11InputVisitor &visitor)
{
    int ndevices = 0;
    Display *dpy = QX11Info::display();

    XDeviceInfo *info = XListInputDevices(dpy, &ndevices);

    for (int i = 0; i < ndevices; ++i) {
        X11InputDevice device(info[i].id, QLatin1String(info[i].name));

        if (visitor.visit(device)) {
            break;
        }
    }

    if (info) {
        XFreeDeviceList(info);
    }
}
