/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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

#include "kynmsystemdbus.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

int main(int argc, char *argv[]){

    QCoreApplication app(argc, argv);

    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.registerService("com.lingmo.network.qt.systemdbus")){
        qCritical() << "QDbus register service failed reason:" << systemBus.lastError();
        exit(1);
    }

    if (!systemBus.registerObject("/", new KynmSystemDbus(), QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)){
        qCritical() << "QDbus register object failed reason:" << systemBus.lastError();
        exit(2);
    }

    return app.exec();
}
