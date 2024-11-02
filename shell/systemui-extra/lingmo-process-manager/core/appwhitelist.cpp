/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "appwhitelist.h"

AppWhitelist::AppWhitelist(ConfigManager &configManager)
    : m_configManager{configManager}
{
}

bool AppWhitelist::AddApp(const QString &desktopFile)
{
    return m_configManager.addAppToWhitelist(desktopFile);
}

bool AppWhitelist::RemoveApp(const QString &desktopFile)
{
    return m_configManager.removeAppFromWhitelist(desktopFile);
}

QStringList AppWhitelist::AppList() const
{
    return m_configManager.appWhitelist();
}
