// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "servicemanagerpublic.h"

ServiceManagerPublic::ServiceManagerPublic(QObject *parent)
    : QDBusService(parent)
{
}

ServiceManagerPublic::~ServiceManagerPublic() { }

void ServiceManagerPublic::addGroup(const QString &groupName)
{
    if (!m_groups.contains(groupName)) {
        m_groups.append(groupName);
    }
}

void ServiceManagerPublic::init(const QDBusConnection::BusType &type)
{
    initPolicy(type, QString(SERVICE_CONFIG_DIR) + "other/manager.json");
}

QStringList ServiceManagerPublic::groups() const
{
    return m_groups;
}

QString ServiceManagerPublic::version() const
{
    return "1.0";
}
