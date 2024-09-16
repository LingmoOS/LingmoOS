// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "invokerfactory.h"
#include "dbusinvoker.h"
#include "deepindefendersettingsInvoker.h"

InvokerFactory::InvokerFactory(QObject *parent)
    : QObject(parent)
{
}

DBusInvokerInterface *InvokerFactory::CreateInvoker(const QString &service, const QString &path, const QString &interface, ConnectType type, QObject *parent)
{
    return new DBusInvoker(service, path, interface, type, parent);
}

SettingsInvokerInterface *InvokerFactory::CreateSettings(const QByteArray &schema_id, const QByteArray &path, QObject *parent)
{
    return new DeepinDefenderSettingsInvoker(schema_id, path, parent);
}
