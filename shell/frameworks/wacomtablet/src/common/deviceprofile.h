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

#ifndef DEVICEPROFILE_H
#define DEVICEPROFILE_H

#include <QList>
#include <QString>

#include "devicetype.h"
#include "property.h"
#include "propertyadaptor.h"

namespace Wacom
{

class DeviceProfilePrivate;

/**
 * This class implements the profile of a single device (stylus/eraser/cursor/pad/touch)
 */
class DeviceProfile : public PropertyAdaptor
{
public:
    /**
     * Default constructor
     */
    DeviceProfile();

    /**
     * @param type The type of the device.
     */
    explicit DeviceProfile(const DeviceType &type);

    /**
     * Copy constructor.
     */
    DeviceProfile(const DeviceProfile &profile);

    /**
     * Default destructor
     */
    ~DeviceProfile() override;

    /**
     * Copy operator.
     *
     * @param that The instance to copy.
     *
     * @return A reference to this instance.
     */
    DeviceProfile &operator=(const DeviceProfile &that);

    /**
     * @return X11 event to which the given button should be mapped.
     */
    const QString getButton(int number) const;

    Wacom::DeviceType getDeviceType() const;

    /**
     * Gets the name of this device profile.
     * This is the same as getDeviceType().
     *
     * @return The name of this profile.
     */
    const QString &getName() const;

    /**
     * @return The value of the given property or an empty string.
     */
    const QString getProperty(const Property &key) const override;

    /**
     * @return A list of properties supported by this profile.
     */
    const QList<Property> getProperties() const override;

    /**
     * Sets the button shortcut by button number.
     *
     * @param number   The button number.
     * @param shortcut The new button shortcut.
     */
    bool setButton(int number, const QString &shortcut);

    /**
     * Sets the device type of this profile.
     *
     * @param type The new device type.
     */
    void setDeviceType(const DeviceType &type);

    /**
     * Sets a property.
     *
     * @param key   The property to set.
     * @param value The property's value.
     */
    bool setProperty(const Property &key, const QString &value) override;

    /**
     * Checks if the given property is supported.
     * This does not mean that the property does have a value.
     *
     * @param property The property to check for.
     */
    bool supportsProperty(const Property &property) const override;

private:
    Q_DECLARE_PRIVATE(DeviceProfile)

    DeviceProfilePrivate *const d_ptr; /**< d-pointer for this class */

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
