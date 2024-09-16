// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUS_INVOKER_FACTORY
#define DBUS_INVOKER_FACTORY

#include "invokerinterface.h"

#include <QtDBus/QDBusReply>

// 简化DBUS调用方法的写法
#define DBUS_BLOCK_INVOKE(interface, funName, ...) \
    interface->Invoke(funName, { __VA_ARGS__ }, BlockMode::BLOCK)
#define DBUS_INVOKE(interface, funName, ...) \
    interface->Invoke(funName, { __VA_ARGS__ }, BlockMode::BLOCKWITHGUI)
#define DBUS_NOBLOCK_INVOKE(interface, funName, ...) \
    interface->Invoke(funName, { __VA_ARGS__ }, BlockMode::NOBLOCK)
#define GET_MESSAGE_VALUE(type, output, message) type output = QDBusReply<type>(message).value()

// implement
class InvokerFactory : public QObject, public InvokerFactoryInterface
{
    Q_OBJECT
public:
    static InvokerFactory &GetInstance()
    {
        static InvokerFactory instance;
        return instance;
    }

    virtual DBusInvokerInterface *CreateInvoker(const QString &service,
                                                const QString &path,
                                                const QString &interface,
                                                ConnectType type = ConnectType::SESSION,
                                                QObject *parent = nullptr) override;
    virtual SettingsInvokerInterface *CreateSettings(const QByteArray &schema_id,
                                                     const QByteArray &path = QByteArray(),
                                                     QObject *parent = nullptr) override;

public:
    void setInvokerInteface(const QString &, DBusInvokerInterface *);
    void setSettinsInteface(const QString &, SettingsInvokerInterface *);

private:
    InvokerFactory();
    InvokerFactory(InvokerFactory &);
    InvokerFactory &operator=(const InvokerFactory &);
    ~InvokerFactory() override;

    QMap<QString, DBusInvokerInterface *> m_intefaceMap;
    QMap<QString, SettingsInvokerInterface *> m_settingsMap;
};

#endif
