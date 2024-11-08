/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#include <QCoreApplication>
#include <QObject>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QTimer>

#include "bluetooth.h"


/*
 * 测试情况：
 * setDeviceState(): 通过
 * getAllDevices():通过
 * enable():通过
 *
*/

int main(int argc, char *argv[])
{
    QCoreApplication app(argc,argv);
    
    BLUETOOTHDBUS::instance();

    QDBusConnection sessionBug = QDBusConnection::sessionBus();
    if (sessionBug.registerService("org.lingmo.bluetooth.test")) {
        sessionBug.registerObject("/bluetooth", BLUETOOTHDBUS::getInstance(),
                                 QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
    }
    app.exec();
    return 0;
}
