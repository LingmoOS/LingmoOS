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

#include "tabletprofile.h"

#include "logging.h"

#include <QHash>

using namespace Wacom;

namespace Wacom
{
/**
 * Private class of the TabletProfile for the d-pointer
 *
 */
class TabletProfilePrivate
{
public:
    QHash<QString, DeviceProfile> devices;
    QString name;
};
}

TabletProfile::TabletProfile()
    : d_ptr(new TabletProfilePrivate)
{
}

TabletProfile::TabletProfile(const QString &name)
    : d_ptr(new TabletProfilePrivate)
{
    Q_D(TabletProfile);
    d->name = name;
}

TabletProfile::TabletProfile(const TabletProfile &profile)
    : d_ptr(new TabletProfilePrivate)
{
    operator=(profile);
}

TabletProfile::~TabletProfile()
{
    delete this->d_ptr;
}

TabletProfile &TabletProfile::operator=(const TabletProfile &that)
{
    Q_D(TabletProfile);
    d->devices = that.d_ptr->devices;
    d->name = that.d_ptr->name;

    return *this;
}

void TabletProfile::clearDevices()
{
    Q_D(TabletProfile);
    d->devices.clear();
}

const DeviceProfile TabletProfile::getDevice(const DeviceType &device) const
{
    Q_D(const TabletProfile);

    if (!hasDevice(device)) {
        return DeviceProfile(device);
    }

    return d->devices.value(device.key());
}

QString TabletProfile::getName() const
{
    Q_D(const TabletProfile);

    return d->name;
}

bool TabletProfile::hasDevice(const DeviceType &device) const
{
    Q_D(const TabletProfile);

    return d->devices.contains(device.key());
}

bool TabletProfile::hasDevice(const QString &device) const
{
    const DeviceType *deviceType = DeviceType::find(device);

    if (!deviceType) {
        return false;
    }

    return hasDevice(*deviceType);
}

QStringList TabletProfile::listDevices() const
{
    Q_D(const TabletProfile);

    QStringList result;

    // keys are all lower case, but we want to list the names as-is
    QList<QString> keys = d->devices.keys();
    for (const auto &key : std::as_const(keys)) {
        const DeviceType *deviceType = DeviceType::find(key);
        if (!deviceType) {
            qCWarning(COMMON) << "DeviceType for" << key << "is null";
            continue;
        }
        result.append(getDevice(*deviceType).getName());
    }
    return result;
}

bool TabletProfile::setDevice(const DeviceProfile &profile)
{
    Q_D(TabletProfile);

    if (profile.getName().isEmpty()) {
        return false;
    }

    d->devices.insert(profile.getName().toLower(), profile);

    return true;
}

void TabletProfile::setName(const QString &name)
{
    Q_D(TabletProfile);

    d->name = name;
}
