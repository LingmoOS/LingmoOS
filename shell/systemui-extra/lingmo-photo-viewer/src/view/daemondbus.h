/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */

#ifndef DAEMONIPCDBUS_H
#define DAEMONIPCDBUS_H

#define LINGMO_USER_GUIDE_PATH "/"

#define LINGMO_USER_GUIDE_SERVICE "com.lingmoUserGuide.hotel"

#define LINGMO_USER_GUIDE_INTERFACE "com.guide.hotel"

#define SERVICE_NAME_SIZE 100

#include <QObject>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <unistd.h>
#include <sys/types.h>

class DaemonDbus : public QObject
{
    Q_OBJECT
    // 定义Interface名称为com.scorpio.test.value
    // Q_CLASSINFO("D-Bus Interface", "com.scorpio.test.value")
public:
    DaemonDbus() {}

public Q_SLOTS:

    int daemonIsNotRunning();
    void showGuide(QString appName);
};

/*
// 使用方法
DaemonIpcDbus *mDaemonIpcDbus;
mDaemonIpcDbus = new DaemonIpcDbus();
if (!mDaemonIpcDbus->daemonIsNotRunning()){
    //增加标题栏帮助菜单、F1快捷键打开用户手册
    mDaemonIpcDbus->showGuide("lingmo-ipmsg");
}
*/

#endif // DAEMONIPCDBUS_H
