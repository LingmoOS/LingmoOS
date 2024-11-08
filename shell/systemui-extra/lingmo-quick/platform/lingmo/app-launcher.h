/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef LINGMO_QUICK_APP_LAUNCHER_H
#define LINGMO_QUICK_APP_LAUNCHER_H
#include <QObject>
namespace LingmoUIQuick {

class AppLauncherPrivate;
class AppLauncher : public QObject
{
    Q_OBJECT
public:
    static AppLauncher *instance();
    ~AppLauncher();

    Q_INVOKABLE void launchApp(const QString &desktopFile);
    Q_INVOKABLE void launchAppWithArguments(const QString &desktopFile, const QStringList &args);
    Q_INVOKABLE void runCommand(const QString &cmd);
    Q_INVOKABLE void openUri(const QString &uri, const QString &parentWindow = "");

private:
    explicit AppLauncher(QObject *parent = nullptr);
    AppLauncherPrivate *d = nullptr;
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_APP_LAUNCHER_H
