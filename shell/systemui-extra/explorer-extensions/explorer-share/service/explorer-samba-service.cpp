/*
 * Copyright (C) 2022, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Ding Jing <dingjing@kylinos.cn>
 *
 */

#include <QCoreApplication>

#include <gio/gio.h>
#include "samba-config.h"

#include <QDebug>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

static GDBusObjectManagerServer* manager = NULL;

static bool registManager (SambaConfig& sc);

int main (int argc, char* argv[])
{
    int ret = -1;
    QCoreApplication app(argc, argv);

    SambaConfig* sc = const_cast<SambaConfig*>(SambaConfig::getInstance());
    if (registManager(*sc)) {
        ret = app.exec();
    }

    return ret;
}


static bool registManager (SambaConfig& sc)
{
    QDBusConnection bus = QDBusConnection::systemBus();
    if (bus.interface()->isServiceRegistered(DBUS_NAME)) {
        qWarning() << "dbus: " DBUS_NAME " already regist!";
        return false;
    }

    if (!bus.registerService(DBUS_NAME)) {
        qWarning() << "dbus: " DBUS_NAME " regist failed!";
        return false;
    }

    if (!bus.registerObject(DBUS_PATH, DBUS_NAME, static_cast<QObject*>(&sc), QDBusConnection::ExportNonScriptableSlots|QDBusConnection::ExportNonScriptableSignals)) {
        qWarning() << "dbus: " DBUS_NAME " regist object failed!";
        return false;
    }

    qDebug() << "dbus: " DBUS_NAME " regist successed!";

    return true;
}
