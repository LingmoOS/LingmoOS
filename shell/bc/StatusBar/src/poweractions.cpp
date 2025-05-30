/*
 * Copyright (C) 2025 Lingmo OS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 * Author:     Lingmo OS Team <team@lingmo.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "poweractions.h"
#include <QCommandLineParser>
#include <QDBusInterface>
#include <QApplication>
#include <QProcess>

const static QString s_dbusName = "com.lingmo.Session";
const static QString s_pathName = "/Session";
const static QString s_interfaceName = "com.lingmo.Session";
const static QString s_ksmserver = "org.kde.ksmserver";
const static QString s_kpathName = "/KSMServer";
const static QString s_kinterfaceName = "org.kde.KSMServerInterface";


PowerActions::PowerActions(QObject *parent)
    : QObject(parent)
{

}

void PowerActions::shutdown()
{
    QDBusInterface iface(s_dbusName, s_pathName, s_interfaceName, QDBusConnection::sessionBus());
    if (iface.isValid()) {
        iface.call("powerOff");
    }
}

void PowerActions::logout()
{
    // QDBusInterface iface(s_dbusName, s_pathName, s_interfaceName, QDBusConnection::sessionBus());
    // if (iface.isValid()) {
    //     iface.call("logout");
    // }
    QDBusInterface ksmserver(s_ksmserver, s_kpathName, s_kinterfaceName, QDBusConnection::sessionBus());
    QDBusMessage reply = ksmserver.call("logout", 0, 0, 0);
}

void PowerActions::reboot()
{
    QDBusInterface iface(s_dbusName, s_pathName, s_interfaceName, QDBusConnection::sessionBus());
    if (iface.isValid()) {
        iface.call("reboot");
    }
}

void PowerActions::lockScreen()
{
    QProcess::startDetached("lingmo-screenlocker", QStringList());
}

void PowerActions::suspend()
{
    QDBusInterface iface(s_dbusName, s_pathName, s_interfaceName, QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.call("suspend");
    }
}
