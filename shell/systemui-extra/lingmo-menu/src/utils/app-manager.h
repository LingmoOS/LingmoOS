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

#ifndef LINGMO_MENU_APP_MANAGER_H
#define LINGMO_MENU_APP_MANAGER_H
#include <QObject>
#include "lingmo-menu-application.h"

class QDBusInterface;

namespace LingmoUIMenu {

class AppManager : public QObject
{
    Q_OBJECT
public:
    static AppManager *instance();
    AppManager(const AppManager &obj) = delete;
    AppManager &operator = (const AppManager &obj) = delete;

    Q_INVOKABLE void launchApp(const QString &appid);
    Q_INVOKABLE void launchBinaryApp(const QString &app, const QString &args = QString());
    Q_INVOKABLE void launchAppWithArguments(const QString &appid, const QStringList &args, const QString &applicationName);
    Q_INVOKABLE void changeFavoriteState(const QString &appid, bool isFavorite);

private:
    explicit AppManager(QObject *parent = nullptr);
    static QStringList parseDesktopFile(const QString &desktopFilePath);

Q_SIGNALS:
    void request(LingmoUIMenuApplication::Command command);
};

} // LingmoUIMenu

#endif //LINGMO_MENU_APP_MANAGER_H
