// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginloader.h"

#include "applet.h"
#include "containment.h"
#include "pluginmetadata.h"
#include "pluginfactory.h"
#include "panel.h"

#include <dobject_p.h>
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QLoggingCategory>
#include <QPluginLoader>
#include <QStandardPaths>
#include <QThread>
#include <QFuture>
#include <QtConcurrent>

DS_BEGIN_NAMESPACE;

DCORE_USE_NAMESPACE

static constexpr auto MetaDataFileName{"metadata.json"};
static constexpr auto PluginSuffix{".so"};

Q_DECLARE_LOGGING_CATEGORY(dsLog)

Q_APPLICATION_STATIC(DPluginLoader, g_instance)

class DPluginLoaderPrivate : public DObjectPrivate
{
public:
    explicit DPluginLoaderPrivate(DPluginLoader *qq)
        : DObjectPrivate(qq)
    {
        m_pluginDirs = builtinPackagePaths();
    }
    void init()
    {
        D_Q(DPluginLoader);
        auto dirs = q->pluginDirs();
        // high priority for builtin plugin.
        auto builtinDirs = builtinPluginPaths();
        std::reverse(builtinDirs.begin(), builtinDirs.end());
        for (const auto &item : builtinDirs) {
            dirs.prepend(item);
        }
        q->setPluginDirs(dirs);

        if (m_loadMetaDatas.isRunning()) {
            m_loadMetaDatas.cancel();
            m_loadMetaDatas.waitForFinished();
        }

        m_plugins.clear();
        m_loadMetaDatas = QtConcurrent::run(std::bind(&DPluginLoaderPrivate::initPlugins, this));
    }

    void initPlugins()
    {
        for (const auto &item : m_pluginDirs) {
            const QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;
            const QStringList nameFilters = {MetaDataFileName};

            QDirIterator it(item, nameFilters, QDir::Files, flags);
            QSet<QString> dirs;
            while (it.hasNext()) {
                it.next();

                const QString dir = it.fileInfo().absoluteDir().path();
                if (dirs.contains(dir)) {
                    continue;
                }
                dirs << dir;

                const QString metadataPath = it.fileInfo().absoluteFilePath();
                DPluginMetaData info = DPluginMetaData::fromJsonFile(metadataPath);
                if (!info.isValid())
                    continue;

                if (m_disabledPlugins.contains(info.pluginId())) {
                    qCDebug(dsLog()) << "Don't load disabled applet." << info.pluginId();
                    continue;
                }

                if (m_plugins.contains(info.pluginId()))
                    continue;

                m_plugins[info.pluginId()] = info;
            }
        }
    }

    QStringList builtinPackagePaths()
    {
        QStringList result;
        // 'DDE_SHELL_PACKAGE_PATH' directory.
        const auto dtkPluginPath = qEnvironmentVariable("DDE_SHELL_PACKAGE_PATH");
        if (!dtkPluginPath.isEmpty())
            result << dtkPluginPath.split(QDir::listSeparator());

        const QString packageDir = buildingDir("packages");
        if (!packageDir.isEmpty())
            result << packageDir;

        for (const auto &item : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
            result << item + "/dde-shell";
        }
        qCDebug(dsLog()) << "Builtin package paths" << result;

        return result;
    }

    QStringList builtinPluginPaths()
    {
        QStringList result;
        // 'DDE_SHELL_PACKAGE_PATH' directory.
        const auto dtkPluginPath = qEnvironmentVariable("DDE_SHELL_PLUGIN_PATH");
        if (!dtkPluginPath.isEmpty())
            result << dtkPluginPath.split(QDir::listSeparator());

        const QString pluginsDir = buildingDir("plugins");
        if (!pluginsDir.isEmpty())
            result << pluginsDir;

        result <<  DDE_SHELL_PLUGIN_INSTALL_DIR;

        qCDebug(dsLog()) << "Builtin plugin paths" << result;
        return result;
    }

    bool existPlugin(const DPluginMetaData &data) const
    {
        const QString fileName = data.pluginId();
        D_QC(DPluginLoader);
        for (const auto &item : q->pluginDirs()) {
            const QDir dir(item);
            if (dir.exists(fileName + PluginSuffix))
                return true;
        }
        return false;
    }

    DAppletFactory *appletFactory(const DPluginMetaData &data)
    {
        if (!existPlugin(data))
            return nullptr;

        DAppletFactory *factory = nullptr;
        const QString fileName = data.pluginId();
        QPluginLoader loader(fileName);
        loader.load();
        if (!loader.isLoaded()) {
            qCWarning(dsLog) << "Load the plugin failed." << loader.errorString();
            return factory;
        }

        const auto &meta = loader.metaData();

        do {
            const auto iid = meta["IID"].toString();
            if (iid.isEmpty())
                break;

            if (iid != QString(qobject_interface_iid<DAppletFactory *>()))
                break;

            if (!loader.instance()) {
                qWarning(dsLog) << "Load the plugin failed." << loader.errorString();
                break;
            }
            factory = qobject_cast<DAppletFactory *>(loader.instance());
            if (!factory) {
                qWarning(dsLog) << "The plugin isn't a DAppletFactory." << fileName;
                break;
            }
        } while (false);

        return factory;
    }

    DPluginMetaData pluginMetaData(const QString &pluginId) const
    {
        const auto it = m_plugins.constFind(pluginId);
        if (it == m_plugins.constEnd()) {
            if (DPluginMetaData::isRootPlugin(pluginId))
                return DPluginMetaData::rootPluginMetaData();
            return DPluginMetaData();
        }
        return it.value();
    }

    inline QString buildingDir(const QString &subdir)
    {
        QDir dir(QCoreApplication::applicationDirPath());
        dir.cdUp();
        if (dir.exists() && dir.exists(subdir))
            return dir.absoluteFilePath(subdir);

        return QString();
    }
    void ensureCompleted() const
    {
        if (m_loadMetaDatas.isRunning()) {
            const_cast<DPluginLoaderPrivate *>(this)->m_loadMetaDatas.waitForFinished();
        }
    }

    QStringList m_pluginDirs;
    QMap<QString, DPluginMetaData> m_plugins;
    QStringList m_disabledPlugins;
    QFuture<void> m_loadMetaDatas;
    QScopedPointer<DApplet> m_rootApplet;

    D_DECLARE_PUBLIC(DPluginLoader)
};

DPluginLoader::DPluginLoader()
    : DObject(*new DPluginLoaderPrivate(this))
{
    D_D(DPluginLoader);
    d->init();
}

DPluginLoader::~DPluginLoader()
{
    destroy();
}

DPluginLoader *DPluginLoader::instance()
{
    return g_instance;
}

QList<DPluginMetaData> DPluginLoader::plugins() const
{
    D_DC(DPluginLoader);
    d->ensureCompleted();
    return d->m_plugins.values();
}

QList<DPluginMetaData> DPluginLoader::rootPlugins() const
{
    D_DC(DPluginLoader);
    d->ensureCompleted();
    QList<DPluginMetaData> rootPlugins;
    for (const auto &item : plugins()) {
        auto parent = parentPlugin(item.pluginId());

        // root plugin can't has parent.
        if (parent.isValid())
            continue;

        if (rootPlugins.contains(item))
            continue;

        rootPlugins << item;
    }
    return rootPlugins;
}

void DPluginLoader::addPackageDir(const QString &dir)
{
    D_D(DPluginLoader);
    d->m_pluginDirs.prepend(dir);
    d->init();
}

void DPluginLoader::addPluginDir(const QString &dir)
{
    if (QCoreApplication::libraryPaths().contains(dir))
        return;
    QCoreApplication::addLibraryPath(dir);
}

QStringList DPluginLoader::pluginDirs() const
{
    return QCoreApplication::libraryPaths();
}

void DPluginLoader::setPluginDirs(const QStringList &dirs)
{
    QCoreApplication::setLibraryPaths(dirs);
}

QStringList DPluginLoader::disabledApplets() const
{
    D_DC(DPluginLoader);
    return d->m_disabledPlugins;
}

void DPluginLoader::setDisabledApplets(const QStringList &pluginIds)
{
    D_D(DPluginLoader);
    if (pluginIds.isEmpty() || d->m_disabledPlugins == pluginIds)
        return;
    for (const auto &item : pluginIds) {
        if (item.isEmpty() || d->m_disabledPlugins.contains(item))
            continue;
        d->m_disabledPlugins << item;
    }
    d->init();
}

DApplet *DPluginLoader::loadApplet(const DAppletData &data)
{
    D_D(DPluginLoader);
    d->ensureCompleted();
    const QString pluginId(data.pluginId());
    DPluginMetaData metaData = d->pluginMetaData(pluginId);
    if (!metaData.isValid())
        return nullptr;

    DApplet *applet = nullptr;
    if (auto factory = d->appletFactory(metaData)) {
        qCDebug(dsLog()) << "Loading applet by factory" << pluginId;
        applet = factory->create();
    }
    if (!applet) {
        const auto containmentType = metaData.value("ContainmentType");
        if (containmentType.isValid()) {
            const QStringList PanelList {"Panel", "Root"};
            if (PanelList.contains(containmentType)) {
                applet = new DPanel();
            } else {
                applet = new DContainment();
            }
        }
    }

    if (!applet) {
        applet = new DApplet();
    }
    if (applet) {
        applet->setMetaData(metaData);
        DAppletData dt = data;
        if (dt.id().isEmpty()) {
            dt.setId(QUuid::createUuid().toString());
        }
        applet->setAppletData(dt);
    }
    return applet;
}

DApplet *DPluginLoader::rootApplet() const
{
    D_DC(DPluginLoader);
    if (!d->m_rootApplet) {
        const auto data = DAppletData::fromPluginMetaData(DPluginMetaData::rootPluginMetaData());
        const_cast<DPluginLoaderPrivate *>(d)->m_rootApplet.reset(const_cast<DPluginLoader *>(this)->loadApplet(data));
    }

    return d->m_rootApplet.data();
}

QList<DPluginMetaData> DPluginLoader::childrenPlugin(const QString &pluginId) const
{
    D_DC(DPluginLoader);
    d->ensureCompleted();
    DPluginMetaData metaData = d->pluginMetaData(pluginId);
    if (!metaData.isValid())
        return {};

    if (DPluginMetaData::isRootPlugin(pluginId))
        return rootPlugins();

    const DPluginMetaData target(metaData);
    QList<DPluginMetaData> result;
    for (const auto &md : d->m_plugins) {
        const QString parentId(md.value("Parent").toString());
        if (parentId == target.pluginId()) {
            result << md;
        }
    }
    return result;
}

DPluginMetaData DPluginLoader::parentPlugin(const QString &pluginId) const
{
    D_DC(DPluginLoader);
    d->ensureCompleted();
    DPluginMetaData metaData = d->pluginMetaData(pluginId);
    if (!metaData.isValid())
        return DPluginMetaData();

    const QString parentId(metaData.value("Parent").toString());
    return d->pluginMetaData(parentId);
}

DPluginMetaData DPluginLoader::plugin(const QString &pluginId) const
{
    D_DC(DPluginLoader);
    d->ensureCompleted();
    return d->pluginMetaData(pluginId);
}

void DPluginLoader::destroy()
{
    D_D(DPluginLoader);
    d->m_rootApplet.reset();
}

DS_END_NAMESPACE
