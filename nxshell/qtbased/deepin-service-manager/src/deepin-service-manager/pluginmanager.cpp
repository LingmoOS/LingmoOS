// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "pluginmanager.h"

#include "pluginloader.h"
#include "utils.h"

#include <QDBusInterface>
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(dsm_PluginManager, "[PluginManager]")

PluginManager::PluginManager(QObject *parent)
    : QObject(parent)
{
}

void PluginManager::init(const QDBusConnection::BusType &type)
{
    m_type = type;
    // Register service
    QDBusConnection connection = m_type == QDBusConnection::SessionBus
            ? QDBusConnection::sessionBus()
            : QDBusConnection::systemBus();
    if (!connection.registerObject(PluginManagerPath,
                                   this,
                                   QDBusConnection::ExportScriptableContents
                                           | QDBusConnection::ExportAllProperties)) {
        qCWarning(dsm_PluginManager)
                << "failed to register dbus object: " << connection.lastError().message();
    }
}

void PluginManager::loadByGroup(const QString &group)
{
    QDBusConnection connection = m_type == QDBusConnection::SessionBus
            ? QDBusConnection::sessionBus()
            : QDBusConnection::systemBus();
    QDBusInterface remote(ServiceManagerName,
                          ServiceManagerPrivatePath,
                          ServiceManagerInterface,
                          connection);
    remote.call("RegisterGroup", group, connection.baseService());

    PluginLoader *loader = new PluginLoader(this);
    connect(loader, &PluginLoader::PluginAdded, this, [this](const QString &name) {
        m_plugins.append(name);
        Q_EMIT PluginAdded(name);
    });
    connect(loader, &PluginLoader::PluginRemoved, this, [this](const QString &name) {
        m_plugins.removeOne(name);
        Q_EMIT PluginRemoved(name);
    });
    loader->init(m_type, true);
    loader->loadByGroup(group);
}

void PluginManager::loadByName(const QString &name)
{
    PluginLoader *loader = new PluginLoader(this);
    loader->init(m_type, false);
    const QString &group = loader->getGroup(name);
    if (group.isEmpty()) {
        qCWarning(dsm_PluginManager) << "failed to get group of plugin: " << name;
        return;
    }
    QDBusConnection connection = m_type == QDBusConnection::SessionBus
            ? QDBusConnection::sessionBus()
            : QDBusConnection::systemBus();
    QDBusInterface remote(ServiceManagerName,
                          ServiceManagerPrivatePath,
                          ServiceManagerInterface,
                          connection);
    remote.call("RegisterGroup", loader->getGroup(name), connection.baseService());

    connect(loader, &PluginLoader::PluginAdded, this, [this](const QString &name) {
        m_plugins.append(name);
        Q_EMIT PluginAdded(name);
    });
    connect(loader, &PluginLoader::PluginRemoved, this, [this](const QString &name) {
        m_plugins.removeOne(name);
        Q_EMIT PluginRemoved(name);
    });
    loader->loadByName(name);
}

QStringList PluginManager::plugins() const
{
    return m_plugins;
}
