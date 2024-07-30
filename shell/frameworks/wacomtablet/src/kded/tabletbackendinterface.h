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

#ifndef TABLETBACKENDINTERFACE_H
#define TABLETBACKENDINTERFACE_H

#include "deviceprofile.h"
#include "devicetype.h"
#include "property.h"
#include "propertyadaptor.h"
#include "tabletinformation.h"
#include "tabletprofile.h"

#include <QString>

namespace Wacom
{
/**
 * This interface only exists so we can create a mock object for unit testing.
 */
class TabletBackendInterface
{
public:
    virtual ~TabletBackendInterface()
    {
    }

    /**
     * Adds a property adaptor for the given device type. The property adaptor
     * will be deleted once this class is destroyed.
     *
     * @param deviceType The device type to add the property adaptor for.
     * @param adaptor    The property adaptor to add.
     */
    virtual void addAdaptor(const DeviceType &deviceType, PropertyAdaptor *adaptor) = 0;

    /**
     * Returns tablet information about the tablet handled by this backend.
     *
     * @return TabletInformation
     */
    virtual const TabletInformation &getInformation() const = 0;

    /**
     * Gets a tablet property. If the property is not supported by any of the
     * adaptors which were added to this tablet, an empty string is returned.
     *
     * @param type     The device to read the property from.
     * @param property The property to get.
     *
     * @return The property value or an empty string.
     */
    virtual const QString getProperty(const DeviceType &type, const Property &property) const = 0;

    /**
     * Applies a profile to the tablet managed by this backend.
     *
     * @param profile The profile to apply.
     */
    virtual void setProfile(const TabletProfile &profile) = 0;

    /**
     * Applies a profile to a device.
     *
     * @param deviceType The device to apply the profile to.
     * @param profile    The profile to apply.
     */
    virtual void setProfile(const DeviceType &deviceType, const DeviceProfile &profile) = 0;

    /**
     * Set the status LEDs for the Intuos4/5 and Cintiq tablets
     *
     * @param led led number that should be set 0-3 or 0-7
     */
    virtual void setStatusLED(int led) = 0;

    /**
     * Set the status LED brightness for the Intuos4/5 and Cintiq tablets
     *
     * @param brightness brightness gets a value of 0 or 128(off), 32 or 160(low), 64 or 192(mid), 96 or 224(high)
     */
    virtual void setStatusLEDBrightness(int brightness) = 0;

    /**
     * Sets a property on a device.
     *
     * @param type     The device to set the property on.
     * @param property The property to set on the device.
     * @param value    The property value to set.
     */
    virtual bool setProperty(const DeviceType &type, const Property &property, const QString &value) = 0;

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
