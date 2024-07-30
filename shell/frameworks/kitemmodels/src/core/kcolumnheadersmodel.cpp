/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcolumnheadersmodel.h"

class KColumnHeadersModelPrivate
{
public:
    QAbstractItemModel *sourceModel = nullptr;
    int sortColumn = -1;
    Qt::SortOrder sortOrder = Qt::AscendingOrder;
};

KColumnHeadersModel::KColumnHeadersModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new KColumnHeadersModelPrivate)
{
}

KColumnHeadersModel::~KColumnHeadersModel()
{
}

int KColumnHeadersModel::rowCount(const QModelIndex &parent) const
{
    if (!d->sourceModel || parent.isValid()) {
        return 0;
    }

    return d->sourceModel->columnCount();
}

QVariant KColumnHeadersModel::data(const QModelIndex &index, int role) const
{
    if (!d->sourceModel || !index.isValid()) {
        return QVariant{};
    }

    if (role == SortRole) {
        if (index.row() == d->sortColumn) {
            return d->sortOrder;
        } else {
            return QVariant{};
        }
    }

    return sourceModel()->headerData(index.row(), Qt::Horizontal, role);
}

QHash<int, QByteArray> KColumnHeadersModel::roleNames() const
{
    if (!d->sourceModel) {
        return QHash<int, QByteArray>{};
    }

    auto names = d->sourceModel->roleNames();
    names.insert(SortRole, "sort");
    return names;
}

QAbstractItemModel *KColumnHeadersModel::sourceModel() const
{
    return d->sourceModel;
}

void KColumnHeadersModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    if (newSourceModel == d->sourceModel) {
        return;
    }

    if (d->sourceModel) {
        d->sourceModel->disconnect(this);
    }

    beginResetModel();
    d->sourceModel = newSourceModel;
    endResetModel();

    if (newSourceModel) {
        connect(newSourceModel, &QAbstractItemModel::columnsAboutToBeInserted, this, [this](const QModelIndex &, int first, int last) {
            beginInsertRows(QModelIndex{}, first, last);
        });
        connect(newSourceModel, &QAbstractItemModel::columnsInserted, this, [this]() {
            endInsertRows();
        });
        connect(newSourceModel,
                &QAbstractItemModel::columnsAboutToBeMoved,
                this,
                [this](const QModelIndex &, int start, int end, const QModelIndex &, int destination) {
                    beginMoveRows(QModelIndex{}, start, end, QModelIndex{}, destination);
                });
        connect(newSourceModel, &QAbstractItemModel::columnsMoved, this, [this]() {
            endMoveRows();
        });
        connect(newSourceModel, &QAbstractItemModel::columnsAboutToBeRemoved, this, [this](const QModelIndex &, int first, int last) {
            beginRemoveRows(QModelIndex{}, first, last);
        });
        connect(newSourceModel, &QAbstractItemModel::columnsRemoved, this, [this]() {
            endRemoveRows();
        });
        connect(newSourceModel, &QAbstractItemModel::headerDataChanged, this, [this](Qt::Orientation orientation, int first, int last) {
            if (orientation == Qt::Horizontal) {
                Q_EMIT dataChanged(index(first, 0), index(last, 0));
            }
        });
        connect(newSourceModel, &QAbstractItemModel::modelAboutToBeReset, this, [this]() {
            beginResetModel();
        });
        connect(newSourceModel, &QAbstractItemModel::modelReset, this, [this]() {
            endResetModel();
        });
    }
}

int KColumnHeadersModel::sortColumn() const
{
    return d->sortColumn;
}

void KColumnHeadersModel::setSortColumn(int newSortColumn)
{
    if (newSortColumn == d->sortColumn) {
        return;
    }

    auto previousSortColumn = d->sortColumn;

    d->sortColumn = newSortColumn;

    if (previousSortColumn >= 0) {
        Q_EMIT dataChanged(index(previousSortColumn), index(previousSortColumn), {SortRole});
    }

    if (newSortColumn >= 0) {
        Q_EMIT dataChanged(index(newSortColumn), index(newSortColumn), {SortRole});
    }

    Q_EMIT sortColumnChanged();
}

Qt::SortOrder KColumnHeadersModel::sortOrder() const
{
    return d->sortOrder;
}

void KColumnHeadersModel::setSortOrder(Qt::SortOrder newSortOrder)
{
    if (newSortOrder == d->sortOrder) {
        return;
    }

    d->sortOrder = newSortOrder;

    if (d->sortColumn >= 0) {
        Q_EMIT dataChanged(index(d->sortColumn), index(d->sortColumn), {SortRole});
    }

    Q_EMIT sortOrderChanged();
}

#include "moc_kcolumnheadersmodel.cpp"
