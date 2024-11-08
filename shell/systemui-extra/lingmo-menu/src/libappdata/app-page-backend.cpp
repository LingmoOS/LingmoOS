/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "app-page-backend.h"

#include "app-list-model.h"
#include "app-category-plugin.h"
#include "app-search-plugin.h"

#include <QDebug>

namespace LingmoUIMenu {

AppPageBackend *AppPageBackend::instance()
{
    static AppPageBackend backend;
    return &backend;
}

AppPageBackend::AppPageBackend(QObject *parent) : QObject(parent), m_appModel(new AppListModel(this))
{
    auto searchPlugin = new AppSearchPlugin(this);
    auto categoryPlugin = new AppCategoryPlugin(this);

    m_plugins.insert(searchPlugin->group(), searchPlugin);
    m_plugins.insert(categoryPlugin->group(), categoryPlugin);

    switchGroup();
}

AppListModel *AppPageBackend::appModel() const
{
    return m_appModel;
}

void AppPageBackend::startSearch(const QString &keyword)
{
    if (m_group != AppListPluginGroup::Search) {
        return;
    }

    auto plugin = m_plugins.value(m_group, nullptr);
    if (plugin) {
        plugin->search(keyword);
    }
}

AppListPluginGroup::Group AppPageBackend::group() const
{
    return AppListPluginGroup::Display;
}

void AppPageBackend::setGroup(AppListPluginGroup::Group group)
{
    if (m_group == group) {
        return;
    }

    m_group = group;
    switchGroup();
    Q_EMIT groupChanged();
}

void AppPageBackend::switchGroup()
{
    const auto plugin = m_plugins.value(m_group, nullptr);
    if (plugin) {
        m_appModel->installPlugin(plugin);
    } else {
        m_appModel->unInstallPlugin();
    }
}

} // LingmoUIMenu
