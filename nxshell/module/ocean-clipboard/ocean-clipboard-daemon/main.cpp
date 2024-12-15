// Copyright (C) 2019 ~ 2022 Uniontech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QGuiApplication>
#include <QDBusError>
#include <QDBusConnection>

#include <DLog>

#include "clipboarddaemon.h"

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);
    a.setOrganizationName("lingmo");
    a.setApplicationName("ocean-clipboard-daemon");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    const QString interface = "org.lingmo.ocean.daemon.Clipboard1";
    const QString path = "/org/lingmo/ocean/daemon/Clipboard1";
    if (!QDBusConnection::sessionBus().registerService(interface)) {
        qWarning() << "DBus register failed, error message:" << QDBusConnection::sessionBus().lastError().message();
        exit(-1);
    }

    ClipboardDaemon daemon;
    QDBusConnection::sessionBus().registerObject(path, &daemon, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
    qDebug() << "Everything is ok!";

    return a.exec();
}
