/*
    SPDX-FileCopyrightText: 2015 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: David Faure <david.faure@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "krearrangecolumnsproxymodel.h"

class KRearrangeColumnsProxyModelPrivate
{
public:
    QList<int> m_sourceColumns;
};

KRearrangeColumnsProxyModel::KRearrangeColumnsProxyModel(QObject *parent)
    : QIdentityProxyModel(parent)
    , d_ptr(new KRearrangeColumnsProxyModelPrivate)
{
}

KRearrangeColumnsProxyModel::~KRearrangeColumnsProxyModel()
{
}

void KRearrangeColumnsProxyModel::setSourceColumns(const QList<int> &columns)
{
    // We could use layoutChanged() here, but we would have to map persistent
    // indexes from the old to the new location...
    beginResetModel();
    d_ptr->m_sourceColumns = columns;
    endResetModel();
}

int KRearrangeColumnsProxyModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (!sourceModel()) {
        return 0;
    }
    return d_ptr->m_sourceColumns.count();
}

int KRearrangeColumnsProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_ASSERT(parent.isValid() ? parent.model() == this : true);
    if (!sourceModel()) {
        return 0;
    }
    if (parent.column() > 0) {
        return 0;
    }
    // The parent in the source model is on column 0, whatever swapping we are doing
    const QModelIndex sourceParent = mapToSource(parent).sibling(parent.row(), 0);
    return sourceModel()->rowCount(sourceParent);
}

bool KRearrangeColumnsProxyModel::hasChildren(const QModelIndex &parent) const
{
    Q_ASSERT(parent.isValid() ? parent.model() == this : true);
    if (!sourceModel()) {
        return false;
    }
    if (d_ptr->m_sourceColumns.isEmpty()) { // no columns configured yet
        return false;
    }
    if (parent.column() > 0) {
        return false;
    }
    const QModelIndex sourceParent = mapToSource(parent).sibling(parent.row(), 0);
    return sourceModel()->rowCount(sourceParent) > 0;
}

QModelIndex KRearrangeColumnsProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_ASSERT(parent.isValid() ? parent.model() == this : true);
    Q_ASSERT(row >= 0);
    Q_ASSERT(column >= 0);

    // Only first column has children
    if (parent.column() > 0) {
        return {};
    }

    if (!sourceModel()) {
        return {};
    }
    if (d_ptr->m_sourceColumns.isEmpty()) {
        return {};
    }

    // The parent in the source model is on column 0, whatever swapping we are doing
    const QModelIndex sourceParent = mapToSource(parent).sibling(parent.row(), 0);

    // Find the child in the source model, we need its internal pointer
    const QModelIndex sourceIndex = sourceModel()->index(row, sourceColumnForProxyColumn(column), sourceParent);
    if (!sourceIndex.isValid()) {
        return QModelIndex();
    }

    return createIndex(row, column, sourceIndex.internalPointer());
}

QModelIndex KRearrangeColumnsProxyModel::parent(const QModelIndex &child) const
{
    Q_ASSERT(child.isValid() ? child.model() == this : true);
    const QModelIndex sourceIndex = mapToSource(child);
    const QModelIndex sourceParent = sourceIndex.parent();
    if (!sourceParent.isValid()) {
        return QModelIndex();
    }
    return createIndex(sourceParent.row(), 0, sourceParent.internalPointer());
}

QVariant KRearrangeColumnsProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (!sourceModel() || section >= d_ptr->m_sourceColumns.count()) {
            return QVariant();
        }
        const int sourceCol = sourceColumnForProxyColumn(section);
        return sourceModel()->headerData(sourceCol, orientation, role);
    } else {
        return QIdentityProxyModel::headerData(section, orientation, role);
    }
}

QModelIndex KRearrangeColumnsProxyModel::sibling(int row, int column, const QModelIndex &idx) const
{
    if (column >= d_ptr->m_sourceColumns.count()) {
        return QModelIndex();
    }
    return index(row, column, idx.parent());
}

QModelIndex KRearrangeColumnsProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) {
        return QModelIndex();
    }
    Q_ASSERT(sourceIndex.model() == sourceModel());
    const int proxyColumn = proxyColumnForSourceColumn(sourceIndex.column());
    return createIndex(sourceIndex.row(), proxyColumn, sourceIndex.internalPointer());
}

QModelIndex KRearrangeColumnsProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid()) {
        return QModelIndex();
    }
    return createSourceIndex(proxyIndex.row(), sourceColumnForProxyColumn(proxyIndex.column()), proxyIndex.internalPointer());
}

int KRearrangeColumnsProxyModel::proxyColumnForSourceColumn(int sourceColumn) const
{
    // If this is too slow, we could add a second QList with index=logical_source_column value=desired_pos_in_proxy.
    return d_ptr->m_sourceColumns.indexOf(sourceColumn);
}

int KRearrangeColumnsProxyModel::sourceColumnForProxyColumn(int proxyColumn) const
{
    Q_ASSERT(proxyColumn >= 0);
    Q_ASSERT(proxyColumn < d_ptr->m_sourceColumns.size());
    return d_ptr->m_sourceColumns.at(proxyColumn);
}

#include "moc_krearrangecolumnsproxymodel.cpp"
