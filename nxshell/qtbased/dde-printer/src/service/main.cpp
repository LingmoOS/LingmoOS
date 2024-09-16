// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCoreApplication>

#include <QDebug>
#include <QDBusError>
#include <QDBusConnection>

#include "service.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-printer-manager");

    auto bus = QDBusConnection::systemBus();

    Service v;
    if (!bus.registerService(SERVICE_MANAGER_INTERFACE_NAME)) {
        qWarning() << "Register QDBus Service Error " << bus.lastError();
        exit(0x0003);
    }

    QDBusConnection::RegisterOptions opts =
        QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals;

    if (!bus.registerObject(SERVICE_MANAGER_INTERFACE_PATH, &v, opts)) {
        qWarning() << "registerObject Error" << bus.lastError();
        exit(0x0003);
    }

    int ret = app.exec();

    bus.unregisterService(SERVICE_MANAGER_INTERFACE_NAME);
    bus.unregisterObject(SERVICE_MANAGER_INTERFACE_PATH);
    bus.disconnectFromBus(bus.name());

    return ret;
}
