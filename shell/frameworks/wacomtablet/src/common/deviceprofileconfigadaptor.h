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

#ifndef DEVICEPROFILECONFIGADAPTOR_H
#define DEVICEPROFILECONFIGADAPTOR_H

#include "configadaptor.h"
#include "deviceprofile.h"

#include <KConfigGroup>

namespace Wacom
{

/**
 * A configuration adapter which can read and write device profiles.
 */
class DeviceProfileConfigAdaptor : public ConfigAdaptor
{
public:
    /**
     * Default constructor
     */
    explicit DeviceProfileConfigAdaptor(DeviceProfile &profile);

    /**
     * Default destructor
     */
    ~DeviceProfileConfigAdaptor() override;

    /**
     * Loads the device configuration from the given config group and sets
     * it on the managed device profile. The name of the device profile
     * will not be changed nor will the old properties be cleared before
     * setting the new ones.
     *
     * @param config The config group to read from.
     *
     * @return True on success, false on error.
     */
    bool loadConfig(const KConfigGroup &config) override;

    /**
     * Saves the device configuration of the currently managed device profile
     * to the given config group. The name of the config group will not be
     * changed and it will not be cleared before setting the new values.
     *
     * @param config The config group to write to.
     *
     * @return True on success, false on error.
     */
    bool saveConfig(KConfigGroup &config) const override;
};

}
#endif // HEADER PROTECTION
