/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "data-provider-manager.h"
#include "event-track.h"
#include "plugin/all-app-data-provider.h"
#include "plugin/app-search-plugin.h"
#include "plugin/app-category-plugin.h"
#include "plugin/app-letter-sort-plugin.h"

namespace LingmoUIMenu {

/**
 * 1.统一不同model显示相同的数据。
 * 2.对model隐藏插件。
 */
DataProviderManager *DataProviderManager::instance()
{
    static DataProviderManager dataProviderManager;
    return &dataProviderManager;
}

DataProviderManager::DataProviderManager()
{
    qRegisterMetaType<DataUpdateMode::Mode>("DataUpdateMode::Mode");
    m_worker.start();
    initProviders();
}

void DataProviderManager::initProviders()
{
    auto *search = new AppSearchPlugin;
    connect(this, &DataProviderManager::toUpdate, search, &AppSearchPlugin::update);
    registerProvider(search);

    auto *category = new AppCategoryPlugin;
    connect(this, &DataProviderManager::toUpdate, category, &AppCategoryPlugin::update);
    registerProvider(category);

    auto *sort = new AppLetterSortPlugin;
    connect(this, &DataProviderManager::toUpdate, sort, &AppLetterSortPlugin::update);
    registerProvider(sort);

    activateProvider(sort->id(), false);
    search->moveToThread(&m_worker);
    category->moveToThread(&m_worker);
    sort->moveToThread(&m_worker);
}

void DataProviderManager::registerProvider(DataProviderPluginIFace *provider)
{
    // if nullptr
    if (!provider) {
        return;
    }

    if (m_providers.contains(provider->id())) {
        return;
    }

    m_providers.insert(provider->id(), provider);
    connect(provider, &AllAppDataProvider::dataChanged, this,
            [this, provider](const QVector<DataEntity> &data, DataUpdateMode::Mode mode, quint32 index) {
        //qDebug() << "==DataProviderManager dataChanged:" << provider->name() << QThread::currentThreadId();
        if (m_activatedPlugin != provider->id()) {
            return;
        }

        Q_EMIT dataChanged(data, mode, index);
    });

    connect(provider, &AllAppDataProvider::labelChanged, this, [this, provider]() {
        if (m_activatedPlugin != provider->id()) { return; }
        Q_EMIT labelChanged();
    });
}

QVector<DataEntity> DataProviderManager::data() const
{
    return m_providers.value(m_activatedPlugin)->data();
}

QVector<LabelItem> DataProviderManager::labels() const
{
    return m_providers.value(m_activatedPlugin)->labels();
}

QString DataProviderManager::activatedProvider() const
{
    return m_activatedPlugin;
}

QStringList DataProviderManager::providers() const
{
    return m_providers.keys();
}

QVector<ProviderInfo> DataProviderManager::providers(PluginGroup::Group group) const
{
    QVector<ProviderInfo> infos;
    for (const auto &provider : m_providers) {
        if (group == provider->group()) {
            infos.append({
                provider->index(),
                provider->id(),
                provider->name(),
                provider->icon(),
                provider->title(),
                provider->group()
            });
        }
    }

    return infos;
}

ProviderInfo DataProviderManager::providerInfo(const QString &id) const
{
    ProviderInfo info;
    DataProviderPluginIFace* provider = m_providers.value(id, nullptr);
    if (provider) {
        info = {
            provider->index(),
            provider->id(),
            provider->name(),
            provider->icon(),
            provider->title(),
            provider->group()
        };
    }

    return info;
}

void DataProviderManager::activateProvider(const QString &id, bool record)
{
    if (!m_providers.contains(id) || m_activatedPlugin == id) {
        return;
    }

    m_activatedPlugin = id;
    Q_EMIT pluginChanged(m_activatedPlugin, m_providers.value(m_activatedPlugin)->group());
    if (record) {
        QMap<QString, QVariant> map;
        map.insert("viewName", id);
        EventTrack::instance()->sendClickEvent("switch_app_view", "AppView", map);
    }
}

DataProviderManager::~DataProviderManager()
{
    m_worker.quit();
    m_worker.wait();

    for (auto &provider : m_providers) {
        provider->deleteLater();
    }
}

void DataProviderManager::forceUpdate() const
{
    m_providers.value(m_activatedPlugin)->forceUpdate();
}

void DataProviderManager::forceUpdate(QString &key) const
{
    m_providers.value(m_activatedPlugin)->forceUpdate(key);
}

} // LingmoUIMenu
