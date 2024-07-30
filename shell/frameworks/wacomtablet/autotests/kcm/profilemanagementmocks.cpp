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

#include "profilemanagementmocks.h"

#include <QtTest>

void Wacom::ProfileManagementIntegrityChecker::setTabletId(const QString &tabletId)
{
    Q_UNUSED(tabletId)
}

void Wacom::ProfileManagementIntegrityChecker::createNewProfile(const QString &profilename)
{
    Q_UNUSED(profilename)
}

const QStringList Wacom::ProfileManagementIntegrityChecker::availableProfiles()
{
    return QStringList();
}

void Wacom::ProfileManagementIntegrityChecker::deleteProfile()
{
}

Wacom::DeviceProfile Wacom::ProfileManagementIntegrityChecker::loadDeviceProfile(const Wacom::DeviceType &device)
{
    return _presetProfiles[device];
}

bool Wacom::ProfileManagementIntegrityChecker::saveDeviceProfile(const Wacom::DeviceProfile &profile)
{
    _savedProfiles[profile.getDeviceType()] = profile;
    return true;
}

void Wacom::ProfileManagementIntegrityChecker::setProfileName(const QString &name)
{
    Q_UNUSED(name)
}

QString Wacom::ProfileManagementIntegrityChecker::profileName() const
{
    // return QString();
    return QString::fromLatin1("Default");
}

void Wacom::ProfileManagementIntegrityChecker::reload()
{
}
