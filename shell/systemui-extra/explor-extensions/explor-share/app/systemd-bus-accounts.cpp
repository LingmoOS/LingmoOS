/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#include "systemd-bus-accounts.h"
#include <QDebug>

SystemDbusAccounts::SystemDbusAccounts(QObject *parent) : QObject(parent)
{
    m_systemInterface = new QDBusInterface("org.freedesktop.Accounts",
                                           "/org/freedesktop/Accounts",
                                           "org.freedesktop.Accounts",
                                           QDBusConnection::systemBus());
//    connect(m_systemInterface, SIGNAL(UserAdded(QDBusObjectPath)), this, SLOT(create_user_success(QDBusObjectPath)));
//    connect(m_systemInterface, SIGNAL(UserDeleted(QDBusObjectPath)), this, SLOT(delete_user_success(QDBusObjectPath)));
}

SystemDbusAccounts::~SystemDbusAccounts()
{
    delete m_systemInterface;
    m_systemInterface = NULL;
}

QStringList SystemDbusAccounts::getListCachedUsers()
{
    QStringList userLists;
    QDBusReply<QList<QDBusObjectPath>> reply = m_systemInterface->call("ListCachedUsers");
    if (reply.isValid()) {
        for (QDBusObjectPath op : reply.value()) {
            userLists << op.path();
        }
    }
    return userLists;
}

QStringList SystemDbusAccounts::getAllUserNames()
{
    QStringList userNames;
    QStringList objectPaths = getListCachedUsers();
    if (!objectPaths.isEmpty()) {
        for (auto objectPath : objectPaths) {
            userNames << getUserName(objectPath);
        }
    }
    return userNames;
}

QString SystemDbusAccounts::getUserName(QString objectPath)
{
    QString userName;
    QDBusInterface *interFace = new QDBusInterface("org.freedesktop.Accounts",
                                                   objectPath,
                                                   "org.freedesktop.DBus.Properties",
                                                   QDBusConnection::systemBus());
    QDBusReply<QDBusVariant> reply = interFace->call("Get", "org.freedesktop.Accounts.User", "UserName");
    if (reply.isValid()) {
        userName = reply.value().variant().toString();
    } else {
        qDebug() << __func__ << "reply failed!";
    }
    return userName;
}

void SystemDbusAccounts::createUserSuccess(QDBusObjectPath objectPath)
{
    emit createUserDone(objectPath.path());
}

void SystemDbusAccounts::deleteUserSuccess(QDBusObjectPath objectPath)
{
    emit deleteUserDone(objectPath.path());
}
