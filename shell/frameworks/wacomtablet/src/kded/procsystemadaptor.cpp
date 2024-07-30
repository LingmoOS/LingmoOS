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

#include "procsystemadaptor.h"

#include "logging.h"
#include "procsystemproperty.h"

#include <QProcess>

using namespace Wacom;

namespace Wacom
{
class ProcSystemAdaptorPrivate
{
public:
    QString deviceName;
}; // CLASS
} // NAMESPACE

ProcSystemAdaptor::ProcSystemAdaptor(const QString &deviceName)
    : PropertyAdaptor(nullptr)
    , d_ptr(new ProcSystemAdaptorPrivate)
{
    Q_D(ProcSystemAdaptor);
    d->deviceName = deviceName;
}

ProcSystemAdaptor::~ProcSystemAdaptor()
{
    delete this->d_ptr;
}

const QList<Property> ProcSystemAdaptor::getProperties() const
{
    return ProcSystemProperty::ids();
}

const QString ProcSystemAdaptor::getProperty(const Property &property) const
{
    Q_D(const ProcSystemAdaptor);

    qCWarning(KDED) << QString::fromLatin1("Can not get unsupported property '%1' from device '%2' using proc system!").arg(property.key()).arg(d->deviceName);

    return QString();
}

bool ProcSystemAdaptor::setProperty(const Property &property, const QString &value)
{
    qCDebug(KDED) << QString::fromLatin1("Setting property '%1' to '%2'.").arg(property.key()).arg(value);

    // https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-driver-wacom
    /* /sys/bus/hid/devices/<bus>:<vid>:<pid>.<n>/wacom_led/status0_luminance
        <obsoleted by the LED class API now exported by the driver>
        Writing to this file sets the status LED luminance (1..127)
        when the stylus does not touch the tablet surface, and no
        button is pressed on the stylus. This luminance level is
        normally lower than the level when a button is pressed.
    */
    /* /sys/bus/hid/devices/<bus>:<vid>:<pid>.<n>/wacom_led/status1_luminance
        <obsoleted by the LED class API now exported by the driver>
        Writing to this file sets the status LED luminance (1..127)
        when the stylus touches the tablet surface, or any button is
        pressed on the stylus.
    */
    // https://www.kernel.org/doc/Documentation/leds/leds-class.txt

    QString cmd;
    if (property == Property::StatusLEDs) {
        int statusLed = value.toInt();
        if (statusLed < 4 && statusLed >= 0) {
            cmd = QString::fromLatin1("bash -c \"echo %1 > /sys/bus/hid/devices/*/wacom_led/status_led0_select\"").arg(statusLed);
        } else if (statusLed < 8 && statusLed >= 4) {
            statusLed -= 4;
            cmd = QString::fromLatin1("bash -c \"echo %1 > /sys/bus/hid/devices/*/wacom_led/status_led1_select\"").arg(statusLed);
        } else {
            return false;
        }
    } else if (property == Property::StatusLEDsBrightness) {
        int statusLedBrightness = value.toInt();
        if (statusLedBrightness < 128 && statusLedBrightness >= 0) {
            cmd = QString::fromLatin1("bash -c \"echo %1 > /sys/bus/hid/devices/*/wacom_led/status0_luminance\"").arg(statusLedBrightness);
        } else if (statusLedBrightness < 256 && statusLedBrightness >= 128) {
            statusLedBrightness -= 128;
            cmd = QString::fromLatin1("bash -c \"echo %1 > /sys/bus/hid/devices/*/wacom_led/status1_luminance\"").arg(statusLedBrightness);
        } else {
            return false;
        }
    } else {
        qCWarning(KDED) << "Unknown Property: " << property.key();
    }

    return QProcess::execute(cmd, {}) == 0;
}

bool ProcSystemAdaptor::supportsProperty(const Property &property) const
{
    return (ProcSystemProperty::map(property));
}
