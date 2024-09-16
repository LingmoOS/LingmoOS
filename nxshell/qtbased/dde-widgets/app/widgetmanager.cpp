// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgetmanager.h"
#include "pluginspec.h"
#include "widgethandler.h"
#include "instanceproxy.h"

#include <QPluginLoader>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>
#include <QWidget>

const QString PLUGIN_DIRECTORY = QStringLiteral(DefaultPluginDirectory);

WIDGETS_FRAME_BEGIN_NAMESPACE
static QPair<quint16, quint16> parseVersion(const QString &version)
{
    QPair<quint16, quint16> res{0, 0};
    const QStringList &items = version.split(".");
    if (items.size() != 2)
        return res;

    bool ok = false;
    const quint16 major = items.first().toUShort(&ok);
    if (!ok)
        return res;
    const quint16 minor = items.last().toUShort(&ok);
    if (!ok)
        return res;

    res.first = major;
    res.second = minor;
    return res;
}

WidgetManager::WidgetManager()
{
}

WidgetManager::~WidgetManager()
{
    shutdown();
}

void WidgetManager::shutdown()
{
    aboutToShutdown(m_widgets.values().toVector());
    // it maybe exist dangling pointer if IWidget is released by QObject.
    qDeleteAll(m_widgets);
    m_widgets.clear();
    qDeleteAll(m_plugins);
    m_plugins.clear();
}

DataStore *WidgetManager::dataStore()
{
    return &m_dataStore;
}

QString WidgetManager::currentVersion()
{
    return QString("1.0");
}

bool WidgetManager::matchVersion(const QString &version)
{
    static const auto supportVersion = parseVersion(currentVersion());
    const auto curr = parseVersion(version);
    return ((curr.first > 0 || curr.second > 0) && (curr.first <= supportVersion.first));
}

void WidgetManager::loadPlugins()
{
    qDeleteAll(m_plugins);
    m_plugins.clear();

    for (QString fileName : pluginPaths()) {

        const auto &info = parsePluginInfo(fileName);
        if (info.plugin) {
            auto spec = loadPlugin(info);
            qDebug(dwLog()) << "load the plugin [" << spec->id() << "] successful." << fileName;
        }
    }
}

QHash<PluginId, WidgetPlugin *> WidgetManager::plugins() const
{
    return m_plugins;
}

QList<WidgetPlugin *> WidgetManager::plugins(const IWidgetPlugin::Type type) const
{
    QList<WidgetPlugin *> result;
    for (auto plugin : qAsConst(m_plugins)) {
        if (plugin->type() != type)
            continue;
        result.append(plugin);
    }
    // sort plugin's order by character sorting.
    std::sort(result.begin(), result.end(), [this](const WidgetPlugin *first, const WidgetPlugin *second){
        return first->id() < second->id();
    });
    return result;
}

QMultiMap<PluginId, Instance *> WidgetManager::loadWidgetStoreInstances()
{
    QMultiMap<PluginId, Instance *> instances;
    for (auto plugin : m_plugins) {
        if (plugin->type() != IWidgetPlugin::Normal) {
            continue;
        }
        for (auto instance : createWidgetStoreInstances(plugin->id())) {
            instances.insert(plugin->id(), instance);
        }
    }
    initialize(instances.values().toVector());
    return instances;
}

WidgetPlugin *WidgetManager::getPlugin(const PluginId &key) const
{
    return m_plugins.value(key);
}

Instance *WidgetManager::createWidget(const PluginId &pluginId, const IWidget::Type &type)
{
    if (auto plugin = getPlugin(pluginId)) {
        auto instance = plugin->createWidget(type);
        Q_ASSERT(instance);
        if (initialize(instance)) {
            typeChanged({instance});
            return instance;
        }
        instance->deleteLater();
    }
    return nullptr;
}

Instance *WidgetManager::createWidget(const PluginId &pluginId, const IWidget::Type &type, const InstanceId &id)
{
    if (auto plugin = getPlugin(pluginId)) {
        auto instance = plugin->createWidget(type, id);
        Q_ASSERT(instance);
        if (initialize(instance)) {
            typeChanged({instance});
            return instance;
        }
        instance->deleteLater();
    }
    return nullptr;
}

void WidgetManager::removeWidget(const InstanceId &instanceId)
{
    if (auto instance = m_widgets.take(instanceId)) {
        aboutToShutdown(instance);
        instance->deleteLater();
    }
}

void WidgetManager::typeChanged(const InstanceId &instanceId, const IWidget::Type &type)
{
    if (auto instance = m_widgets.value(instanceId)) {
        WidgetHandlerImpl::get(instance->handler())->m_type = type;
        instance->typeChanged(type);
    }
}

void WidgetManager::typeChanged(const QVector<Instance *> &instances)
{
    for (auto instance : qAsConst(instances)) {
        instance->typeChanged(instance->handler()->type());
    }
}

Instance *WidgetManager::getInstance(const InstanceId &key)
{
    return m_widgets.value(key);
}

void WidgetManager::showWidgets(const QVector<Instance *> &instances)
{
    for (auto instance : qAsConst(instances)) {
        instance->showWidgets();
    }
}

void WidgetManager::hideWidgets(const QVector<Instance *> &instances)
{
    for (auto instance : qAsConst(instances)) {
        instance->hideWidgets();
    }
}

QVector<Instance *> WidgetManager::initialize(const QVector<Instance *> &instances)
{
    QVector<Instance *> failed;
    QVector<QFuture<void>> futures;
    futures.reserve(instances.size());
    for (auto instance : qAsConst(instances)) {
        qDebug(dwLog()) << "initialize widget." << instance->handler()->pluginId() << instance->handler()->id();
        if (!instance->initialize(m_arguments)) {
            failed.append(instance);
            continue;
        }

        m_widgets[instance->handler()->id()] = instance;

        qDebug(dwLog()) << "delayInitialize widget." << instance->handler()->pluginId() << instance->handler()->id();
        futures << QtConcurrent::run(instance, &Instance::delayInitialize);
    }
    // TODO delay those blocking.
    for (auto item : futures) {
        item.waitForFinished();
    }
    return failed;
}

bool WidgetManager::initialize(Instance *instance)
{
    return initialize(QVector<Instance *>{instance}).isEmpty();
}

void WidgetManager::aboutToShutdown(const QVector<Instance *> &instances)
{
    for (auto instance : qAsConst(instances)) {
        qDebug(dwLog()) << "aboutToShutdown widget." << instance->handler()->pluginId() << instance->handler()->id();
        instance->aboutToShutdown();
    }
}

void WidgetManager::aboutToShutdown(Instance *instance)
{
    aboutToShutdown(QVector<Instance *>{instance});
}

void WidgetManager::clearDataStore(const PluginId &id)
{
    QFile::remove(dataStorePath(id));
}

void WidgetManager::clearDataStore(const PluginId &id, const InstanceId &instanceId)
{
    const auto &path = dataStorePath(id);
    if (!QFile::exists(path))
        return;

    DataStore dataStore(path, QSettings::NativeFormat);
    dataStore.beginGroup(instanceId);
    dataStore.remove("");
    dataStore.endGroup();
}

QString WidgetManager::dataStorePath(const PluginId &pluginId) const
{
    QFileInfo fileInfo(m_dataStore.fileName());
    const QDir dir(fileInfo.absoluteDir());
    const QString &baseName = fileInfo.baseName();
    return dir.absoluteFilePath(QString("%1-%2.json").arg(baseName).arg(pluginId));
}

QList<PluginPath> WidgetManager::addingPlugins()
{
    QList<PluginPath> newPluginIds;
    const QList<PluginId> prePluginIds = m_plugins.keys();

    for (QString fileName : pluginPaths()) {
        const auto &info = parsePluginInfo(fileName);
        if (info.plugin) {
            if (prePluginIds.contains(info.id)) {
                continue;
            }
            auto spec = loadPlugin(info);
            qDebug(dwLog()) << "load new plugin [" << spec->id() << "] successful." << fileName;

            newPluginIds << info.fileName;
        }
    }
    return newPluginIds;
}

QList<Instance *> WidgetManager::createWidgetStoreInstances(const PluginId &key)
{
    QVector<Instance *> instances;
    if (auto plugin = getPlugin(key)) {
        static const QVector<IWidget::Type> Types{IWidget::Small, IWidget::Middle, IWidget::Large};
        for (auto type : Types) {
            auto instance = plugin->createWidgetForWidgetStore(type);
            if (!instance) {
                plugin->removeSupportType(type);
                continue;
            }

            instances << instance;
        }
    }
    const auto &failedInstances = initialize(instances);
    if (!failedInstances.isEmpty()) {
        for (auto item : failedInstances) {
            const auto index = instances.indexOf(item);
            if (index < 0)
                continue;

            if (auto plugin = m_plugins.value(instances[index]->handler()->pluginId())) {
                plugin->removeSupportType(instances[index]->handler()->type());
            }
            instances.remove(index);
        }
        qDeleteAll(failedInstances);
    }
    typeChanged(instances);
    return instances.toList();
}

WidgetPluginSpec *WidgetManager::loadPlugin(const PluginPath &pluginPath)
{
    const auto info = parsePluginInfo(pluginPath);
    if (info.plugin) {
        return loadPlugin(info);
    }
    return nullptr;
}

WidgetPluginSpec *WidgetManager::loadPlugin(const PluginInfo &info)
{
    Q_ASSERT(info.plugin);

    auto spec = new WidgetPluginSpec (info);

    auto store = new DataStore(dataStorePath(spec->id()), QSettings::NativeFormat);
    qDebug(dwLog()) << "loadPlugin() config's filePath:" << store->fileName();
    spec->setDataStore(store);
    m_plugins.insert(spec->id(), spec);

    return spec;
}

bool WidgetManager::isPlugin(const QString &fileName) const
{
    const auto &info = parsePluginInfo(fileName);
    return info.plugin;
}

PluginInfo WidgetManager::parsePluginInfo(const QString &fileName) const
{
    PluginInfo info;

    QPluginLoader loader(fileName);
    const auto &meta = loader.metaData();

    do {
        const auto iid = meta["IID"].toString();
        if (iid.isEmpty())
            break;

        if (iid != QString(qobject_interface_iid<IWidgetPlugin *>()))
            break;

        info.id = meta["MetaData"]["pluginId"].toString();
        if (info.id.isEmpty()) {
            qWarning(dwLog()) << "pluginId not existed in MetaData for the plugin." << fileName;
            break;
        }
        info.version = meta["MetaData"]["version"].toString();
        if (!matchVersion(info.version)) {
            qWarning(dwLog()) << QString("plugin version [%1] is not matched by [%2].").arg(info.version).arg(currentVersion()) << fileName;
            break;
        }
        if (!loader.instance()) {
            qWarning(dwLog()) << "load the plugin error." << loader.errorString();
            break;
        }
        info.plugin = qobject_cast<IWidgetPlugin *>(loader.instance());
        if (!info.plugin) {
            qWarning(dwLog()) << "the plugin isn't a IWidgetPlugin." << fileName;
            break;
        }
        info.fileName = fileName;
    } while (false);

    if (!info.plugin) {
        loader.unload();
    }

    return info;
}

void WidgetManager::removePlugin(const PluginId &key)
{
    auto plugin = m_plugins.take(key);
    Q_ASSERT(plugin);

    delete plugin;
}

QList<PluginId> WidgetManager::removingPlugins() const
{
    QList<PluginId> removePluginIds;
    const QList<PluginId> prePluginIds = m_plugins.keys();
    for (auto pluginId : prePluginIds) {
        auto plugin = getPlugin(pluginId);
        if (isPlugin(plugin->m_fileName)) {
            continue;
        }
        removePluginIds << pluginId;
    }
    return removePluginIds;
}

QStringList WidgetManager::pluginPaths() const
{
    // The same pluginid will be overwritten by later, `DDE_WIDGETS_PLUGIN_DIRS` > `./plugins` > `/usr`
    QStringList dirs;
    dirs << PLUGIN_DIRECTORY;
    dirs << QCoreApplication::applicationDirPath() + "/plugins";
    const QString &envPaths = qEnvironmentVariable("DDE_WIDGETS_PLUGIN_DIRS");
    if (!envPaths.isEmpty()) {
        QStringList list = envPaths.split(':');
        std::reverse(list.begin(), list.end());
        dirs << list;
    }

    qDebug(dwLog()) << "load plugins from those dir:" << dirs;
    QStringList pluginPaths;
    for (auto dir : qAsConst(dirs)) {
        auto pluginsDir = QDir(dir);
        if (!pluginsDir.exists())
            continue;

        const auto entryList = pluginsDir.entryList(QDir::Files);
        for (const QString &fileName : qAsConst(entryList)) {
            const auto path = pluginsDir.absoluteFilePath(fileName);
            if (!QLibrary::isLibrary(path))
                continue;

            if (!isPlugin(path))
                continue;

            pluginPaths << path;
        }
    }
    return pluginPaths;
}

QList<Instance *> WidgetManager::getInstances(const PluginId &key) const
{
    QList<Instance *> result;
    for (auto instance : m_widgets) {
        if (instance->handler()->pluginId() == key) {
            result << instance;
        }
    }
    return result;
}

QList<Instance *> WidgetManager::instances() const
{
    return m_widgets.values();
}

void WidgetManager::showAllWidgets()
{
    showWidgets(instances().toVector());
}

void WidgetManager::hideAllWidgets()
{
    hideWidgets(instances().toVector());
}
WIDGETS_FRAME_END_NAMESPACE
