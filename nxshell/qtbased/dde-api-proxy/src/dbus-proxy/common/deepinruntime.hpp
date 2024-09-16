// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QVector>
#include <QDBusObjectPath>
#include <QDBusObjectPath>
#include <QDBusVariant>
#include <QProcess>
#include <QDebug>
#include <QStringList>

class DeepinRuntime {
public:
    static bool CheckStartddeSession()
    {
        QVector<uint> pids = getStartddePID();
        for (uint pid : pids) {
            QDBusInterface systemd("org.freedesktop.systemd1", "/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager", QDBusConnection::systemBus());
            QDBusReply<QDBusObjectPath> reply = systemd.call("GetUnitByPID", pid);
            if (!reply.isValid()) {
                continue;
            }
            qInfo() << "startdde unit:" << reply.value().path();
            if (reply.value().path().isEmpty()) {
                continue;
            }
            QDBusInterface unit("org.freedesktop.systemd1", reply.value().path(), "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
            QDBusReply<QDBusVariant> replyId = unit.call("Get", "org.freedesktop.systemd1.Unit", "Id");
            if (!replyId.isValid()) {
                continue;
            }
            QString id = replyId.value().variant().toString();
            if (!id.endsWith("service")) {
                qWarning() << "check startdde error: is not a service.";
                return false;
            }
        }
        return true;
    }
private:
    static QVector<uint> getStartddePID() 
    {
        QProcess process;
        process.setProgram("pidof");
        QStringList argument("startdde");
        process.setArguments(argument);
        process.start();
        process.waitForFinished();
        QStringList pids = QString::fromLocal8Bit(process.readAllStandardOutput()).simplified().split(" ");
        QVector<uint> ret;
        for (auto pid : pids) {
            if (!pid.isEmpty()) {
                ret.append(pid.toUInt());
            }
        }
        qDebug() << "Startdde Pids:" << ret;
        return ret;
    }
};



