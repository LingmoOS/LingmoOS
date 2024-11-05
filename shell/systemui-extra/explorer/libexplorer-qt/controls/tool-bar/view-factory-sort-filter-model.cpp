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

#include "view-factory-sort-filter-model.h"
#include "directory-view-factory-manager.h"
#include "directory-view-plugin-iface.h"
#include "directory-view-plugin-iface2.h"

#include "view-factory-model.h"

using namespace Peony;

//Proxy Model 2
ViewFactorySortFilterModel2::ViewFactorySortFilterModel2(QObject *parent) : QSortFilterProxyModel(parent)
{
    ViewFactoryModel2 *model = new ViewFactoryModel2(this);
    setSourceModel(model);
}

bool ViewFactorySortFilterModel2::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    ViewFactoryModel2 *model = static_cast<ViewFactoryModel2*>(sourceModel());

    auto index = model->index(sourceRow, 0, sourceParent);
    auto manager = DirectoryViewFactoryManager2::getInstance();
    auto viewId = index.data(Qt::UserRole).toString();
    auto factory = manager->getFactory(viewId);

    return factory->supportUri(model->m_current_uri);

    int zoom_level_hint = factory->zoom_level_hint();
    int priority = factory->priority(model->m_current_uri);

    auto same_zoom_level_factories = model->m_factory_hash.values(zoom_level_hint);
    bool accept = true;
    for (auto pair : same_zoom_level_factories) {
        if (pair.first > priority) {
            accept = false;
            break;
        }
    }
    return accept;
}

bool ViewFactorySortFilterModel2::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    auto manager = DirectoryViewFactoryManager2::getInstance();

    auto leftId = left.data(Qt::UserRole).toString();
    auto rightId = right.data(Qt::UserRole).toString();

    auto leftFactory = manager->getFactory(leftId);
    auto rightFactory = manager->getFactory(rightId);

    return leftFactory->zoom_level_hint()<rightFactory->zoom_level_hint();
}

void ViewFactorySortFilterModel2::setDirectoryUri(const QString &uri)
{
    ViewFactoryModel2 *model = static_cast<ViewFactoryModel2*>(sourceModel());
    model->setDirectoryUri(uri);
    sort(0);
}

const QModelIndex ViewFactorySortFilterModel2::getIndexFromViewId(const QString &viewId)
{
    ViewFactoryModel2 *model = static_cast<ViewFactoryModel2*>(sourceModel());
    auto sourceIndex = model->getIndexFromViewId(viewId);
    return mapFromSource(sourceIndex);
}

const QString ViewFactorySortFilterModel2::getHighestPriorityViewId(int zoom_level_hint)
{
    ViewFactoryModel2 *model = static_cast<ViewFactoryModel2*>(sourceModel());
    return model->getHighestPriorityViewId(zoom_level_hint);
}

const QStringList ViewFactorySortFilterModel2::supportViewIds()
{
    //QStringList l;
    ViewFactoryModel2 *model = static_cast<ViewFactoryModel2*>(sourceModel());
    auto l = model->supportViewIds();
    l.sort();
    return l;
    //return l;
}

const QIcon ViewFactorySortFilterModel2::iconFromViewId(const QString &viewId)
{
    auto manager = DirectoryViewFactoryManager2::getInstance();
    auto factory = manager->getFactory(viewId);
    if (factory) {
        return factory->icon();
    }
    return QIcon();
}

const QString ViewFactorySortFilterModel2::getViewDisplayNameFromId(const QString &viewId)
{
    auto manager = DirectoryViewFactoryManager2::getInstance();
    return manager->getFactory(viewId)->viewName();
}
