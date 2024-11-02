/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "view-factory-model.h"
#include "directory-view-factory-manager.h"
#include "directory-view-plugin-iface.h"
#include "directory-view-plugin-iface2.h"
#include <QPair>

using namespace Peony;

//ViewFactory2 model
ViewFactoryModel2::ViewFactoryModel2(QObject *parent)
    : QAbstractListModel(parent)
{

}

void ViewFactoryModel2::setDirectoryUri(const QString &uri)
{
    beginResetModel();
    m_support_views_id.clear();
    m_factory_hash.clear();
    m_current_uri = uri;
    auto viewManager = DirectoryViewFactoryManager2::getInstance();
    auto defaultList = viewManager->getFactoryNames();

    for (auto id : defaultList) {
        if (viewManager->getFactory(id)->supportUri(m_current_uri)) {
            QPair<int, QString> pair(viewManager->getFactory(id)->priority(m_current_uri), id);
            m_factory_hash.insert(viewManager->getFactory(id)->zoom_level_hint(),
                                  pair);

            m_support_views_id<<id;
        }
    }

    endResetModel();
}

const QString ViewFactoryModel2::getViewId(int index)
{
    if (index > m_support_views_id.count() - 1 || index < 0) {
        return nullptr;
    }
    return m_support_views_id.at(index);
}

int ViewFactoryModel2::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return m_support_views_id.count();
}

const QString ViewFactoryModel2::getHighestPriorityViewId(int zoom_level_hint)
{
    auto manager = DirectoryViewFactoryManager2::getInstance();
    auto pairs = m_factory_hash.values(zoom_level_hint);
    DirectoryViewPluginIface2 *factory = nullptr;
    int priority = -9999;
    QString result;
    for (auto pair : pairs) {
        factory = manager->getFactory(pair.second);
        if (factory->priority(m_current_uri) > priority) {
            result = pair.second;
            priority = pair.first;
        }
    }
    return result;
}

QVariant ViewFactoryModel2::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto viewManager = DirectoryViewFactoryManager2::getInstance();
    auto factory = viewManager->getFactory(viewManager->getFactoryNames().at(index.row()));

    switch (role) {
    case Qt::DecorationRole:
        return factory->viewIcon();
    case Qt::ToolTipRole:
        return factory->viewIdentity();
    case Qt::UserRole:
        return factory->viewIdentity();
    default:
        break;
    }
    return QVariant();
}

const QModelIndex ViewFactoryModel2::getIndexFromViewId(const QString &viewId)
{
    if (!m_support_views_id.contains(viewId))
        return QModelIndex();
    return index(m_support_views_id.indexOf(viewId));
}
