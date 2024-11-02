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

#include "widget-model.h"
#include "widget-extension-model.h"
#include <QSortFilterProxyModel>
#include <QDebug>

namespace LingmoUIMenu {

// ====== WidgetModel ====== //
WidgetModel::WidgetModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    init();
}

WidgetMetadata::Types WidgetModel::types() const
{
    return m_types;
}

void WidgetModel::setTypes(WidgetMetadata::Types types)
{
    if (m_types == types) {
        return;
    }

    m_types = types;
    invalidateFilter();
}

WidgetMetadata::Flags WidgetModel::flags() const
{
    return m_flags;
}

void WidgetModel::setFlags(WidgetMetadata::Flags flags)
{
    if (m_flags == flags) {
        return;
    }

    m_flags = flags;
    invalidateFilter();
}

bool WidgetModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    bool acceptFlag = m_types.testFlag(index.data(WidgetMetadata::Type).value<WidgetMetadata::TypeValue>());
    if (acceptFlag) {
        return m_flags & index.data(WidgetMetadata::Flag).value<WidgetMetadata::FlagValue>();
    }
    return false;
}

void WidgetModel::init()
{
    QSortFilterProxyModel::setSourceModel(WidgetExtensionModel::instance());
//    invalidateFilter();
}

void WidgetModel::send(int index, const QVariantMap &data)
{
    auto sourceModel = qobject_cast<WidgetExtensionModel*>(QSortFilterProxyModel::sourceModel());
    if (sourceModel) {
        sourceModel->notify(index, data);
    }
}

} // LingmoUIMenu
