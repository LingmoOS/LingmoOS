// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"
#include "singleton/singleapplication.h"

#include <dde-cooperation-framework/dpf.h>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

static constexpr char kPluginInterface[] { "org.deepin.plugin.daemon" };
static constexpr char kPluginCore[] { "daemon-core" };

using namespace deepin_cross;

static bool loadPlugins()
{
    QStringList pluginsDirs;
#ifdef QT_DEBUG
    const QString &pluginsDir { DDE_COOPERATION_PLUGIN_ROOT_DEBUG_DIR };
    qInfo() << QString("Load plugins path : %1").arg(pluginsDir);
    pluginsDirs.push_back(pluginsDir);
#else
    pluginsDirs << QString(DDE_COOPERATION_PLUGIN_ROOT_DIR);
    pluginsDirs << QString(DEEPIN_DAEMON_PLUGIN_DIR);
    pluginsDirs << QDir::currentPath() + "/plugins";
#endif
#if defined(WIN32)
    pluginsDirs << QCoreApplication::applicationDirPath();
#endif

    qInfo() << "Using plugins dir:" << pluginsDirs;
    //TODO(zhangs): use config
    static const QStringList kLazyLoadPluginNames {};
    QStringList blackNames;

    DPF_NAMESPACE::LifeCycle::initialize({ kPluginInterface }, pluginsDirs, blackNames, kLazyLoadPluginNames);

    qInfo() << "Depend library paths:" << QCoreApplication::libraryPaths();
    qInfo() << "Load plugin paths: " << dpf::LifeCycle::pluginPaths();

    // read all plugins in setting paths
    if (!DPF_NAMESPACE::LifeCycle::readPlugins())
        return false;

    // We should make sure that the core plugin is loaded first
    auto corePlugin = DPF_NAMESPACE::LifeCycle::pluginMetaObj(kPluginCore);
    if (corePlugin.isNull())
        return false;
    if (!corePlugin->fileName().contains(kPluginCore))
        return false;
    if (!DPF_NAMESPACE::LifeCycle::loadPlugin(corePlugin))
        return false;

    // load plugins without core
    if (!DPF_NAMESPACE::LifeCycle::loadPlugins())
        return false;

    return true;
}

int main(int argc, char *argv[])
{
    // qputenv("QT_LOGGING_RULES", "cooperation-daemon.debug=true");
    // qputenv("CUTEIPC_DEBUG", "1");

    deepin_cross::SingleApplication app(argc, argv);
    bool canSetSingle = app.setSingleInstance(app.applicationName());
    if (!canSetSingle) {
        qCritical() << app.applicationName() << "is already running.";
        return 0;
    }

    if (!loadPlugins()) {
        qCritical() << "load plugin failed";
        return -1;
    }

    int ret = app.exec();

    return ret;
}
