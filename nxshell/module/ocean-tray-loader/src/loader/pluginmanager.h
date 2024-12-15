// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class PluginsItemInterface;

class PluginManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginManager(QObject *parent = nullptr);

    void setPluginPaths(const QStringList &paths);
    bool loadPlugins();
    QVector<PluginsItemInterface *> loadedPlugins() const;

private:
    void loadPlugin(const QString &pluginFilePath);
    void loadPluginsFromDir(const QString &dirPath);

private:
    QStringList m_pluginPaths;
    QVector<PluginsItemInterface *> m_loadedPlugins;
};
