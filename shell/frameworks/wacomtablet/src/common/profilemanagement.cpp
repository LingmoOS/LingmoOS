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

#include "profilemanagement.h"

#include "logging.h"

// common
#include "dbustabletinterface.h"
#include "deviceprofiledefaults.h"
#include "devicetype.h"
#include "property.h"
#include "screenrotation.h"
#include "tabletinfo.h"
#include "tabletprofile.h"

// Qt includes
#include <QDBusInterface>
#include <QDBusReply>

using namespace Wacom;

ProfileManagement::ProfileManagement()
    : m_profileManager(QLatin1String("tabletprofilesrc"))
{
}

ProfileManagement::ProfileManagement(const QString &deviceName, bool hasTouch)
    : m_deviceName(deviceName)
    , m_hasTouch(hasTouch)
    , m_profileManager(QLatin1String("tabletprofilesrc"))
{
    qCDebug(COMMON) << "Create instance for :: " << deviceName << "Touch?" << hasTouch;
}

ProfileManagement &ProfileManagement::instance()
{
    static ProfileManagement instance;
    return instance;
}

ProfileManagement &ProfileManagement::instance(const QString &deviceName, bool hasTouch)
{
    static ProfileManagement instance(deviceName, hasTouch);
    return instance;
}

void ProfileManagement::setTabletId(const QString &tabletId)
{
    m_tabletId = tabletId;
}

void ProfileManagement::createNewProfile(const QString &profilename)
{
    if (profilename.isEmpty()) {
        qCWarning(COMMON) << "Can not create a profile with no name!";
    }

    // get information via DBus
    m_profileName = profilename;

    if (m_deviceName.isEmpty()) {
        qCWarning(COMMON) << "No device information is found. Can't create a new profile";
        return;
    }

    qCDebug(COMMON) << "Creating a new profile for device" << m_deviceName;

    m_profileManager.readProfiles(m_deviceName);
    TabletProfile tabletProfile = m_profileManager.loadProfile(profilename);

    DeviceProfile padDevice = tabletProfile.getDevice(DeviceType::Pad);
    DeviceProfile stylusDevice = tabletProfile.getDevice(DeviceType::Stylus);
    DeviceProfile eraserDevice = tabletProfile.getDevice(DeviceType::Eraser);

    setupDefaultPad(padDevice);
    setupDefaultStylus(stylusDevice);
    setupDefaultStylus(eraserDevice);

    tabletProfile.setDevice(padDevice);
    tabletProfile.setDevice(stylusDevice);
    tabletProfile.setDevice(eraserDevice);

    // also add section for the touch if we have a touch tool
    if (m_hasTouch) {
        DeviceProfile touchDevice = tabletProfile.getDevice(DeviceType::Touch);
        setupDefaultTouch(touchDevice);
        tabletProfile.setDevice(touchDevice);
    }

    m_profileManager.saveProfile(tabletProfile);

    // FIXME: workaround for devices with a separate touch sensor USB ID
    // Correct way to solve this is probably rewrite this whole class or completely get rid of it
    if (!m_sensorId.isEmpty()) {
        m_profileManager.readProfiles(m_sensorId);
        TabletProfile childTabletProfile = m_profileManager.loadProfile(profilename);
        DeviceProfile touchDevice = childTabletProfile.getDevice(DeviceType::Touch);
        setupDefaultTouch(touchDevice);
        childTabletProfile.setDevice(touchDevice);
        m_profileManager.saveProfile(childTabletProfile);
    }

    m_profileManager.readProfiles(m_deviceName);
}

const QStringList ProfileManagement::availableProfiles()
{
    m_profileManager.readProfiles(m_deviceName);
    return m_profileManager.listProfiles();
}

void ProfileManagement::deleteProfile()
{
    m_profileManager.readProfiles(m_deviceName);
    m_profileManager.deleteProfile(m_profileName);

    if (!m_sensorId.isEmpty()) {
        m_profileManager.readProfiles(m_sensorId);
        m_profileManager.deleteProfile(m_profileName);
    }

    m_profileName.clear();
    m_profileManager.reload();

    if (m_profileManager.listProfiles().isEmpty()) {
        createNewProfile(QLatin1String("Default"));
        m_profileManager.reload();
    }
}

DeviceProfile ProfileManagement::loadDeviceProfile(const DeviceType &device)
{
    if (m_sensorId.isEmpty() || device != DeviceType::Touch) {
        m_profileManager.readProfiles(m_deviceName);
    } else {
        m_profileManager.readProfiles(m_sensorId);
    }

    return m_profileManager.loadProfile(m_profileName).getDevice(device);
}

bool ProfileManagement::saveDeviceProfile(const DeviceProfile &profile)
{
    if (m_sensorId.isEmpty() || profile.getDeviceType() != DeviceType::Touch) {
        m_profileManager.readProfiles(m_deviceName);
    } else {
        m_profileManager.readProfiles(m_sensorId);
    }
    TabletProfile tabletProfile = m_profileManager.loadProfile(m_profileName);
    tabletProfile.setDevice(profile);

    return m_profileManager.saveProfile(tabletProfile);
}

void ProfileManagement::setProfileName(const QString &name)
{
    m_profileName = name;
}

QString ProfileManagement::profileName() const
{
    return m_profileName;
}

void ProfileManagement::reload()
{
    auto vendorId = DBusTabletInterface::instance().getInformation(m_tabletId, TabletInfo::CompanyId.key());
    vendorId.waitForFinished();
    if (vendorId.isValid()) {
        m_vendorId = vendorId;
    } else {
        qCWarning(COMMON) << "Couldn't get vendor id for" << m_tabletId;
        m_vendorId = QString::fromLatin1("unknown");
    }

    m_deviceName = QString::fromLatin1("%1:%2").arg(m_vendorId).arg(m_tabletId);

    auto touchSensorId = DBusTabletInterface::instance().getTouchSensorId(m_tabletId);
    m_sensorId = touchSensorId.value();
    if (touchSensorId.isValid() && !m_sensorId.isEmpty()) {
        m_sensorId = QString::fromLatin1("%1:%2").arg(m_vendorId).arg(m_sensorId);
        qCInfo(COMMON) << "Multi-device touch" << m_sensorId;
    }

    auto touchName = DBusTabletInterface::instance().getDeviceName(m_tabletId, DeviceType::Touch.key());
    touchName.waitForFinished();
    if (touchName.isValid()) {
        qCDebug(COMMON) << "touchName for" << m_tabletId << "is" << touchName.value();
        m_hasTouch = !QString(touchName.value()).isEmpty();
    } else {
        m_hasTouch = false;
    }
}
