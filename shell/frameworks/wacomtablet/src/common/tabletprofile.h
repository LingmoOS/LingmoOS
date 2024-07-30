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

#ifndef TABLETPROFILE_H
#define TABLETPROFILE_H

#include <KConfigGroup>
#include <QString>

#include "deviceprofile.h"

namespace Wacom
{

class TabletProfilePrivate;

/**
 * This class implements the profile of a single device (stylus/eraser/cursor/pad/touch)
 */
class TabletProfile
{
public:
    /**
     * Default constructor
     */
    TabletProfile();

    /**
     * Initializes new instance with name.
     *
     * @param name The name of this profile.
     */
    explicit TabletProfile(const QString &name);

    /**
     * Copy Constructor
     *
     * @param profile The profile to copy.
     */
    TabletProfile(const TabletProfile &profile);

    /**
     * Default destructor
     */
    ~TabletProfile();

    /**
     * Copy operator.
     *
     * @param that The instance to copy.
     */
    TabletProfile &operator=(const TabletProfile &that);

    /**
     * Clears all devices from the current profile.
     */
    void clearDevices();

    /**
     * Gets the name of this tablet configuration.
     *
     * @return The name of this tablet configuration.
     */
    QString getName() const;

    /**
     * Gets the profile of a device. If the device does not exist within this
     * tablet configuration and empty device profile is returned.
     *
     * @param device The name of the device profile to get.
     *
     * @return The requested device profile or an empty one if the requested one does not exist.
     */
    const DeviceProfile getDevice(const DeviceType &device) const;

    /**
     * Checks if this tablet has a configuration for the given device (stylus/eraser/touch/pad/...)
     *
     * @param device The device type (stylus/eraser/touch/pad/...).
     *
     * @return True if a configuration is present, else false.
     */
    bool hasDevice(const DeviceType &device) const;

    /**
     * Checks if the tablet has configuration for the given device.
     * The parameters has to be a string as returned by DeviceType::key().
     *
     * @param device The device type to check for.
     *
     * @return True if a configuration is present, else false.
     */
    bool hasDevice(const QString &device) const;

    /**
     * Lists all device profile names of this tablet configuration.
     *
     * @return A list of device profile names.
     */
    QStringList listDevices() const;

    /**
     * Sets a device profile. For this to work, the profile has to have a name set.
     *
     * @param profile The profile to add.
     *
     * @return True if the profile was added, else false.
     */
    bool setDevice(const DeviceProfile &profile);

    /**
     * Sets the name of this profile.
     *
     * @param name The new name of this profile.
     */
    void setName(const QString &name);

private:
    Q_DECLARE_PRIVATE(TabletProfile)

    TabletProfilePrivate *const d_ptr; /**< d-pointer for this class */
};

} // NAMESPACE
#endif // HEADER PROTECTION
