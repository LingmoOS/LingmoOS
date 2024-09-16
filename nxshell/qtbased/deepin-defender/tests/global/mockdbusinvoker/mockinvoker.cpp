// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mockinvoker.h"

MockDbusInvokerInterface::MockDbusInvokerInterface(const QString &service, const QString &path, const QString &interface, ConnectType type, QObject *parent)
    : QObject(parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(service);
    Q_UNUSED(path);
    Q_UNUSED(interface);
    Q_UNUSED(type);
}

QDBusMessage MockDbusInvokerInterface::Invoke(const QString &name, const QList<QVariant> &functionParams, BlockMode mode)
{
    Q_UNUSED(name);
    Q_UNUSED(functionParams);
    Q_UNUSED(mode);
    return QDBusMessage();
}

bool MockDbusInvokerInterface::EmitSignal(const QString &name, const QList<QVariant> &arguments)
{
    Q_UNUSED(name);
    Q_UNUSED(arguments);
    return true;
}

bool MockDbusInvokerInterface::Connect(const QString &signal, QObject *reciver, const char *slot)
{
    Q_UNUSED(signal);
    Q_UNUSED(reciver);
    Q_UNUSED(slot);
    return true;
}

MockDbusInvokerInterface::~MockDbusInvokerInterface()
{
}
