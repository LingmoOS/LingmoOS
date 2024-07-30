/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "personpluginmanager.h"
#include "backends/basepersonsdatasource.h"

#include "kpeople_debug.h"

#include <KPluginFactory>
#include <KPluginMetaData>

#include <QMutex>
#include <QMutexLocker>

using namespace KPeople;

class PersonPluginManagerPrivate
{
public:
    PersonPluginManagerPrivate();
    ~PersonPluginManagerPrivate();
    QHash<QString /* SourceName*/, BasePersonsDataSource *> dataSourcePlugins;

    void loadDataSourcePlugins();
    bool m_autoloadDataSourcePlugins = true;
    bool m_loadedDataSourcePlugins = false;
    QMutex m_mutex;
};

Q_GLOBAL_STATIC(PersonPluginManagerPrivate, s_instance)

PersonPluginManagerPrivate::PersonPluginManagerPrivate()
{
}

PersonPluginManagerPrivate::~PersonPluginManagerPrivate()
{
    qDeleteAll(dataSourcePlugins);
}

void PersonPluginManagerPrivate::loadDataSourcePlugins()
{
    const QList<KPluginMetaData> pluginList = KPluginMetaData::findPlugins(QStringLiteral("kpeople/datasource"));
    for (const KPluginMetaData &data : pluginList) {
        auto dataSourceResult = KPluginFactory::instantiatePlugin<BasePersonsDataSource>(data);
        if (dataSourceResult) {
            BasePersonsDataSource *dataSource = dataSourceResult.plugin;
            const QString pluginId = dataSource->sourcePluginId();
            if (!dataSourcePlugins.contains(pluginId)) {
                dataSourcePlugins[pluginId] = dataSource;
            } else {
                dataSource->deleteLater();
                qCDebug(KPEOPLE_LOG) << "Plugin" << pluginId << "was already loaded manually, ignoring...";
            }
        } else {
            qCWarning(KPEOPLE_LOG) << "Failed to create data source " << dataSourceResult.errorText << data.fileName();
        }
    }

    m_loadedDataSourcePlugins = true;
}

void PersonPluginManager::setAutoloadDataSourcePlugins(bool autoloadDataSourcePlugins)
{
    s_instance->m_autoloadDataSourcePlugins = autoloadDataSourcePlugins;
}

void PersonPluginManager::addDataSource(const QString &sourceId, BasePersonsDataSource *source)
{
    QMutexLocker locker(&s_instance->m_mutex);
    if (s_instance->dataSourcePlugins.contains(sourceId)) {
        qCWarning(KPEOPLE_LOG) << "Attempting to load data source that is already loaded, overriding!";
        s_instance->dataSourcePlugins[sourceId]->deleteLater();
    }
    s_instance->dataSourcePlugins.insert(sourceId, source);
}

void PersonPluginManager::setDataSourcePlugins(const QHash<QString, BasePersonsDataSource *> &dataSources)
{
    QMutexLocker locker(&s_instance->m_mutex);
    qDeleteAll(s_instance->dataSourcePlugins);
    s_instance->dataSourcePlugins = dataSources;
    s_instance->m_loadedDataSourcePlugins = true;
}

QList<BasePersonsDataSource *> PersonPluginManager::dataSourcePlugins()
{
    QMutexLocker locker(&s_instance->m_mutex);
    if (!s_instance->m_loadedDataSourcePlugins && s_instance->m_autoloadDataSourcePlugins) {
        s_instance->loadDataSourcePlugins();
    }
    return s_instance->dataSourcePlugins.values();
}

BasePersonsDataSource *PersonPluginManager::dataSource(const QString &sourceId)
{
    QMutexLocker locker(&s_instance->m_mutex);
    if (!s_instance->m_loadedDataSourcePlugins && s_instance->m_autoloadDataSourcePlugins) {
        s_instance->loadDataSourcePlugins();
    }
    return s_instance->dataSourcePlugins.value(sourceId);
}

bool KPeople::PersonPluginManager::addContact(const QVariantMap &properties)
{
    bool ret = false;
    for (auto p : std::as_const(s_instance->dataSourcePlugins)) {
        auto v2 = dynamic_cast<BasePersonsDataSourceV2 *>(p);
        if (!v2) {
            continue;
        }
        const bool added = v2->addContact(properties);
        ret |= added;
    }
    return ret;
}

bool KPeople::PersonPluginManager::deleteContact(const QString &uri)
{
    bool ret = false;
    for (auto p : std::as_const(s_instance->dataSourcePlugins)) {
        auto v2 = dynamic_cast<BasePersonsDataSourceV2 *>(p);
        if (!v2) {
            continue;
        }
        const bool deleted = v2->deleteContact(uri);
        ret |= deleted;
    }
    return ret;
}
