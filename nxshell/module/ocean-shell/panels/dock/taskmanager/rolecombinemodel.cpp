// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rolecombinemodel.h"

#include <algorithm>

RoleCombineModel::RoleCombineModel(QAbstractItemModel* major, QAbstractItemModel* minor, int majorRoles, CombineFunc func, QObject* parent)
    : QAbstractProxyModel(parent)
{
    setSourceModel(major);
    m_minor = minor;
    // create minor row & column map
    int rowCount = major->rowCount();
    int columnCount = major->columnCount();
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < columnCount; j++) {
            QModelIndex majorIndex = major->index(i, j);
            QModelIndex minorIndex = func(majorIndex.data(majorRoles), m_minor);
            if (majorIndex.isValid() && minorIndex.isValid())
                m_indexMap[qMakePair(i, j)] = qMakePair(minorIndex.row(), minorIndex.column());
        }
    }

    connect(sourceModel(), &QAbstractItemModel::rowsInserted, this, [this, majorRoles, func](const QModelIndex &parent, int first, int last) {
        beginInsertRows(index(parent.row(), parent.column()), first, last);
        for (int i = first; i <= last; i++) {
            QModelIndex majorIndex = sourceModel()->index(i, 0);
            QModelIndex minorIndex = func(majorIndex.data(majorRoles), m_minor);
            if (majorIndex.isValid() && minorIndex.isValid())
                m_indexMap[qMakePair(i, 0)] = qMakePair(minorIndex.row(), minorIndex.column());
        }
        endInsertRows();
    });
    connect(sourceModel(), &QAbstractItemModel::columnsInserted, this, [this, majorRoles, func](const QModelIndex &parent, int first, int last) {
        beginInsertColumns(index(parent.row(), parent.column()), first, last);
        for (int j = first; j <= last; j++) {
            QModelIndex majorIndex = sourceModel()->index(0, j);
            QModelIndex minorIndex = func(majorIndex.data(majorRoles), m_minor);
            if (majorIndex.isValid() && minorIndex.isValid())
                m_indexMap[qMakePair(0, j)] = qMakePair(minorIndex.row(), minorIndex.column());
        }
        endInsertColumns();
    });

    connect(sourceModel(), &QAbstractItemModel::rowsRemoved, this, [this](const QModelIndex &parent, int first, int last) {
        beginRemoveRows(index(parent.row(), parent.column()), first, last);
        for (int i = first; i <= last; i++) {
            if (m_indexMap.contains(qMakePair(i, 0))) {
                m_indexMap.remove(qMakePair(i, 0));
            }
        }
        endRemoveRows();
    });
    connect(sourceModel(), &QAbstractItemModel::columnsRemoved, this, [this](const QModelIndex &parent, int first, int last) {
        beginRemoveColumns(index(parent.row(), parent.column()), first, last);
        for (int j = first; j <= last; j++) {
            if (m_indexMap.contains(qMakePair(0, j))) {
                m_indexMap.remove(qMakePair(0, j));
            }
        }
        endRemoveColumns();
    });

    // connect changedSignal
    connect(major, &QAbstractItemModel::dataChanged, this,
        [this, majorRoles, func](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles){
            for (int i = topLeft.row(); i <= bottomRight.row(); i++) {
                for (int j = topLeft.column(); j <= bottomRight.column(); j++) {
                    QModelIndex majorIndex = sourceModel()->index(i, j);
                    QModelIndex minorIndex = func(majorIndex.data(majorRoles), m_minor);
                    if (majorIndex.isValid() && minorIndex.isValid())
                        m_indexMap[qMakePair(i, j)] = qMakePair(minorIndex.row(), minorIndex.column());
                }
            }

            Q_EMIT dataChanged(index(topLeft.row(), topLeft.column()),
                index(bottomRight.row(), bottomRight.column()),
                roles + (roles.contains(majorRoles) ? m_minorRolesMap.values() : QList<int>())
            );
    });

    // appended roles from minor datachanged
    connect(m_minor, &QAbstractItemModel::dataChanged, this,
        [this, majorRoles, func](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles){
            for (int i = topLeft.row(); i <= bottomRight.row(); i++) {
                for (int j =  topLeft.column(); j <= bottomRight.column(); j++) {
                    auto majorPos = m_indexMap.key(qMakePair(i, j), qMakePair(-1, -1));
                    if (-1 == majorPos.first && -1 == majorPos.second)
                        continue;

                    auto majorIndex = sourceModel()->index(majorPos.first, majorPos.second);
                    if (!majorIndex.isValid())
                        continue;

                    auto minorIndex = func(majorIndex.data(majorRoles), m_minor);
                    if (!minorIndex.isValid())
                        continue;

                    m_indexMap[majorPos] = qMakePair(minorIndex.row(), minorIndex.column());
                    Q_EMIT dataChanged(majorIndex, majorIndex, m_minorRolesMap.values());
                }
            }
    });

    connect(m_minor, &QAbstractItemModel::rowsInserted, this,
        [this, majorRoles, func](const QModelIndex &parent, int first, int last){
        auto rowCount = sourceModel()->rowCount();
        auto columnCount = sourceModel()->columnCount();
        for (int i = 0; i < rowCount; i++) {
            for (int j = 0; j < columnCount; j++) {
                // alreay bind, pass this
                if (m_indexMap.contains(qMakePair(i ,j)))
                    continue;

                QModelIndex majorIndex = sourceModel()->index(i, j);
                QModelIndex minorIndex = func(majorIndex.data(majorRoles), m_minor);
                if (majorIndex.isValid() && minorIndex.isValid())
                    m_indexMap[qMakePair(i, j)] = qMakePair(minorIndex.row(), minorIndex.column());
            }
        }
    });

    // TODO: support columsInserted

    // create minor role map
    auto minorRolenames = m_minor->roleNames();
    auto thisRoleNames = roleNames();
    std::for_each(minorRolenames.constBegin(), minorRolenames.constEnd(), [&minorRolenames, &thisRoleNames, this](auto &roleName){
        m_minorRolesMap.insert(thisRoleNames.key(roleName), minorRolenames.key(roleName));
    });
}

QHash<int, QByteArray> RoleCombineModel::roleNames() const
{
    auto roleNames = sourceModel()->roleNames();
    auto keys = sourceModel()->roleNames().keys();
    auto lastRole = *(std::max_element(keys.constBegin(), keys.constEnd()));
    auto minorRoleNames = m_minor->roleNames().values();
    std::for_each(minorRoleNames.constBegin(), minorRoleNames.constEnd(), [&lastRole, &roleNames, this](auto &roleName){
        roleNames.insert(++lastRole, roleName);
    });

    return roleNames;
}

int RoleCombineModel::rowCount(const QModelIndex &parent) const
{
    return sourceModel()->rowCount();
}

int RoleCombineModel::columnCount(const QModelIndex &parent) const
{
    return sourceModel()->columnCount();
}

QVariant RoleCombineModel::data(const QModelIndex &index, int role) const
{
    if (m_minorRolesMap.contains(role)) {
        int row, column;
        std::tie(row, column) = m_indexMap.value(qMakePair(index.row(), index.column()), qMakePair(-1, -1));
        return m_minor->data(m_minor->index(row, column), m_minorRolesMap[role]);
    } else {
        return sourceModel()->data(sourceModel()->index(index.row(), index.column()), role);
    }
}

bool RoleCombineModel::hasIndex(int row, int column, const QModelIndex &parent) const
{
    return sourceModel()->hasIndex(row, column, parent);
}

QModelIndex RoleCombineModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    auto index = sourceModel()->index(row, column);
    return createIndex(row, column, index.internalPointer());
}

QModelIndex RoleCombineModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

QAbstractItemModel* RoleCombineModel::majorModel() const
{
    return sourceModel();
}

QAbstractItemModel* RoleCombineModel::minorModel() const
{
    return m_minor;
}

QModelIndex RoleCombineModel::mapToSource(const QModelIndex &proxyIndex) const
{
    return sourceModel()->index(proxyIndex.row(), proxyIndex.column());
}

QModelIndex RoleCombineModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    return index(sourceIndex.row(), sourceIndex.column());
}
