// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display1adaptor.h"
#include "display.h"

#include <QGuiApplication>
#include <DLog>

#include <systemd/sd-daemon.h>

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    app.setOrganizationName("lingmo");
    app.setApplicationName("ocean-display");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    Display1 *display = new Display1();
    new Display1Adaptor(display);

    QDBusConnection::sessionBus().registerService("org.lingmo.ocean.Display1");
    QDBusConnection::sessionBus().registerObject("/org/lingmo/ocean/Display1", "org.lingmo.ocean.Display1", display);

    sd_notify(0, "READY=1");

    return app.exec();
}
