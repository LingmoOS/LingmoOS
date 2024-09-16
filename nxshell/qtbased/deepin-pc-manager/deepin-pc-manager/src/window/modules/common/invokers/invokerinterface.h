// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUS_INVOKE_INTERFACE_H
#define DBUS_INVOKE_INTERFACE_H

#include <QObject>
#include <QVariant>
#include <QtDBus/QDBusMessage>

enum ConnectType { SYSTEM, SESSION };

enum InvokeType { CALL, SIGNAL };

enum BlockMode { NOBLOCK, BLOCK, BLOCKWITHGUI, AUTODETECT };

// interface
class DBusInvokerInterface
{
public:
    virtual QDBusMessage Invoke(const QString &name,
                                const QList<QVariant> &functionParams = QList<QVariant>(),
                                BlockMode mode = BlockMode::BLOCKWITHGUI) = 0;
    virtual bool EmitSignal(const QString &name,
                            const QList<QVariant> &arguments = QList<QVariant>()) = 0;
    virtual bool Connect(const QString &signal, QObject *reciver, const char *slot) = 0;
    virtual bool Disconnect(const QString &signal, QObject *reciver, const char *slot) = 0;

    virtual ~DBusInvokerInterface() { }
};

class SettingsInvokerInterface
{
public:
    virtual QVariant GetValue(const QString &key) const = 0;
    virtual void SetValue(const QString &key, const QVariant &value) = 0;
    virtual void Reset(const QString &key) = 0;

    virtual ~SettingsInvokerInterface() { }
};

class InvokerFactoryInterface
{
public:
    virtual DBusInvokerInterface *CreateInvoker(const QString &service,
                                                const QString &path,
                                                const QString &interface,
                                                ConnectType type = ConnectType::SESSION,
                                                QObject *parent = nullptr) = 0;
    virtual SettingsInvokerInterface *CreateSettings(const QByteArray &schema_id,
                                                     const QByteArray &path = QByteArray(),
                                                     QObject *parent = nullptr) = 0;

    virtual ~InvokerFactoryInterface() { }
};

#endif
