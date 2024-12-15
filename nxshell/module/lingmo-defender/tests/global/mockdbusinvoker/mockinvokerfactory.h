// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOCK_DBUS_INVOKER_FACTORY_H
#define MOCK_DBUS_INVOKER_FACTORY_H

#include "invokers/invokerinterface.h"

// 依照接口重写并提供虚方法
// 意味着调用时，即可以直接CreateInvoker
// 也可以必要时通过gmock多态返回期望的类指针
class MockDbusInvokerFactory : public QObject
    , public InvokerFactoryInterface
{
    Q_OBJECT
public:
    MockDbusInvokerFactory();
    virtual DBusInvokerInterface *CreateInvoker(const QString &service, const QString &path, const QString &interface, ConnectType type = ConnectType::SESSION, QObject *parent = nullptr) override;
    virtual SettingsInvokerInterface *CreateSettings(const QByteArray &schema_id, const QByteArray &path = QByteArray(), QObject *parent = nullptr) override;
    virtual ~MockDbusInvokerFactory() override {}

public:
    void setInvokerInteface(const QString &interfaceName, DBusInvokerInterface *);
    void setSettinsInteface(const QString &settingName, SettingsInvokerInterface *);

private:
    QMap<QString, DBusInvokerInterface *> m_intefaceMap;
    QMap<QString, SettingsInvokerInterface *> m_settingsMap;
};

#endif
