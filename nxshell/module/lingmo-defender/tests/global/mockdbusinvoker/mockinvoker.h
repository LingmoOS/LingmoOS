// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOCK_DBUS_INVOKER_H
#define MOCK_DBUS_INVOKER_H

#include "invokers/invokerinterface.h"

#include <QObject>
#include <QDBusMessage>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class MockDbusInvokerInterface : public QObject
    , public DBusInvokerInterface
{
    Q_OBJECT
public:
    explicit MockDbusInvokerInterface(const QString &service = "", const QString &path = "", const QString &interface = "", ConnectType type = ConnectType::SESSION, QObject *parent = nullptr);
    virtual ~MockDbusInvokerInterface();

    virtual QDBusMessage Invoke(const QString &name, const QList<QVariant> &functionParams = QList<QVariant>(), BlockMode mode = BlockMode::BLOCKWITHGUI);
    virtual bool EmitSignal(const QString &name, const QList<QVariant> &arguments = QList<QVariant>());
    virtual bool Connect(const QString &signal, QObject *reciver, const char *slot);
};

class MockDbusInvokerImpl : public MockDbusInvokerInterface
{
public:
    MOCK_METHOD3(Invoke, QDBusMessage(const QString &, const QList<QVariant> &, BlockMode));
    MOCK_METHOD2(EmitSignal, bool(const QString &, const QList<QVariant> &));
    MOCK_METHOD3(Connect, bool(const QString &, QObject *, const char *));
};
#endif
