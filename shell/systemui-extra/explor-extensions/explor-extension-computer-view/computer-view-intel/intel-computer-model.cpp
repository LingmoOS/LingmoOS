/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#include "intel-computer-model.h"

#include "intel-abstract-computer-item.h"
#include "intel-computer-volume-item.h"
#include "intel-computer-remote-volume-item.h"
#include "intel-computer-network-item.h"

using namespace Intel;

ComputerModel::ComputerModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    beginResetModel();

    m_parentNode = new AbstractComputerItem(this, nullptr, this);
    auto computerItem = new ComputerVolumeItem(nullptr, this, m_parentNode);
    m_parentNode->m_children<<computerItem;
    computerItem->findChildren();
    computerItem->expand(true);

//    auto remoteItem = new ComputerRemoteVolumeItem("computer:///", this, m_parentNode);
//    m_parentNode->m_children<<remoteItem;
//    remoteItem->findChildren();

    auto networkItem = new ComputerNetworkItem("network:///", this, m_parentNode);
    m_parentNode->m_children<<networkItem;
    networkItem->findChildren();

    endResetModel();
}

void ComputerModel::beginInsertItem(const QModelIndex &parent, int row)
{
    beginInsertRows(parent, row, row);
}

void ComputerModel::endInsterItem()
{
    endInsertRows();
}

void ComputerModel::beginRemoveItem(const QModelIndex &parent, int row)
{
    beginRemoveRows(parent, row, row);
}

void ComputerModel::endRemoveItem()
{
    endRemoveRows();
}

QModelIndex ComputerModel::createItemIndex(int row, QObject *item)
{
    return createIndex(row, 0, item);
}

QModelIndex ComputerModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        auto item = m_parentNode->m_children.at(row);
        return createIndex(row, column, item);
    }

    auto parentNode = static_cast<AbstractComputerItem *>(parent.internalPointer());
    if (parentNode->m_children.count() < row)
        return QModelIndex();

    return createIndex(row, column, parentNode->m_children.at(row));
}

QModelIndex ComputerModel::parent(const QModelIndex &index) const
{
    auto item = static_cast<AbstractComputerItem *>(index.internalPointer());
    if (item->m_parentNode) {
        return item->m_parentNode->itemIndex();
    }
    return QModelIndex();
}

int ComputerModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_parentNode->m_children.count();

    auto item = static_cast<AbstractComputerItem *>(parent.internalPointer());
    return item->m_children.count();
}

int ComputerModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant ComputerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto item = static_cast<AbstractComputerItem *>(index.internalPointer());
    switch (role) {
    case Qt::DisplayRole:
        return item->displayName();
    case Qt::DecorationRole: {
        if (item->m_parentNode == m_parentNode)
            return QVariant();
        return item->icon();
    }
    case Qt::ToolTipRole:
        return item->displayName();
    default:
        return QVariant();
    }
}

bool ComputerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        // FIXME: Implement me!
        Q_EMIT dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags ComputerModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index); // FIXME: Implement me!
}

QString ComputerModel::tryGetVolumeUriFromMountRoot(const QString &mountRootUri)
{
    auto value = m_volumeTargetMap.value(mountRootUri);
    return value;
}

void ComputerModel::refresh()
{
    for (auto child : m_parentNode->m_children) {
        for (auto child2 : child->m_children) {
            if (auto item = qobject_cast<AbstractComputerItem *>(child2)) {
                item->updateInfo();
            }
        }
    }
}
