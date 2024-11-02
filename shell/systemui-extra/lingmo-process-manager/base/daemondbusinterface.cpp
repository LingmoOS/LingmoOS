/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "daemondbusinterface.h"
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusConnection>
#include <QApplication>
#include <QDebug>
#include <iostream>
#include "misc.h"

namespace {

const char *daemon_dbus_service = "com.lingmo.ProcessManagerDaemon";
const char *daemon_dbus_path = "/com/lingmo/ProcessManagerDaemon";
const char *daemon_dbus_interface = "com.lingmo.ProcessManagerDaemon";

}

DaemonDbusInterface::DaemonDbusInterface(QObject* parent)
    : QObject(parent)
    , dbusInterface(daemon_dbus_service, daemon_dbus_path,
                    daemon_dbus_interface, QDBusConnection::systemBus())
{
    connect(&dbusInterface, SIGNAL(ResourceThresholdWarning(QString,int)),
            this, SLOT(onResourceWarning(QString,int)));
}

void DaemonDbusInterface::onResourceWarning(const QString& resource, int level)
{
    qDebug() << "Get resource warning:" << resource << "warning level:" << level;
    Q_EMIT ResourceThresholdWarning(resource.toStdString(), level);
}

void DaemonDbusInterface::createProcessGroup(
    const std::string &path, const std::vector<std::string> &controllers,
    const std::vector<int> &pids)
{
    qDebug() << "DaemonDbusInterface::createProcessGroup: "
             << QString::fromStdString(path)
             << misc::stl2qt::vectorString2QStringList(controllers) << pids.size();

    QList<QVariant> argumentList;
    argumentList
        << QVariant::fromValue(QString::fromStdString(path))
        << QVariant::fromValue(misc::stl2qt::vectorString2QStringList(controllers))
        << QVariant::fromValue(misc::stl2qt::vector2QList<int>(pids));
    asyncCallWithArgumentList("CreateProcessGroup", argumentList);
}

void DaemonDbusInterface::moveProcessToGroup(
    const std::string &path, const std::vector<std::string> &controllers,
    const std::vector<int> &pids)
{
    qDebug() << "DaemonDbusInterface::moveProcessesToGroup: "
             << QString::fromStdString(path)
             << misc::stl2qt::vectorString2QStringList(controllers) << pids.size();

    QList<QVariant> argumentList;
    argumentList
        << QVariant::fromValue(QString::fromStdString(path))
        << QVariant::fromValue(misc::stl2qt::vectorString2QStringList(controllers))
        << QVariant::fromValue(misc::stl2qt::vector2QList<int>(pids));
    asyncCallWithArgumentList("MoveProcessToGroup", argumentList);
}

void DaemonDbusInterface::setProcessGroupResourceLimit(
    const std::string &path, const std::string &controller,
    const std::string &file, const std::string &value)
{
    qDebug() << "DaemonDbusInterface::setProcessGroupResourceLimit: "
             << QString::fromStdString(path)
             << QString::fromStdString(controller)
             << QString::fromStdString(file)
             << QString::fromStdString(value);

    QList<QVariant> argumentList;
    argumentList
        << QVariant::fromValue(QString::fromStdString(path))
        << QVariant::fromValue(QString::fromStdString(controller))
        << QVariant::fromValue(QString::fromStdString(file))
        << QVariant::fromValue(QString::fromStdString(value));
    asyncCallWithArgumentList("SetProcessGroupResourceLimit", argumentList);
}

void DaemonDbusInterface::setSystemdUnitPropertyEnabled(
    const std::string &unitName, const std::string &propertyName, bool enabled)
{
    qDebug() << "DaemonDbusInterface::setSystemdUnitPropertyEnabled: "
             << QString::fromStdString(unitName)
             << QString::fromStdString(propertyName)
             << enabled;
    QList<QVariant> argumentList;
    argumentList
        << QVariant::fromValue(QString::fromStdString(unitName))
        << QVariant::fromValue(QString::fromStdString(propertyName))
        << QVariant::fromValue(enabled);
    asyncCallWithArgumentList("SetSystemdUnitPropertyEnabled", argumentList);
}

double DaemonDbusInterface::getCpuEnergyConsumption()
{
    QDBusPendingReply<double> reply = callWithArgumentList("GetCpuEnergyConsumption", {});
    if (!reply.isValid()) {
        qWarning() << "GetCpuEnergyConsumption failed: " << reply.error();
        return 0;
    }
    return reply.value();
}

void DaemonDbusInterface::reclaimProcesses(const std::vector<int> &pids)
{
    qDebug() << "DaemonDbusInterface::reclaimProcesses: " << misc::stl2qt::vector2QList<int>(pids);

    QList<QVariant> argumentList = {QVariant::fromValue(misc::stl2qt::vector2QList<int>(pids))};
    asyncCallWithArgumentList("ReclaimProcesses", argumentList);
}

void DaemonDbusInterface::reclaimProcessGroups(const std::vector<std::string> &groupNames)
{
    qDebug() << "DaemonDbusInterface::reclaimProcesses: " << misc::stl2qt::vectorString2QStringList(groupNames);

    QList<QVariant> argumentList = {QVariant::fromValue(misc::stl2qt::vectorString2QStringList(groupNames))};
    asyncCallWithArgumentList("ReclaimProcessGroups", argumentList);
}

void DaemonDbusInterface::asyncCallWithArgumentList(const QString &method, const QList<QVariant> &args)
{
    QDBusPendingCall pcall = dbusInterface.asyncCallWithArgumentList(method, args);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, qApp,
                     [watcher, method](QDBusPendingCallWatcher *call) {
        QDBusPendingReply<> reply = *call;
        if (reply.isError()) {
            qWarning() << QString("Call daemon dbus %1 error: ").arg(method) << reply.error();
        }
        call->deleteLater();
        watcher->deleteLater();
    });
}

QDBusMessage DaemonDbusInterface::callWithArgumentList(
    const QString &method, const QList<QVariant> &args)
{
    return dbusInterface.callWithArgumentList(QDBus::Block, method, args);
}
