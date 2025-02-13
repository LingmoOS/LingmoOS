/*
    Copyright 2017 Jan Grulich <jgrulich@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "configuration.h"

#include <QSettings>
#include <sys/types.h>
#include <pwd.h>

static bool propManageVirtualConnectionsInitialized = false;
static bool propManageVirtualConnections = false;

Configuration::Configuration()
{
    ::passwd *pw = ::getpwuid(::getuid());
    m_userName = QString::fromLocal8Bit(pw->pw_name);
}

Configuration &Configuration::self()
{
    static Configuration c;
    return c;
}

bool Configuration::unlockModemOnDetection()
{
    QSettings config(QSettings::UserScope, "lingmoos", "nm");
    config.beginGroup(QLatin1String("General"));

    return config.value("UnlockModemOnDetection", true).toBool();
}

void Configuration::setUnlockModemOnDetection(bool unlock)
{
    QSettings config(QSettings::UserScope, "lingmoos", "nm");
    config.beginGroup(QLatin1String("General"));
    config.setValue(QLatin1String("UnlockModemOnDetection"), unlock);
}

bool Configuration::manageVirtualConnections()
{
    // Avoid reading from the config file over and over
    if (propManageVirtualConnectionsInitialized) {
        return propManageVirtualConnections;
    }

    QSettings config(QSettings::UserScope, "lingmoos", "nm");
    config.beginGroup(QLatin1String("General"));

    propManageVirtualConnections = config.value(QLatin1String("ManageVirtualConnections"), false).toBool();
    propManageVirtualConnectionsInitialized = true;

    return propManageVirtualConnections;
}

void Configuration::setManageVirtualConnections(bool manage)
{
    QSettings config(QSettings::UserScope, "lingmoos", "nm");
    config.beginGroup(QLatin1String("General"));

    config.setValue(QLatin1String("ManageVirtualConnections"), manage);
    propManageVirtualConnections = manage;
}

bool Configuration::airplaneModeEnabled()
{
    // Check whether other devices are disabled to assume airplane mode is enabled
    // after suspend
    const bool isWifiDisabled = !NetworkManager::isWirelessEnabled() || !NetworkManager::isWirelessHardwareEnabled();
    const bool isWwanDisabled = !NetworkManager::isWwanEnabled() || !NetworkManager::isWwanHardwareEnabled();

    QSettings config(QSettings::UserScope, "lingmoos", "nm");
    config.beginGroup(QLatin1String("General"));

    if (config.value(QLatin1String("AirplaneModeEnabled"), false).toBool()) {
        // We can assume that airplane mode is still activated after resume
        if (isWifiDisabled && isWwanDisabled)
            return true;
        else {
            setAirplaneModeEnabled(false);
        }
    }

    return false;
}

void Configuration::setAirplaneModeEnabled(bool enabled)
{
    QSettings config(QSettings::UserScope, "lingmoos", "nm");
    config.beginGroup(QLatin1String("General"));
    config.setValue(QLatin1String("AirplaneModeEnabled"), enabled);
}

QString Configuration::hotspotName()
{
    QSettings config(QSettings::UserScope, "lingmoos", "nm");
    config.beginGroup(QLatin1String("General"));

    const QString defaultName = m_userName + QLatin1String("-hotspot");

    return config.value(QLatin1String("HotspotName"), defaultName).toString();
}

void Configuration::setHotspotName(const QString &name)
{
    QSettings config(QSettings::UserScope, "lingmoos", "nm");
    config.beginGroup(QLatin1String("General"));
    config.setValue(QLatin1String("HotspotName"), name);
}

QString Configuration::hotspotPassword()
{
    QSettings config(QSettings::UserScope, "lingmoos", "nm");
    config.beginGroup(QLatin1String("General"));
    return config.value(QLatin1String("HotspotPassword"), QString()).toString();
}

void Configuration::setHotspotPassword(const QString &password)
{
    QSettings config(QSettings::UserScope, "lingmoos", "nm");
    config.beginGroup(QLatin1String("General"));
    config.setValue(QLatin1String("HotspotPassword"), password);
}

QString Configuration::hotspotConnectionPath()
{
    QSettings config(QSettings::UserScope, "lingmoos", "nm");
    config.beginGroup(QLatin1String("General"));
    return config.value(QLatin1String("HotspotConnectionPath"), QString()).toString();
}

void Configuration::setHotspotConnectionPath(const QString &path)
{
    QSettings config(QSettings::UserScope, "lingmoos", "nm");
    config.beginGroup(QLatin1String("General"));
    config.setValue(QLatin1String("HotspotConnectionPath"), path);
}

bool Configuration::showPasswordDialog()
{
    QSettings config(QSettings::UserScope, "lingmoos", "nm");
    config.beginGroup(QLatin1String("General"));

    return config.value(QLatin1String("ShowPasswordDialog"), true).toBool();
}

