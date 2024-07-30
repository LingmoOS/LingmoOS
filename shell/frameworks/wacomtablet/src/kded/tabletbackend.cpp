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

#include "tabletbackend.h"

#include "logging.h"
#include "procsystemadaptor.h"
#include "procsystemproperty.h"
#include "property.h"
#include "propertyset.h"

namespace Wacom
{
class TabletBackendPrivate
{
public:
    TabletBackend::DeviceMap deviceAdaptors;
    PropertyAdaptor *statusLEDAdaptor;
    TabletInformation tabletInformation;
};
}

using namespace Wacom;

TabletBackend::TabletBackend(const Wacom::TabletInformation &tabletInformation)
    : d_ptr(new TabletBackendPrivate)
{
    Q_D(TabletBackend);
    d->tabletInformation = tabletInformation;

    d_ptr->statusLEDAdaptor = new ProcSystemAdaptor(d->tabletInformation.getDeviceName(DeviceType::Pad));
}

TabletBackend::~TabletBackend()
{
    // delete all property adaptors
    DeviceMap::iterator deviceIter;

    for (deviceIter = d_ptr->deviceAdaptors.begin(); deviceIter != d_ptr->deviceAdaptors.end(); ++deviceIter) {
        AdaptorList::iterator adaptorIter = deviceIter.value().begin();

        while (adaptorIter != deviceIter.value().end()) {
            delete (*adaptorIter);
            adaptorIter = deviceIter.value().erase(adaptorIter);
        }
    }

    delete d_ptr->statusLEDAdaptor;

    // delete private class
    delete d_ptr;
}

void TabletBackend::addAdaptor(const DeviceType &deviceType, PropertyAdaptor *adaptor)
{
    Q_D(TabletBackend);

    d->deviceAdaptors[deviceType].append(adaptor);
}

const TabletInformation &TabletBackend::getInformation() const
{
    Q_D(const TabletBackend);
    return d->tabletInformation;
}

const QString TabletBackend::getProperty(const DeviceType &type, const Property &property) const
{
    Q_D(const TabletBackend);

    DeviceMap::const_iterator adaptors = d->deviceAdaptors.constFind(type);
    if (adaptors == d->deviceAdaptors.constEnd()) {
        qCWarning(KDED) << QString::fromLatin1("Could not get property '%1' from unsupported device type '%2'!").arg(property.key()).arg(type.key());
        return QString();
    }

    foreach (const PropertyAdaptor *adaptor, adaptors.value()) {
        if (adaptor->supportsProperty(property)) {
            return adaptor->getProperty(property);
        }
    }

    return QString();
}

void TabletBackend::setProfile(const TabletProfile &profile)
{
    Q_D(TabletBackend);

    foreach (const DeviceType &deviceType, DeviceType::list()) {
        if (d->tabletInformation.hasDevice(deviceType)) {
            if (profile.hasDevice(deviceType)) {
                qCDebug(KDED) << QString::fromLatin1("Setting profile '%1' on tablet '%2', device '%3'")
                                     .arg(profile.getName())
                                     .arg(d->tabletInformation.get(TabletInfo::TabletName))
                                     .arg(deviceType.key());
                DeviceProfile deviceProfile = profile.getDevice(deviceType);
                setProfile(deviceType, deviceProfile);
            } else {
                qCDebug(KDED) << QString::fromLatin1("Skipping '%1' settings as the current profile does not contain any settings for this device...")
                                     .arg(deviceType.key());
            }
        } else {
            qCDebug(KDED) << QString::fromLatin1("Skipping '%1' settings as the device does not support it...").arg(deviceType.key());
        }
    }
}

void TabletBackend::setProfile(const DeviceType &deviceType, const DeviceProfile &profile)
{
    Q_D(TabletBackend);

    DeviceMap::iterator adaptors = d->deviceAdaptors.find(deviceType);
    if (adaptors == d->deviceAdaptors.end()) {
        qCWarning(KDED) << QString::fromLatin1("Could not set profile on unsupported device type '%1'!").arg(deviceType.key());
        return;
    }

    QString value;

    // set properties on all adaptors
    foreach (PropertyAdaptor *adaptor, adaptors.value()) {
        // ask adaptor which properties it supports
        // this will also ensure that the properties are set in the correct order
        foreach (const Property &property, adaptor->getProperties()) {
            // set property value if it is supported by the profile and not empty
            if (profile.supportsProperty(property)) {
                value = profile.getProperty(property);

                if (!value.isEmpty()) {
                    adaptor->setProperty(property, value);
                }
            }
        }
    }
}

void TabletBackend::setStatusLED(int led)
{
    Q_D(TabletBackend);
    if (d->tabletInformation.statusLEDs() > 0) {
        d_ptr->statusLEDAdaptor->setProperty(Property::StatusLEDs, QString::number(led));
    }
}

void TabletBackend::setStatusLEDBrightness(int brightness)
{
    Q_D(TabletBackend);
    if (d->tabletInformation.statusLEDs() > 0) {
        d_ptr->statusLEDAdaptor->setProperty(Property::StatusLEDsBrightness, QString::number(brightness));
    }
}

bool TabletBackend::setProperty(const DeviceType &type, const Property &property, const QString &value)
{
    Q_D(TabletBackend);

    DeviceMap::iterator adaptors = d->deviceAdaptors.find(type);
    if (adaptors == d->deviceAdaptors.end()) {
        qCWarning(KDED)
            << QString::fromLatin1("Could not set property '%1' to '%2' on unsupported device type '%3'!").arg(property.key()).arg(value).arg(type.key());
        return false;
    }

    bool returnValue = false;
    foreach (PropertyAdaptor *adaptor, adaptors.value()) {
        if (adaptor->supportsProperty(property)) {
            if (adaptor->setProperty(property, value)) {
                returnValue = true;
            }
        }
    }

    return returnValue;
}
