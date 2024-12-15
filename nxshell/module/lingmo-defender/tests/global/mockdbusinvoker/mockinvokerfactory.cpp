// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mockinvokerfactory.h"
#include "mockinvoker.h"
#include "../mockgsettings/mockgsettings.h"
#include <QDBusMessage>

MockDbusInvokerFactory::MockDbusInvokerFactory()
{
}

DBusInvokerInterface *MockDbusInvokerFactory::CreateInvoker(const QString &service, const QString &path, const QString &interface, ConnectType type, QObject *parent)
{
    if (!parent) {
        parent = this;
    }
    auto it = m_intefaceMap.find(interface);
    if (it != m_intefaceMap.end()) {
        return it.value();
    }

    return new MockDbusInvokerInterface(service, path, interface, type, parent);
}

SettingsInvokerInterface *MockDbusInvokerFactory::CreateSettings(const QByteArray &schema_id, const QByteArray &path, QObject *parent)
{
    SettingsInvokerInterface *rst = nullptr;
    if (!parent) {
        parent = this;
    }

    if (m_settingsMap.find(schema_id) != m_settingsMap.end()) {
        return m_settingsMap[schema_id];
    }

    m_settingsMap.find(schema_id) != m_settingsMap.end() ? rst = m_settingsMap[schema_id] : rst = new MockGSettingInterface(schema_id, path, parent);

    return rst;
}

void MockDbusInvokerFactory::setInvokerInteface(const QString &interfaceName, DBusInvokerInterface *interface)
{
    m_intefaceMap[interfaceName] = interface;
}

void MockDbusInvokerFactory::setSettinsInteface(const QString &settingName, SettingsInvokerInterface *interface)
{
    m_settingsMap[settingName] = interface;
}
