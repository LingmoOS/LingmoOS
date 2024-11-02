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

#include "app-group-model.h"
#include <QDebug>

namespace LingmoUIMenu {

AppGroupModel::AppGroupModel(QObject *parent) : QAbstractProxyModel(parent)
{

}

QModelIndex AppGroupModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column != 0 || row < 0) {
        return {};
    }

    // 通过parent找到所属的组，将组的信息放入index附带数据中，作为判断依据
    if (parent.isValid()) {
        const auto subItems = m_groups.at(parent.row());
        if (row >= subItems->size()) {
            return {};
        }

        return createIndex(row, column, subItems);
    }

    return createIndex(row, column, nullptr);
}

QModelIndex AppGroupModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return {};
    }

    auto subItems = static_cast<QVector<int>*>(child.internalPointer());
    if (subItems) {
        return createIndex(m_groups.indexOf(subItems), 0);
    }

    return {};
}

bool AppGroupModel::hasChildren(const QModelIndex &parent) const
{
    if (!sourceModel()) {
        return false;
    }

    // root
    if (!parent.isValid()) {
        return !m_groups.isEmpty();
    }

    // child, 两层
    if (parent.parent().isValid()) {
        return false;
    }

    return true;
}

int AppGroupModel::rowCount(const QModelIndex &parent) const
{
    if (!sourceModel()) {
        return 0;
    }

    // root
    if (!parent.isValid()) {
        return m_groups.size();
    }

    if (parent.parent().isValid()) {
        return 0;
    }

    int i = parent.row();
    if (i < 0 || i >= m_groups.size()) {
        return 0;
    }

    return m_groups.at(i)->size();
}

int AppGroupModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QHash<int, QByteArray> AppGroupModel::roleNames() const
{
    return QAbstractItemModel::roleNames();
}

void AppGroupModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (AppGroupModel::sourceModel() == sourceModel) {
        return;
    }

    beginResetModel();
    if (AppGroupModel::sourceModel()) {
        AppGroupModel::sourceModel()->disconnect(this);
    }

    QAbstractProxyModel::setSourceModel(sourceModel);

    if (sourceModel) {
        rebuildAppGroups();

        connect(sourceModel, &QAbstractItemModel::dataChanged, this, &AppGroupModel::onDataChanged);
        connect(sourceModel, &QAbstractItemModel::layoutChanged, this, &AppGroupModel::onLayoutChanged);
        connect(sourceModel, &QAbstractItemModel::rowsInserted, this, &AppGroupModel::onRowsInserted);
        connect(sourceModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &AppGroupModel::onRowsAboutToBeRemoved);
        connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, [=] (const QModelIndex &parent, int first, int last) {
            if (m_needRebuild) {
                beginResetModel();
                rebuildAppGroups();
                endResetModel();
                m_needRebuild = false;

            } else if (!parent.isValid()) {
                reLocationIndex(first, -(last - first + 1));
            }
        });
        connect(sourceModel, &QAbstractItemModel::modelReset, this, [=] {
            beginResetModel();
            rebuildAppGroups();
            endResetModel();
        });
    }

    endResetModel();
}

QModelIndex AppGroupModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!sourceModel() || !proxyIndex.isValid()) {
        return {};
    }

    auto subItems = static_cast<QVector<int>*>(proxyIndex.internalPointer());
    if (subItems) {
        int idx = subItems->at(proxyIndex.row());
        return sourceModel()->index(idx, 0);
    }

    return {};
}

QModelIndex AppGroupModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceModel() || !sourceIndex.isValid()) {
        return {};
    }

    int groupIndex = findGroupIndex(sourceIndex);
    if (groupIndex < 0) {
        return {};
    }

    int tempIndex = m_groups.at(groupIndex)->indexOf(sourceIndex.row());
    return index(tempIndex, 0, index(groupIndex, 0, QModelIndex()));
}

QVariant AppGroupModel::data(const QModelIndex &proxyIndex, int role) const
{
    if (!checkIndex(proxyIndex, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    if (proxyIndex.parent().isValid()) {
        return QAbstractProxyModel::data(proxyIndex, role);
    }

    if (role == DataEntity::Name || role == DataEntity::Group) {
        return sourceModel()->index(m_groups.at(proxyIndex.row())->first(), 0).data(DataEntity::Group);
    }

    return {};
}

void AppGroupModel::rebuildAppGroups()
{
    qDeleteAll(m_groups);
    m_groups.clear();

    for (int i = 0; i < sourceModel()->rowCount(); ++i) {
        int groupIndex = findGroupIndex(sourceModel()->index(i, 0));
        QVector<int> *subItems {nullptr};
        if (groupIndex < 0) {
            subItems = new QVector<int>();
            m_groups.append(subItems);
        } else {
            subItems = m_groups[groupIndex];
        }

        subItems->append(i);
    }
}

int AppGroupModel::findGroupIndex(const QModelIndex &sourceIndex) const
{
    for (int i = 0; i < m_groups.size(); ++i) {
        const QVector<int> *subItems = m_groups.at(i);
        if (subItems->isEmpty()) {
            continue;
        }

        // 使用group属性进行分组
        QString groupA = sourceIndex.data(DataEntity::Group).toString();
        QString groupB = sourceModel()->index(subItems->at(0), 0).data(DataEntity::Group).toString();
        if (groupA == groupB) {
            return i;
        }
    }

    return -1;
}

void AppGroupModel::insertApp(int groupIndex, const QModelIndex &sourceIndex)
{
    int newIndex = sourceIndex.row();
    if (groupIndex < 0 || groupIndex >= m_groups.size()) {
        if (newIndex > 0) {
            // 查找前一个item的index
            newIndex = findGroupIndex(sourceModel()->index(--newIndex, 0));
            if (newIndex < 0) {
                newIndex = m_groups.size();
            } else {
                ++newIndex;
            }
        }

        beginInsertRows(QModelIndex(), newIndex, newIndex);
        m_groups.insert(newIndex, new QVector<int>(1, sourceIndex.row()));
        endInsertRows();
        return;
    }

    int index = 0;
    QVector<int> *subItems = m_groups[groupIndex];
    for (; index < subItems->size(); ++index) {
        if (newIndex < subItems->at(index)) {
            break;
        }
    }

    beginInsertRows(AppGroupModel::index(groupIndex, 0, QModelIndex()), index, index);
    subItems->insert(index, newIndex);
    endInsertRows();
}

// slots
void AppGroupModel::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
        QModelIndex proxyIndex = mapFromSource(sourceModel()->index(i, 0));
        Q_EMIT dataChanged(proxyIndex, proxyIndex, roles);
    }
}

void AppGroupModel::onLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint)
{
    Q_UNUSED(parents)
    Q_UNUSED(hint)
    beginResetModel();
    rebuildAppGroups();
    endResetModel();
}

void AppGroupModel::onRowsInserted(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid()) {
        return;
    }

    if (first < (sourceModel()->rowCount() - 1)) {
        reLocationIndex(first, (last - first + 1));
    }

    for (int i = first; i <= last; ++i) {
        QModelIndex sourceIndex = sourceModel()->index(i, 0, parent);
        insertApp(findGroupIndex(sourceIndex), sourceIndex);
    }
}

void AppGroupModel::reLocationIndex(int base, int offset)
{
    for (QVector<int> *group : m_groups) {
        QMutableVectorIterator<int> it(*group);
        while (it.hasNext()) {
            const int &value = it.next();
            if (value >= base) {
                it.setValue(value + offset);
            }
        }
    }
}

void AppGroupModel::onRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid()) {
        return;
    }

    int groupIndex = 0, itemIndex = 0;
    for (int i = first; i <= last; ++i) {
        groupIndex = findGroupIndex(sourceModel()->index(i, 0));
        if (groupIndex < 0) {
            continue;
        }

        auto subItems = m_groups[groupIndex];
        itemIndex = subItems->indexOf(i);
        if (itemIndex < 0) {
            // 如果出现错误，那么重新构建映射关系
            m_needRebuild = true;
            break;
        }

        if (subItems->size() > 1) {
            // 删除组里的元素
            beginRemoveRows(index(groupIndex, 0, QModelIndex()), itemIndex, itemIndex);
            subItems->removeAt(itemIndex);
            endRemoveRows();
        } else {
            // 删除组
            beginRemoveRows(QModelIndex(), groupIndex, groupIndex);
            delete m_groups.takeAt(groupIndex);
            endRemoveRows();
        }
    }
}

int AppGroupModel::findLabelIndex(const QString &label) const
{
    int rowCount = AppGroupModel::rowCount(QModelIndex());
    for (int i = 0; i < rowCount; ++i) {
        if (index(i, 0, QModelIndex()).data(DataEntity::Group).toString() == label) {
            return i;
        }
    }

    return -1;
}

} // LingmoUIMenu
