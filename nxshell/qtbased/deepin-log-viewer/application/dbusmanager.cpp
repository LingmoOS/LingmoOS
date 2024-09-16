// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusmanager.h"
#include "utils.h"

#include <unistd.h>
#include <pwd.h>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QLoggingCategory>
#include <QProcess>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logDBusManager, "org.deepin.log.viewer.dbus.manager")
#else
Q_LOGGING_CATEGORY(logDBusManager, "org.deepin.log.viewer.dbus.manager", QtInfoMsg)
#endif

/**
 * @brief DBusManager::DBusManager
 * dbus接口获取工具类
 * @param parent
 */
DBusManager::DBusManager(QObject *parent) : QObject(parent)
{

}

bool DBusManager::isSEOpen()
{
    bool bIsSEOpen = false;
    QDBusInterface interfaceSE("com.deepin.daemon.SecurityEnhance", "/com/deepin/daemon/SecurityEnhance", "com.deepin.daemon.SecurityEnhance", QDBusConnection::systemBus());
    if (interfaceSE.isValid()) {
        QDBusReply<QString> reply = interfaceSE.call(QStringLiteral("Status"));
        if (!reply.error().message().isEmpty())
            qCWarning(logDBusManager) << qPrintable(QString("com.deepin.daemon.SecurityEnhance.Status DBus error: %1").arg(reply.error().message()));

        if (reply.value() == "close")
            bIsSEOpen = false;
        else
            bIsSEOpen = true;
    } else {
        qCWarning(logDBusManager) << qPrintable(QString("isSEOpen failed! interface error: %1").arg(interfaceSE.lastError().message()));
    }

    return bIsSEOpen;
}

bool DBusManager::isAuditAdmin()
{
    bool bIsAuditAdmin = false;

    // 获取当前系统用户名
    struct passwd* pwd = getpwuid(getuid());
    QString currentUserName = pwd->pw_name;

    // 根据用户名判断用户身份，查看是否为审计管理员
    QDBusInterface interfaceSE("com.deepin.daemon.SecurityEnhance", "/com/deepin/daemon/SecurityEnhance", "com.deepin.daemon.SecurityEnhance", QDBusConnection::systemBus());
    if (interfaceSE.isValid()) {
        QDBusReply<QString> reply = interfaceSE.call(QStringLiteral("GetSEUserByName"), currentUserName);
        if (!reply.error().message().isEmpty())
            qCWarning(logDBusManager) << qPrintable(QString("com.deepin.daemon.SecurityEnhance.GetSEUserByName DBus error: %1").arg(reply.error().message()));

        if (reply.value() == "audadm_u" || reply.value() == "auditadm_u")
            bIsAuditAdmin = true;
    } else {
        qCWarning(logDBusManager) << qPrintable(QString("isAuditAdmin failed! interface error: %1").arg(interfaceSE.lastError().message()));
    }

    return bIsAuditAdmin;
}

QString DBusManager::getHomePathByFreeDesktop()
{
    QString homePath;
    QDBusInterface interface("org.freedesktop.login1", "/org/freedesktop/login1/user/self", "org.freedesktop.login1.User", QDBusConnection::systemBus());
    QString userName = qvariant_cast<QString>(interface.property("Name"));

    if (!userName.isEmpty())
        homePath = "/home/" + userName;

    return homePath;
}
