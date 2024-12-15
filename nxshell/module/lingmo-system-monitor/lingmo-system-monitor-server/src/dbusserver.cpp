// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusserver.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QProcess>
#include <QDebug>
#include <QTimer>
#include <QCoreApplication>

DBusServer::DBusServer(QObject *parent)
    : QObject(parent)
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (dbus.registerService("com.lingmo.SystemMonitorServer")) {
        QDBusConnection::RegisterOptions opts =
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals |
            QDBusConnection::ExportAllProperties;
        dbus.registerObject("/com/lingmo/SystemMonitorServer", this, opts);
    }
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, [ = ]() { qApp->exit(0); });
}

void DBusServer::exitDBusServer(int msec)
{
    qApp->processEvents();
    m_timer.start(msec);
}

void DBusServer::showCpuAlarmNotify(const QString &argument)
{
    QProcess::startDetached("/usr/bin/lingmo-system-monitor", QStringList() << "alarm" << "cpu" << argument);
    exitDBusServer(8000);
}

void DBusServer::showMemoryAlarmNotify(const QString &argument)
{
    QProcess::startDetached("/usr/bin/lingmo-system-monitor", QStringList() << "alarm" << "memory" << argument);
    exitDBusServer(8000);
}

void DBusServer::showLingmoSystemMoniter()
{
    // 显示系统监视器
    auto launchProcessByAM = [](){
        QDBusMessage message = QDBusMessage::createMethodCall(
                "org.desktopspec.ApplicationManager1",
                "/org/desktopspec/ApplicationManager1/lingmo_2dsystem_2dmonitor",
                "org.desktopspec.ApplicationManager1.Application",
                "Launch");

        message << QString("") << QStringList() << QVariantMap();

        QDBusMessage reply = QDBusConnection::sessionBus().call(message);
        if (reply.type() != QDBusMessage::ReplyMessage) {
            qWarning() << "Launch lingmo-system-monitor main process error:" << reply.errorMessage();
            return;
        }
    };
    launchProcessByAM();
    // QString cmd("qdbus com.lingmo.SystemMonitorMain /com/lingmo/SystemMonitorMain com.lingmo.SystemMonitorMain.slotRaiseWindow");
    QString cmd("gdbus call -e -d  com.lingmo.SystemMonitorMain -o /com/lingmo/SystemMonitorMain -m com.lingmo.SystemMonitorMain.slotRaiseWindow");
    QTimer::singleShot(100, this, [ = ]() {
        QProcess::startDetached(cmd);
        qApp->processEvents();
        exitDBusServer(8000);
    });
}
