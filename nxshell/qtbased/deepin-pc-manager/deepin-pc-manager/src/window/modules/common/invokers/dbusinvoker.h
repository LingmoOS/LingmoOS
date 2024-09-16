// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUS_INVOKER_H
#define DBUS_INVOKER_H

#include "invokerinterface.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusReply>

// interface
class DBusInvoker : public QObject, public DBusInvokerInterface
{
    Q_OBJECT
public:
    DBusInvoker(const QString &service,
                const QString &path,
                const QString &interface,
                ConnectType type = ConnectType::SESSION,
                QObject *parent = nullptr);

    virtual QDBusMessage Invoke(const QString &name,
                                const QList<QVariant> &functionParams = QList<QVariant>(),
                                BlockMode mode = BlockMode::BLOCKWITHGUI) override;
    virtual bool EmitSignal(const QString &name,
                            const QList<QVariant> &arguments = QList<QVariant>()) override;
    virtual bool Connect(const QString &signal, QObject *reciver, const char *slot) override;
    virtual bool Disconnect(const QString &signal, QObject *reciver, const char *slot) override;
    virtual ~DBusInvoker() override;

private:
    QString m_service;
    QString m_path;
    QString m_interface;
    ConnectType m_type;
    QDBusConnection m_connection;

private:
    DBusInvoker(const DBusInvoker &);
    DBusInvoker &operator=(const DBusInvoker &);
};

#endif
