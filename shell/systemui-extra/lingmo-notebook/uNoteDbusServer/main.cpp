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

#include <QCoreApplication>
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QDBusError>
#include <QDBusMetaType>
#include <QDBusMessage>

#include "uNoteDbusServer.h"
#include "note_adaptor.h"

int main(int argc, char *argv[])
{

    QCoreApplication a(argc, argv);
    QDBusConnection sessionDbus = QDBusConnection::sessionBus();
    if(!sessionDbus.registerService("org.lingmo.note")) {
        qCritical() << "QDbus register service failed reason:" << sessionDbus.lastError().message();
        //exit(1);
    }
    UNoteDbusServer *dbus = new UNoteDbusServer();
    new InterfaceAdaptor(dbus);
    if(!sessionDbus.registerObject("/org/lingmo/note", dbus)) {
        qCritical() << "QDbus register object failed reason:" << sessionDbus.lastError().message();
    }

    return a.exec();
}

