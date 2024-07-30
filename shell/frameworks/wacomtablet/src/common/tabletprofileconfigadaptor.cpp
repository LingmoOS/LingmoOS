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

#include "tabletprofileconfigadaptor.h"

#include "deviceprofileconfigadaptor.h"
#include "logging.h"

using namespace Wacom;

namespace Wacom
{
/**
 * Private class for the d-pointer.
 */
class TabletProfileConfigAdaptorPrivate
{
public:
    TabletProfile *profile = nullptr;
}; // CLASS
} // NAMESPACE

TabletProfileConfigAdaptor::TabletProfileConfigAdaptor(TabletProfile &profile)
    : ConfigAdaptor(nullptr)
    , d_ptr(new TabletProfileConfigAdaptorPrivate)
{
    Q_D(TabletProfileConfigAdaptor);
    d->profile = &profile;
}

TabletProfileConfigAdaptor::~TabletProfileConfigAdaptor()
{
    delete this->d_ptr;
}

bool TabletProfileConfigAdaptor::loadConfig(const KConfigGroup &config)
{
    Q_D(TabletProfileConfigAdaptor);
    if (d->profile == nullptr) {
        qCWarning(COMMON) << "Profile is null";
        return false;
    }

    d->profile->setName(config.name());
    d->profile->clearDevices();

    QStringList devices = config.groupList();

    foreach (const QString &dev, devices) {
        const DeviceType *deviceType = DeviceType::find(dev);

        if (deviceType == nullptr) {
            qCWarning(COMMON) << QString::fromLatin1("Invalid device identifier '%1' found in configuration file!").arg(dev);
            continue;
        }

        KConfigGroup devconfig(&config, dev);
        DeviceProfile devprofile(*deviceType);
        DeviceProfileConfigAdaptor devadaptor(devprofile);

        devadaptor.loadConfig(devconfig);

        d->profile->setDevice(devprofile);
    }

    return true;
}

bool TabletProfileConfigAdaptor::saveConfig(KConfigGroup &config) const
{
    Q_D(const TabletProfileConfigAdaptor);
    if (d->profile == nullptr) {
        qCWarning(COMMON) << "Profile is null";
        return false;
    }

    // delete all groups before writing out the new device groups
    QStringList groups = config.groupList();

    foreach (const QString &group, groups) {
        KConfigGroup(&config, group).deleteGroup();
    }

    // write out new device config groups
    QStringList devices = d->profile->listDevices();

    foreach (const QString &dev, devices) {
        const DeviceType *deviceType = DeviceType::find(dev);
        if (deviceType == nullptr) {
            qCWarning(COMMON) << QString::fromLatin1("Invalid device identifier '%1' found in configuration file!").arg(dev);
            continue;
        }

        KConfigGroup devconfig = KConfigGroup(&config, dev);
        DeviceProfile devprofile = d->profile->getDevice(*deviceType);
        DeviceProfileConfigAdaptor devadaptor(devprofile);

        devconfig.deleteGroup();
        devadaptor.saveConfig(devconfig);
    }

    return true;
}
