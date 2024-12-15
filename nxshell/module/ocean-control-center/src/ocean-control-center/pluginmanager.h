// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QObject>
#include <QQmlContext>
#include <QStringList>
#include <QVector>

class QPluginLoader;
class QQmlComponent;
class QThreadPool;

namespace oceanuiV25 {
class OceanUIObject;
class OceanUIManager;
struct PluginData;

class PluginManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginManager(OceanUIManager *parent);
    ~PluginManager();
    void loadModules(OceanUIObject *root, bool async, const QStringList &dirs);
    bool loadFinished() const;
    void beginDelete();

    inline bool isDeleting() const { return m_isDeleting; }

public Q_SLOTS:
    void cancelLoad();
    void updatePluginStatus(PluginData *plugin, uint status, const QString &log = QString());

Q_SIGNALS:
    void addObject(OceanUIObject *obj);
    void loadAllFinished();

    void pluginEndStatusChanged(PluginData *plugin);

private:
    bool compareVersion(const QString &targetVersion, const QString &baseVersion);
    QThreadPool *threadPool();

private Q_SLOTS:
    void loadPlugin(PluginData *plugin);
    void loadMetaData(PluginData *plugin);
    void loadModule(PluginData *plugin);
    void loadMain(PluginData *plugin);
    void createModule(QQmlComponent *component);
    void createMain(QQmlComponent *component);
    void addMainObject(PluginData *plugin);

    void moduleLoading();
    void mainLoading();

    void onHideModuleChanged(const QSet<QString> &hideModule);
    void onVisibleToAppChanged(bool visibleToApp);

private:
    OceanUIManager *m_manager;
    QList<PluginData *> m_plugins; // cache for other plugin
    OceanUIObject *m_rootModule;       // root module from MainWindow
    QThreadPool *m_threadPool;
    bool m_isDeleting;
};

} // namespace oceanuiV25
