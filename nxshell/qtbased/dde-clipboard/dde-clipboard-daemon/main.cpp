// Copyright (C) 2019 ~ 2022 Uniontech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QDBusError>
#include <QDBusConnection>

#include <DLog>

#include "clipboarddaemon.h"

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("deepin");
    a.setApplicationName("dde-clipboard-daemon");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    const QString interface = "org.deepin.dde.daemon.Clipboard1";
    const QString path = "/org/deepin/dde/daemon/Clipboard1";
    if (!QDBusConnection::sessionBus().registerService(interface)) {
        qWarning() << "DBus register failed, error message:" << QDBusConnection::sessionBus().lastError().message();
        exit(-1);
    }

    ClipboardDaemon daemon;
    QDBusConnection::sessionBus().registerObject(path, &daemon, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
    qDebug() << "Everything is ok!";

    return a.exec();
}