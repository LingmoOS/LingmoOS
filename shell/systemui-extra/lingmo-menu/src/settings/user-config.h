/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef LINGMO_MENU_USER_CONFIG_H
#define LINGMO_MENU_USER_CONFIG_H

#include <QSet>
#include <QVector>
#include <QObject>
#include <QMutex>

namespace LingmoUIMenu {

class UserConfig : public QObject
{
    Q_OBJECT
public:
    static const QString configFilePath;
    static const QString configFileName;

    static UserConfig *instance();

    bool isFirstStartUp() const;

    QSet<QString> preInstalledApps() const;
    void addPreInstalledApp(const QString &appid);
    void removePreInstalledApp(const QString &appid);
    bool isPreInstalledApps(const QString &appid) const;

    void sync();

private:
    explicit UserConfig(QObject *parent=nullptr);

    void init();
    void initConfigFile();
    void readConfigFile();
    void writeConfigFile();

private:
    bool m_isFirstStartUp {false};
    QMutex m_mutex;

    QSet<QString> m_preInstalledApps;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_USER_CONFIG_H
