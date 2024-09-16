// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ddlog.h"
#include "dbusalarmnotify.h"
#include "helper.hpp"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QProcess>
#include <QDebug>
#include <QDateTime>

using namespace DDLog;

#define AlarmMessageTimeOut 10000
QMutex DBusAlarmNotify::mutex;
QAtomicPointer<DBusAlarmNotify> DBusAlarmNotify::instance = nullptr;

DBusAlarmNotify &DBusAlarmNotify::getInstance()
{
    if (instance.testAndSetOrdered(nullptr, nullptr)) {
        QMutexLocker locker(&mutex);

        instance.testAndSetOrdered(nullptr, new DBusAlarmNotify);
    }
    return *instance;
}

void DBusAlarmNotify::showAlarmNotify(const QStringList &allArguments)
{
    if (allArguments[1].compare("cpu", Qt::CaseInsensitive) == 0) {
        bool isok = false;
        int cpuUsage = allArguments[2].toInt(&isok);
        if (isok) {
            QString topic(tr("Warning"));
            QString msg = QString(tr("Your CPU usage is higher than %1%!")).arg(cpuUsage);
            int timeout = AlarmMessageTimeOut;
            showAlarmNotify(topic, msg, timeout);
        }
    } else if (allArguments[1].compare("memory", Qt::CaseInsensitive) == 0) {
        bool isok = false;
        int memoryUsage = allArguments[2].toInt(&isok);
        if (isok) {
            QString topic(tr("Warning"));
            QString msg = QString(tr("Your memory usage is higher than %1%!")).arg(memoryUsage);
            int timeout = AlarmMessageTimeOut;
            showAlarmNotify(topic, msg, timeout);
        }
    }
}

void DBusAlarmNotify::showAlarmNotify(QString topic, QString msg, int timeout)
{
    QDBusMessage ddeNotify = QDBusMessage::createMethodCall(common::systemInfo().NotificationService,
                                                            common::systemInfo().NotificationPath,
                                                            common::systemInfo().NotificationInterface,
                                                            "Notify");
    QStringList action;
    action << "_open1" << tr("View");   //添加按钮
    QVariantMap inform;   //按钮的点击操作
    // 操作打开系统监视器
    QString openSystemMonitor = QString("qdbus,org.deepin.SystemMonitorDaemon,"
                                        "/org/deepin/SystemMonitorDaemon,"
                                        "org.deepin.SystemMonitorDaemon.showDeepinSystemMoniter");

    inform.insert(QString("x-deepin-action-_open1"), openSystemMonitor);

    QList<QVariant> ddeArgs;
    ddeArgs << QString("deepin-system-monitor");   // app name
    ddeArgs << uint(0);   // id = 0 不指定窗口 id
    ddeArgs << QString("deepin-system-monitor");   // icon
    ddeArgs << topic;   // notify topic
    ddeArgs << msg;   // notify msg body
    ddeArgs << action;   // button
    ddeArgs << inform;   // button operation
    ddeArgs << timeout;   // notify timeout

    ddeNotify.setArguments(ddeArgs);

    QDBusMessage replyMsg = QDBusConnection::sessionBus().call(ddeNotify);

    if (replyMsg.type() == QDBusMessage::ErrorMessage) {
        qCWarning(app) << __FUNCTION__ << __LINE__ << ", dde notify dbus method call fail , error name :"
                       << replyMsg.errorName() << " , error msg :" << replyMsg.errorMessage();
        QString cmd = QString("gdbus call -e -d  org.deepin.SystemMonitorDaemon -o /org/deepin/SystemMonitorDaemon -m org.deepin.SystemMonitorDaemon.setAlaramLastTimeInterval 0");
        QProcess::startDetached(cmd);
    } else {
        qint64 lastAlarmTimeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
        QString cmd = QString("gdbus call -e -d  org.deepin.SystemMonitorDaemon -o /org/deepin/SystemMonitorDaemon -m org.deepin.SystemMonitorDaemon.setAlaramLastTimeInterval %1").arg(lastAlarmTimeStamp);
        QProcess::startDetached(cmd);
    }
}

DBusAlarmNotify::DBusAlarmNotify(QObject *parent)
    : QObject(parent)
{
}
