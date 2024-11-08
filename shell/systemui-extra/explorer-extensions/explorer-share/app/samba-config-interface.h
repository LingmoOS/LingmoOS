/*
 * Copyright (C) 2022, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: Ding Jing <dingjing@kylinos.cn>
 *
 */

#ifndef SAMBACONFIGINTERFACE_H
#define SAMBACONFIGINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.lingmo.samba.config.share
 */
class SambaConfigInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.lingmo.samba.share.config"; }

public:
    SambaConfigInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~SambaConfigInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> finished()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QStringLiteral("finished"), argumentList);
    }

    inline QDBusPendingReply<bool> hasPasswd()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QStringLiteral("hasPasswd"), argumentList);
    }

    inline QDBusPendingReply<bool> init(const QString &name, int pid, int uid)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(name) << QVariant::fromValue(pid) << QVariant::fromValue(uid);
        return callWithArgumentList(QDBus::Block, QStringLiteral("init"), argumentList);
    }

    inline QDBusPendingReply<bool> setPasswd(const QString &passwd)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(passwd);
        return callWithArgumentList(QDBus::Block, QStringLiteral("setPasswd"), argumentList);
    }

Q_SIGNALS: // SIGNALS 
};
#endif
