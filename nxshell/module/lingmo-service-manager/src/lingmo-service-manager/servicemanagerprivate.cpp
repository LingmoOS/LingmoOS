// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "servicemanagerprivate.h"

#include <QDebug>

ServiceManagerPrivate::ServiceManagerPrivate(QObject *parent)
    : QDBusService(parent)
{
}

ServiceManagerPrivate::~ServiceManagerPrivate() { }

void ServiceManagerPrivate::RegisterGroup(const QString &groupName, const QString &serviceName)
{
    emit requestRegisterGroup(groupName, serviceName);
}

void ServiceManagerPrivate::init(const QDBusConnection::BusType &type)
{
    initPolicy(type, QString(SERVICE_CONFIG_DIR) + "other/manager.json");
}
