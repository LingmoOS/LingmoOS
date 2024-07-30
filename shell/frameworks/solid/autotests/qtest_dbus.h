/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_QTEST_DBUS_H
#define SOLID_QTEST_DBUS_H

#include <QDebug>
#include <QProcess>
#include <QTest>
#include <stdlib.h>

// clang-format off
#define QTEST_GUILESS_MAIN_SYSTEM_DBUS(TestObject) \
int main(int argc, char *argv[]) \
{ \
    QProcess dbus; \
    dbus.start("dbus-launch", QStringList()); \
    dbus.waitForFinished(10000);    \
    QByteArray session = dbus.readLine(); \
    if (session.isEmpty()) { \
        qFatal("Couldn't execute new dbus session"); \
    } \
    int pos = session.indexOf('='); \
    setenv("DBUS_SYSTEM_BUS_ADDRESS", session.right(session.size() - pos - 1).trimmed(), 1); \
    qDebug() << session.right(session.size() - pos - 1).trimmed(); \
    session = dbus.readLine(); \
    pos = session.indexOf('='); \
    QByteArray pid = session.right(session.size() - pos - 1).trimmed(); \
    QCoreApplication app( argc, argv ); \
    app.setApplicationName( QLatin1String("qttest") ); \
    TestObject tc; \
    int result = QTest::qExec( &tc, argc, argv ); \
    dbus.start("kill", QStringList() << "-9" << pid); \
    dbus.waitForFinished(); \
    return result; \
}
#endif //SOLID_QTEST_DBUS_H
