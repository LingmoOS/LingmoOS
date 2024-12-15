// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "./dbus/lingmowmfaker.h"
#include "wmadaptor.h"

#include <DLog>

#include <QGuiApplication>

int main (int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("lingmo");
    app.setApplicationName("org.lingmo.ocean.lingmowmfaker");
    const QString sessionType = qEnvironmentVariable("XDG_SESSION_TYPE");
    if (sessionType == "wayland") {
        qWarning() << "ocean-fakewm does not support session type:" << sessionType;
        return -1;
    }
    LingmoWMFaker faker;
    WmAdaptor wmAdaptor(&faker);
    bool registerWmServiceSuccessed = QDBusConnection::sessionBus().registerService("com.lingmo.wm");
    bool registerWmObjectSuccessed = QDBusConnection::sessionBus().registerObject("/com/lingmo/wm", "com.lingmo.wm", &faker);
    if (!registerWmServiceSuccessed || !registerWmObjectSuccessed) {
        qWarning() << "wm dbus service already registered";
        return -1;
    }
    return app.exec();
}
