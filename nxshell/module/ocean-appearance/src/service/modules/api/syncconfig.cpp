// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "syncconfig.h"
#include "../common/commondefine.h"

#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QDebug>
// 数据同步处理，和UOS ID同步数据
SyncConfig::SyncConfig(QString name, QString path)
    : name(name)
    , path(path)
{
    APPEARANCEDBUS.registerObject(path, this, QDBusConnection::ExportNonScriptableSlots);
    APPEARANCEDBUS.connect("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "NameOwnerChanged", this, SLOT(handleNameOwnerChanged(QString, QString, QString)));
    registerConfig();
}

QByteArray SyncConfig::Get()
{
    return nullptr;
}

void SyncConfig::Set(QByteArray)
{
}

void SyncConfig::registerConfig()
{
    QDBusMessage dbusMessage = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "NameHasOwner");
    dbusMessage << SYNCSERVICENAME;
    if (false == QDBusPendingReply<bool>(QDBusConnection::sessionBus().asyncCall(dbusMessage)))
        return;

    QDBusMessage syncMessage = QDBusMessage::createMethodCall(SYNCSERVICENAME, SYNCSERVICEPATH, SYNCSERVICEINTERFACE, "Register");
    syncMessage << name << path;
    QDBusConnection::sessionBus().asyncCall(syncMessage);
    qInfo() << "SyncConfig" << name << path;
}

void SyncConfig::handleNameOwnerChanged(QString name, QString oldOwner, QString newOwner)
{
    if (name == SYNCSERVICENAME && !newOwner.isEmpty()) {
        registerConfig();
    }
}
