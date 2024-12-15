// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUS_INVOKER_FACTORY
#define DBUS_INVOKER_FACTORY

#include "invokerinterface.h"

// implement
class InvokerFactory : public QObject
    , public InvokerFactoryInterface
{
    Q_OBJECT
public:
    InvokerFactory(QObject *parent = nullptr);
    virtual ~InvokerFactory() override {}
    virtual DBusInvokerInterface *CreateInvoker(const QString &service, const QString &path, const QString &interface, ConnectType type = ConnectType::SESSION, QObject *parent = nullptr) override;
    virtual SettingsInvokerInterface *CreateSettings(const QByteArray &schema_id, const QByteArray &path = QByteArray(), QObject *parent = nullptr) override;
};

#endif
