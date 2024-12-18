// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <ocean-cooperation-framework/ocean_cooperation_framework_global.h>
#include <ocean-cooperation-framework/lifecycle/plugin.h>
#include <ocean-cooperation-framework/lifecycle/pluginmetaobject.h>

#include <QPluginLoader>
#include <QSettings>
#include <QObject>
#include <QQueue>
#include <QSharedData>
#include <QDirIterator>
#include <QScopedPointer>

DPF_BEGIN_NAMESPACE

class PluginManagerPrivate;

class PluginManager : public QObject
{
    Q_OBJECT
    friend class PluginManagerPrivate;
    QSharedPointer<PluginManagerPrivate> d;

public:
    explicit PluginManager();
    QStringList pluginIIDs() const;
    QStringList pluginPaths() const;
    QStringList blackList() const;
    QStringList lazyLoadList() const;
    void addPluginIID(const QString &pluginIIDs);
    void addBlackPluginName(const QString &name);
    void addLazyLoadPluginName(const QString &name);
    void setPluginPaths(const QStringList &pluginPaths);

    bool readPlugins();
    bool loadPlugins();
    bool initPlugins();
    bool startPlugins();
    void stopPlugins();

    PluginMetaObjectPointer pluginMetaObj(const QString &pluginName,
                                          const QString version = "") const;

    bool loadPlugin(PluginMetaObjectPointer &pointer);
    bool initPlugin(PluginMetaObjectPointer &pointer);
    bool startPlugin(PluginMetaObjectPointer &pointer);
    bool stopPlugin(PluginMetaObjectPointer &pointer);
    bool isAllPluginsInitialized();
    bool isAllPluginsStarted();
};

DPF_END_NAMESPACE

#endif   // PLUGINMANAGER_H
