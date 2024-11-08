/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
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

#include "label-model.h"
#include "data-provider-manager.h"

namespace LingmoUIMenu {

LabelModel::LabelModel(QObject *parent) : QAbstractListModel(parent)
{
    reloadLabelData();
    connect(DataProviderManager::instance(),&DataProviderManager::pluginChanged, this,&LabelModel::reloadLabelData);
    connect(DataProviderManager::instance(),&DataProviderManager::labelChanged, this,&LabelModel::reloadLabelData);
}

int LabelModel::rowCount(const QModelIndex &parent) const
{
    return m_labels.size();
}

QVariant LabelModel::data(const QModelIndex &index, int role) const
{
    int i = index.row();
    if (i < 0 || i >= m_labels.size()) {
        return {};
    }

    switch (role) {
        case LabelItem::IsDisable:
            return m_labels.at(i).isDisable();
        case LabelItem::Id:
            return m_labels.at(i).id();
        case LabelItem::Index:
            return m_labels.at(i).index();
        case LabelItem::DisplayName:
            return m_labels.at(i).displayName();
        default:
            break;
    }

    return {};
}

QHash<int, QByteArray> LabelModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names.insert(LabelItem::IsDisable, "isDisable");
    names.insert(LabelItem::Id, "id");
    names.insert(LabelItem::Index, "index");
    names.insert(LabelItem::DisplayName, "displayName");
    return names;
}

void LabelModel::reloadLabelData()
{
    QVector<LabelItem> labels = DataProviderManager::instance()->labels();
    Q_EMIT beginResetModel();
    m_labels.swap(labels);
    Q_EMIT endResetModel();
}

} // LingmoUIMenu
