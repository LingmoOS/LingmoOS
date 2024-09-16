// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QtDBus/QtDBus>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QFile>

#include "signal_handler.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCoreApplication::setApplicationName("wloutput-interface-test");

    QDBusInterface interface("org.deepin.dde.KWayland1", "/org/deepin/dde/KWayland1/Output",
                             "org.deepin.dde.KWayland1.Output",
                             QDBusConnection::sessionBus());
    if(!interface.isValid())
    {
        qDebug() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        exit(1);
    }

    signal_handler sh;
    QObject::connect(&interface, SIGNAL(OutputAdded(QString)),
                     &sh, SLOT(onOutputAdded(QString)));

    QObject::connect(&interface, SIGNAL(OutputRemoved(QString)),
                     &sh, SLOT(onOutputRemoved(QString)));


    QObject::connect(&interface, SIGNAL(OutputChanged(QString)),
                     &sh, SLOT(onOutputAdded(QString)));

    QDBusReply<QString> reply;

    reply = interface.call("ListOutput");
    if (reply.isValid())
    {
        qDebug() << "call ListOutput()";
        qDebug() << "replay: " << reply.value();

        QFile file("./1.txt");
        file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        file.write(reply.value().toUtf8());
        file.close();

    }

    reply = interface.call("GetOutput", "e369853df7");
    if (reply.isValid())
    {
        qDebug() << "call GetOutput()";
        qDebug() << "replay: " << reply.value();
    }

    QFile file("./1.txt");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray t = file.readAll();
    file.close();

    interface.call("Apply", QString(t));

    qDebug() << "well done";

    return a.exec();
}
