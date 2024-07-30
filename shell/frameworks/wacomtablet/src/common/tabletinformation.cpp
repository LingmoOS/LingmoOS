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

#include "tabletinformation.h"

#include "stringutils.h"

namespace Wacom
{
class TabletInformationPrivate
{
public:
    /*
     * Declarations
     */
    typedef QMap<QString, DeviceInformation> DeviceInformationMap;
    typedef QMap<QString, QString> TabletInfoMap;

    /*
     * Members
     */
    QString unknown; //!< An empty string which allows us to return a const reference.

    QMap<QString, QString> buttonMap;
    DeviceInformationMap deviceMap;
    TabletInfoMap infoMap;
    bool isAvailable = false;
    bool hasButtons = false;

    TabletInformationPrivate &operator=(const TabletInformationPrivate &that)
    {
        buttonMap = that.buttonMap;
        deviceMap = that.deviceMap;
        infoMap = that.infoMap;
        isAvailable = that.isAvailable;
        hasButtons = that.hasButtons;

        return *this;
    }

    bool operator==(const TabletInformationPrivate &that) const
    {
        // we don't care if the device is available or not
        // we also don't care about the button map
        if (hasButtons != that.hasButtons || infoMap.size() != that.infoMap.size() || deviceMap.size() != that.deviceMap.size()) {
            return false;
        }

        // QMap is always ordered by key so we can just iterate over it and compare keys directly
        TabletInfoMap::const_iterator thisInfoIter = infoMap.constBegin();
        TabletInfoMap::const_iterator thatInfoIter = that.infoMap.constBegin();

        while (thatInfoIter != infoMap.constEnd() && thatInfoIter != that.infoMap.constEnd()) {
            if (thisInfoIter.key().compare(thatInfoIter.key(), Qt::CaseInsensitive) != 0) {
                return false;
            }

            if (thisInfoIter.value().compare(thatInfoIter.value(), Qt::CaseInsensitive) != 0) {
                return false;
            }

            ++thisInfoIter;
            ++thatInfoIter;
        }

        DeviceInformationMap::const_iterator thisDevIter = deviceMap.constBegin();
        DeviceInformationMap::const_iterator thatDevIter = that.deviceMap.constBegin();

        while (thisDevIter != deviceMap.constEnd() && thatDevIter != that.deviceMap.constEnd()) {
            if (thisDevIter.key().compare(thatDevIter.key(), Qt::CaseInsensitive) != 0) {
                return false;
            }

            if (thisDevIter.value() != thatDevIter.value()) {
                return false;
            }

            ++thisDevIter;
            ++thatDevIter;
        }

        return true;
    }
}; // CLASS TabletInformationPrivate
} // NAMESPACE Wacom

using namespace Wacom;

TabletInformation::TabletInformation()
    : d_ptr(new TabletInformationPrivate)
{
    d_ptr->unknown.clear();
}

TabletInformation::TabletInformation(long tabletSerial)
    : d_ptr(new TabletInformationPrivate)
{
    set(TabletInfo::TabletSerial, QString::number(tabletSerial));
    d_ptr->unknown.clear();
}

TabletInformation::TabletInformation(const TabletInformation &that)
    : d_ptr(new TabletInformationPrivate)
{
    operator=(that);
}

TabletInformation::~TabletInformation()
{
    delete d_ptr;
}

TabletInformation &TabletInformation::operator=(const TabletInformation &that)
{
    Q_D(TabletInformation);

    d->operator=(*(that.d_ptr));

    return *this;
}

bool TabletInformation::operator!=(const TabletInformation &other) const
{
    return !operator==(other);
}

bool TabletInformation::operator==(const TabletInformation &other) const
{
    Q_D(const TabletInformation);
    return other.d_ptr != nullptr && d->operator==(*(other.d_ptr));
}

const QString &TabletInformation::get(const TabletInfo &info) const
{
    Q_D(const TabletInformation);

    TabletInformationPrivate::TabletInfoMap::const_iterator iter = d->infoMap.constFind(info.key());

    if (iter == d->infoMap.constEnd()) {
        return d->unknown;
    }

    return iter.value();
}

bool TabletInformation::getBool(const TabletInfo &info) const
{
    return (StringUtils::asBool(get(info)));
}

int TabletInformation::getInt(const TabletInfo &info) const
{
    return (get(info).toInt());
}

const QMap<QString, QString> &TabletInformation::getButtonMap() const
{
    Q_D(const TabletInformation);
    return d->buttonMap;
}

const DeviceInformation *TabletInformation::getDevice(const DeviceType &deviceType) const
{
    Q_D(const TabletInformation);

    TabletInformationPrivate::DeviceInformationMap::ConstIterator iter = d->deviceMap.constFind(deviceType.key());

    if (iter == d->deviceMap.constEnd()) {
        return nullptr;
    }

    return &(iter.value());
}

const QStringList TabletInformation::getDeviceList() const
{
    QString device;
    QStringList deviceList;

    foreach (const DeviceType &type, DeviceType::list()) {
        device = getDeviceName(type);

        if (!device.isEmpty()) {
            deviceList.append(device);
        }
    }

    return deviceList;
}

const QString &TabletInformation::getDeviceName(const DeviceType &device) const
{
    Q_D(const TabletInformation);

    TabletInformationPrivate::DeviceInformationMap::ConstIterator iter = d->deviceMap.find(device.key());

    if (iter == d->deviceMap.constEnd()) {
        return d->unknown;
    }

    return iter->getName();
}

long int TabletInformation::getTabletSerial() const
{
    return get(TabletInfo::TabletSerial).toLong();
}

bool TabletInformation::hasButtons() const
{
    if (getBool(TabletInfo::HasLeftTouchStrip) || getBool(TabletInfo::HasRightTouchStrip) || getBool(TabletInfo::HasTouchRing) || getBool(TabletInfo::HasWheel)
        || getInt(TabletInfo::NumPadButtons) > 0) {
        return true;
    }

    return false;
}

bool TabletInformation::hasButtonMap() const
{
    Q_D(const TabletInformation);
    return (d->buttonMap.size() > 0);
}

bool TabletInformation::hasDevice(int deviceId) const
{
    foreach (const DeviceType &type, DeviceType::list()) {
        const DeviceInformation *device = getDevice(type);

        if (device && device->getDeviceId() == deviceId) {
            return true;
        }
    }

    return false;
}

bool TabletInformation::hasDevice(const DeviceType &device) const
{
    Q_D(const TabletInformation);
    return d->deviceMap.contains(device.key());
}

int TabletInformation::statusLEDs() const
{
    return getInt(TabletInfo::StatusLEDs);
}

bool TabletInformation::isAvailable() const
{
    Q_D(const TabletInformation);
    return d->isAvailable;
}

void TabletInformation::set(const TabletInfo &info, const QString &value)
{
    Q_D(TabletInformation);

    // setting the tablet serial requires updating the id for now
    if (info == TabletInfo::TabletSerial) {
        long serial = value.toLong();

        if (serial > 0) {
            set(TabletInfo::TabletId, QString::fromLatin1("%1").arg(serial, 4, 16, QLatin1Char('0')).toUpper());
        }
    }

    TabletInformationPrivate::TabletInfoMap::iterator iter = d->infoMap.find(info.key());

    // make sure only keys exist which actually have a value
    if (iter != d->infoMap.end()) {
        if (value.isEmpty()) {
            d->infoMap.erase(iter);
        } else {
            iter.value() = value;
        }

    } else if (!value.isEmpty()) {
        d->infoMap.insert(info.key(), value);
    }
}

void TabletInformation::set(const TabletInfo &info, bool value)
{
    QString strValue = value ? QLatin1String("true") : QLatin1String("false");
    set(info, strValue);
}

void TabletInformation::set(const TabletInfo &info, int value)
{
    set(info, QString::number(value));
}

void TabletInformation::setAvailable(bool value)
{
    Q_D(TabletInformation);
    d->isAvailable = value;
}

void TabletInformation::setBool(const TabletInfo &info, const QString &value)
{
    if (StringUtils::asBool(value)) {
        set(info, QLatin1String("true"));
    } else {
        set(info, QLatin1String("false"));
    }
}

void TabletInformation::setButtonMap(const QMap<QString, QString> &buttonMap)
{
    Q_D(TabletInformation);
    d->buttonMap = buttonMap;
}

void TabletInformation::setDevice(const DeviceInformation &device)
{
    Q_D(TabletInformation);
    d->deviceMap.insert(device.getType().key(), device);
}

QString TabletInformation::getUniqueDeviceId() const
{
    return QString::fromLatin1("%1:%2").arg(get(TabletInfo::CompanyId)).arg(get(TabletInfo::TabletId));
}

QString TabletInformation::getLegacyUniqueDeviceId() const
{
    return get(TabletInfo::TabletName);
}
