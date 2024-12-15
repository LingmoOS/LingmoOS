// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MANAGER_UTILS_H
#define MANAGER_UTILS_H

#include <QDBusConnection>

#define ServiceManagerInterface "org.lingmo.ServiceManager1"
#define PluginManagerInterface "org.lingmo.ServiceManager1.Plugin"
#define ServiceGroupInterface "org.lingmo.Group1"

static const QString &ServiceManagerName = QStringLiteral("org.lingmo.ServiceManager1");
static const QString &ServiceManagerPath = QStringLiteral("/org/lingmo/ServiceManager1");
static const QString &ServiceGroupPath = QStringLiteral("/org/lingmo/Group1/");
static const QString &ServiceManagerPrivatePath =
        QStringLiteral("/org/lingmo/ServiceManager1/Private");

static const QString &PluginManagerPath = QStringLiteral("/org/lingmo/ServiceManager1/Plugin");

static const QMap<QDBusConnection::BusType, QString> typeMap{
    { QDBusConnection::SystemBus, "system" }, { QDBusConnection::SessionBus, "user" }
};
#endif // MANAGER_UTILS_H
