// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmanager.h"
#include "pluginsiteminterface_v2.h"
#include "widgetplugin.h"

#include <QFileInfo>
#include <QPluginLoader>

PluginManager::PluginManager(QObject *parent)
    : QObject(parent)
{
}

void PluginManager::setPluginPaths(const QStringList &paths)
{
    m_pluginPaths = paths;
}

bool PluginManager::loadPlugins()
{
    for (const QString &path : m_pluginPaths) {
        QFileInfo pathInfo(path);

        if (!pathInfo.exists()) {
            qWarning() << "Path does not exist:" << path;
            continue;
        }

        if (pathInfo.isDir()) {
            loadPluginsFromDir(path);
        } else if (pathInfo.isFile()) {
            loadPlugin(path);
        } else {
            qWarning() << "Unsupported path type:" << path;
        }
    }

    return !m_loadedPlugins.isEmpty();
}

QVector<PluginsItemInterface *> PluginManager::loadedPlugins() const
{
    return m_loadedPlugins;
}

void PluginManager::loadPlugin(const QString &pluginFilePath)
{
    auto *pluginLoader = new QPluginLoader(pluginFilePath, this);
    const QJsonObject &meta = pluginLoader->metaData().value("MetaData").toObject();
    const QString &pluginApi = meta.value("api").toString();
    Q_UNUSED(pluginApi);

    if (!pluginLoader->load()) {
        qWarning() << "Failed to load plugin:" << pluginFilePath << pluginLoader->errorString();
        pluginLoader->deleteLater();
        return;
    }

    QObject *instance = pluginLoader->instance();
    PluginsItemInterface *interface = qobject_cast<PluginsItemInterfaceV2 *>(instance);
    if (!interface) {
        interface = qobject_cast<PluginsItemInterface *>(instance);
    }

    if (!interface) {
        qWarning() << "Failed to get valid interface from plugin:" << pluginFilePath;
        pluginLoader->unload();
        pluginLoader->deleteLater();
        return;
    }

    m_loadedPlugins.append(interface);
    qDebug() << "Loaded plugin:" << interface->pluginName();

    (void)new dock::WidgetPlugin(interface, this);
}

void PluginManager::loadPluginsFromDir(const QString &dirPath)
{
    QDir pluginDir(dirPath);
    if (!pluginDir.exists()) {
        qWarning() << "Directory does not exist:" << dirPath;
        return;
    }

    QFileInfoList entries = pluginDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &entry : entries) {
        if (entry.isDir()) {
            loadPluginsFromDir(entry.absoluteFilePath());
        } else {
            if (entry.suffix() != "so") continue;
            loadPlugin(entry.absoluteFilePath());
        }
    }
}
