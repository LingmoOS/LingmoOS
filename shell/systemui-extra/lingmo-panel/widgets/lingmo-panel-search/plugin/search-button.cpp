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

#include "search-button.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDebug>
#include <QDBusReply>
#include <QProcess>

SearchButton::SearchButton(QObject *parent) : QObject(parent)
{
}

void SearchButton::LingmoUISearch()
{
    QDBusInterface interface(QStringLiteral("com.lingmo.search.service"), QStringLiteral("/"), QStringLiteral("org.lingmo.search.service"), QDBusConnection::sessionBus());
    interface.asyncCall(QStringLiteral("mainWindowSwitch"));
}

void SearchButton::openSearchSetting()
{
    bool res(false);
    QDBusInterface appLaunchInterface(QStringLiteral("com.lingmo.ProcessManager"),
                                      QStringLiteral("/com/lingmo/ProcessManager/AppLauncher"),
                                      QStringLiteral("com.lingmo.ProcessManager.AppLauncher"),
                                      QDBusConnection::sessionBus());
    if(!appLaunchInterface.isValid()) {
        qWarning() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        res = false;
    } else {
        appLaunchInterface.setTimeout(10000);
        QDBusReply<void> reply = appLaunchInterface.call("LaunchAppWithArguments", "/usr/share/applications/lingmo-control-center.desktop", QStringList{"-m", "search"});
        if(reply.isValid()) {
            res = true;
        } else {
            qWarning() << "ProcessManager dbus called failed!" << reply.error();
            res = false;
        }
    }
    if (!res) {
        //打开控制面板对应页面
        QProcess::startDetached("lingmo-control-center", {"-m", "search"});
    }
}
