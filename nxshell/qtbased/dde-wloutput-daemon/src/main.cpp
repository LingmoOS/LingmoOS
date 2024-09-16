// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QGuiApplication>
#include <QDebug>
#include <QString>

#include <DLog>

#include "wloutput_interface.h"

const QString VERSION = "1.0.1";

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    a.setOrganizationName("deepin");
    a.setApplicationName("wloutput-interface");
    a.setApplicationVersion(VERSION); // app version

    DLogManager::registerFileAppender();
    DLogManager::registerConsoleAppender();

    wloutput_interface object(&a);
    Q_UNUSED(object);

    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.registerService(SERVER) ||
            !conn.registerObject(PATH, &a)) {
        qDebug() << "dbus service already registered!";
    }

//    if(!object.InitDBus())
//    {
//        qDebug() << "InitDBus failed!";
//        exit(-1);
//    }

    object.StartWork();

    qDebug() << "wloutput-interface";

    return a.exec();
}
