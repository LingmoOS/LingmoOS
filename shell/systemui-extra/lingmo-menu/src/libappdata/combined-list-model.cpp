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

#include "combined-list-model.h"

#include <QDebug>

namespace LingmoUIMenu {

// CombinedListModel
CombinedListModel::CombinedListModel(QObject *parent) : QAbstractItemModel(parent)
{

}

QModelIndex CombinedListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column != 0 || row < 0 || row >= rowCount(QModelIndex())) {
        return {};
    }

    int start = 0;
    for (const auto &pair : m_subModels) {
        int rc = pair.second;
        if (row < (start + rc)) {
            return createIndex(row, 0, pair.first);
        } else {
            start += rc;
        }
    }

    return {};
}

QModelIndex CombinedListModel::parent(const QModelIndex &child) const
{
    return {};
}

int CombinedListModel::rowCount(const QModelIndex &parent) const
{
    int count = 0;
    for (const auto &pair : m_subModels) {
        count += pair.second;
    }

    return count;
}

int CombinedListModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QHash<int, QByteArray> CombinedListModel::roleNames() const
{
    return QAbstractItemModel::roleNames();
}

QModelIndex CombinedListModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid()) {
        return {};
    }

    auto sourceModel = static_cast<QAbstractItemModel*>(proxyIndex.internalPointer());
    if (!sourceModel) {
        return {};
    }

    int offset = offsetOfSubModel(sourceModel);
    if (offset < 0) {
        return {};
    }

    return sourceModel->index(proxyIndex.row() - offset, 0);
}

QModelIndex CombinedListModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) {
        return {};
    }

    int offset = offsetOfSubModel(sourceIndex.model());
    if (offset < 0) {
        return {};
    }

    return createIndex(offset + sourceIndex.row(), 0);
}

int CombinedListModel::offsetOfSubModel(const QAbstractItemModel *subModel) const
{
    int offset = 0;
    for (const auto &pair : m_subModels) {
        if (pair.first == subModel) {
            return offset;
        } else {
            offset += pair.second;
        }
    }

    return -1;
}

QVariant CombinedListModel::data(const QModelIndex &proxyIndex, int role) const
{
    if (!checkIndex(proxyIndex, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    QModelIndex sourceIndex = mapToSource(proxyIndex);
    if (!sourceIndex.isValid()) {
        return {};
    }

    return sourceIndex.data(role);
}

int CombinedListModel::subModelCount() const
{
    return m_subModels.size();
}

const QAbstractItemModel *CombinedListModel::subModelAt(int index) const
{
    if (index < 0 || index >= m_subModels.size()) {
        return nullptr;
    }

    return m_subModels.at(index).first;
}

void CombinedListModel::insertSubModel(QAbstractItemModel *subModel, int index)
{
    if (!subModel || offsetOfSubModel(subModel) >= 0) {
        return;
    }

    beginResetModel();

    if (index < 0 || index > m_subModels.size()) {
        m_subModels.append({subModel, subModel->rowCount()});
    } else {
        m_subModels.insert(index, {subModel, subModel->rowCount()});
    }

    // 在删除前通知上层model进行处理
    connect(subModel, &QAbstractItemModel::rowsAboutToBeRemoved,
            this, [subModel, this] (const QModelIndex &parent, int first, int last) {
            int offset = offsetOfSubModel(subModel);
            if (offset >= 0) {
                beginRemoveRows(mapFromSource(parent), offset + first, offset + last);
                for (auto &pair : m_subModels) {
                    if (pair.first == subModel) {
//                        pair.second = subModel->rowCount();
                        pair.second = pair.second - (last - first + 1);
                    }
                }
                endRemoveRows();
            }
    });

    connect(subModel, &QAbstractItemModel::rowsInserted,
            this, [subModel, this] (const QModelIndex &parent, int first, int last) {
            int offset = offsetOfSubModel(subModel);
            if (offset >= 0) {
                beginInsertRows(mapFromSource(parent), offset + first, offset + last);
                for (auto &pair : m_subModels) {
                    if (pair.first == subModel) {
//                        pair.second = subModel->rowCount();
                        pair.second = pair.second + (last - first + 1);
                    }
                }
                endInsertRows();
            }
    });

    connect(subModel, &QAbstractItemModel::dataChanged,
            this, [subModel, this] (const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
        Q_EMIT dataChanged( mapFromSource(topLeft),  mapFromSource(bottomRight), roles);
    });

    connect(subModel, &QAbstractItemModel::modelReset, this, [subModel, this] {
        beginResetModel();
        for (auto &pair : m_subModels) {
            if (pair.first == subModel) {
                pair.second = pair.second;
            }
        }
        endResetModel();
    });

    connect(subModel, &QAbstractItemModel::layoutAboutToBeChanged, this, [this] {
        emit layoutAboutToBeChanged();
    });

    connect(subModel, &QAbstractItemModel::layoutChanged, this, [this] {
        emit layoutChanged();
    });

    endResetModel();
}

void CombinedListModel::removeSubModel(int index)
{
    if (index < 0 || index >= m_subModels.size()) {
        return;
    }

    QPair<QAbstractItemModel*, int> pair = m_subModels[index];
    int offset = offsetOfSubModel(pair.first);

    beginRemoveRows(QModelIndex(), offset, offset + pair.second);
    pair = m_subModels.takeAt(index);
    disconnect(pair.first, nullptr, this, nullptr);
    endRemoveRows();
}

void CombinedListModel::removeSubModel(QAbstractItemModel *subModel)
{
    removeSubModel(indexOfSubModel(subModel));
}

int CombinedListModel::indexOfSubModel(QAbstractItemModel *subModel)
{
    for (int i = 0; i < m_subModels.size(); ++i) {
        if (m_subModels.at(i).first == subModel) {
            return i;
        }
    }

    return -1;
}

} // LingmoUIMenu
