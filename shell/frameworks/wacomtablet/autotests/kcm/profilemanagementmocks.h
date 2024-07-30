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

#pragma once

#include "profilemanagementinterface.h"

#include "deviceprofile.h"

namespace Wacom
{

class ProfileManagementIntegrityChecker : public ProfileManagementInterface
{
    // ProfileManagementInterface interface
public:
    void setTabletId(const QString &tabletId) override;
    void createNewProfile(const QString &profilename) override;
    const QStringList availableProfiles() override;
    void deleteProfile() override;
    DeviceProfile loadDeviceProfile(const DeviceType &device) override;
    bool saveDeviceProfile(const DeviceProfile &profile) override;
    void setProfileName(const QString &name) override;
    QString profileName() const override;
    void reload() override;

    // Test functions
    QMap<DeviceType, DeviceProfile> _presetProfiles;
    QMap<DeviceType, DeviceProfile> _savedProfiles;
};

}
