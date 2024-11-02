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

#ifndef APPWHITELIST_H
#define APPWHITELIST_H

#include <QObject>
#include "configmanager.h"

class AppWhitelist : public QObject
{
    Q_OBJECT
public:
    explicit AppWhitelist(ConfigManager &configManager);

    // dbus service interfaces
    bool AddApp(const QString &desktopFile);
    bool RemoveApp(const QString &desktopFile);
    QStringList AppList() const;

private:
    ConfigManager &m_configManager;
};

#endif // APPWHITELIST_H
