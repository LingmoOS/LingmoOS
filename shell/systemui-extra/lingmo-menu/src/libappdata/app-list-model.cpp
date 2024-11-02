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

#include "app-list-model.h"
#include "data-entity.h"
#include "context-menu-manager.h"

#include <QDebug>

namespace LingmoUIMenu {

AppListHeader::AppListHeader(QObject *parent) : QObject(parent)
{

}

QString AppListHeader::title() const
{
    return m_title;
}

void AppListHeader::setTitle(const QString &title)
{
    if (title == m_title) {
        return;
    }

    m_title = title;
    Q_EMIT titleChanged();
}

QList<QAction*> AppListHeader::actions() const
{
    return m_actions;
}

void AppListHeader::addAction(QAction *action)
{
    if (m_actions.indexOf(action) >= 0) {
        return;
    }

    m_actions.append(action);
    Q_EMIT actionsChanged();
    setVisible(true);
}

bool AppListHeader::visible() const
{
    return m_visible;
}

void AppListHeader::setVisible(bool visible)
{
    if (m_visible == visible) {
        return;
    }

    m_visible = visible;
    Q_EMIT visibleChanged();
}

void AppListHeader::removeAction(QAction *action)
{
    if (m_actions.removeOne(action)) {
        Q_EMIT actionsChanged();
        setVisible(!m_actions.isEmpty());
    }
}

void AppListHeader::removeAllAction()
{
    m_actions.clear();
    Q_EMIT actionsChanged();
    setVisible(false);
}

// ====== //
AppListModel::AppListModel(QObject *parent) : QSortFilterProxyModel(parent), m_header(new AppListHeader(this))
{
    qRegisterMetaType<LingmoUIMenu::AppListModel*>();
    qRegisterMetaType<LingmoUIMenu::AppListHeader*>();
}

QHash<int, QByteArray> AppListModel::roleNames() const
{
    return DataEntity::AppRoleNames();
}

AppListHeader *AppListModel::getHeader() const
{
    return m_header;
}

void AppListModel::installPlugin(AppListPluginInterface *plugin)
{
    if (!plugin || m_plugin == plugin) {
        return;
    }

    unInstallPlugin();

    m_plugin = plugin;
    QSortFilterProxyModel::setSourceModel(plugin->dataModel());

    for (const auto &action : plugin->actions()) {
        m_header->addAction(action);
    }

    m_header->setTitle(plugin->title());
    connect(m_plugin, &AppListPluginInterface::titleChanged, this, [this, plugin] {
        m_header->setTitle(plugin->title());
    });
    connect(m_plugin, &AppListPluginInterface::labelChanged, this, [this, plugin] {
        Q_EMIT labelBottleChanged();
    });

    Q_EMIT labelBottleChanged();
}

void AppListModel::unInstallPlugin()
{
    if (!m_plugin) {
        return;
    }

    m_header->setTitle("");
    m_header->removeAllAction();
    QSortFilterProxyModel::setSourceModel(nullptr);
    disconnect(m_plugin, nullptr, this, nullptr);
    m_plugin = nullptr;

    Q_EMIT labelBottleChanged();
}

void AppListModel::openMenu(const int &index, MenuInfo::Location location) const
{
    QModelIndex idx = AppListModel::index(index, 0);
    if (checkIndex(idx, CheckIndexOption::IndexIsValid)) {
        ContextMenuManager::instance()->showMenu(idx.data(DataEntity::Entity).value<DataEntity>(), location);
    }
}

LabelBottle *AppListModel::labelBottle() const
{
    if (m_plugin) {
        return m_plugin->labelBottle();
    }

    return nullptr;
}

int AppListModel::findLabelIndex(const QString &label) const
{
    // TODO: 潜在的优化空间
    int count = AppListModel::rowCount();
    for (int i = 0; i < count; ++i) {
        if (AppListModel::sourceModel()->index(i, 0).data(DataEntity::Group).toString() == label) {
            return i;
        }
    }

    return -1;
}

} // LingmoUIMenu
