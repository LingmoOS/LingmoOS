// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "system_service_sort_filter_proxy_model.h"
#include "system_service_table_model.h"
#include "common/common.h"

// proxy model constructor
SystemServiceSortFilterProxyModel::SystemServiceSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

// check if more data can be fetched for given parent
bool SystemServiceSortFilterProxyModel::canFetchMore(const QModelIndex &parent) const
{
    return QSortFilterProxyModel::canFetchMore(parent);
}

// fetches any available data for the items with the parent specified by the parent index
void SystemServiceSortFilterProxyModel::fetchMore(const QModelIndex &parent)
{
    if (!filterRegExp().pattern().isEmpty()) {
        // when search content is non-empty, to avoid model refresh malfunction,
        // we need load all contents in a batch.
        while (canFetchMore(parent)) {
            QSortFilterProxyModel::fetchMore(parent);
        }
    }

    if (canFetchMore(parent))
        QSortFilterProxyModel::fetchMore(parent);
    // fetch one more time to fix scroll malfuntion when scroll to the last page with key press
    if (canFetchMore(parent))
        QSortFilterProxyModel::fetchMore(parent);
}

// filters the row of specified parent with given pattern
bool SystemServiceSortFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    // service name
    QModelIndex index0 =
        sourceModel()->index(row, SystemServiceTableModel::kSystemServiceNameColumn, parent);
    // service description
    QModelIndex index1 =
        sourceModel()->index(row, SystemServiceTableModel::kSystemServiceDescriptionColumn, parent);
    // main pid of the service
    QModelIndex index2 =
        sourceModel()->index(row, SystemServiceTableModel::kSystemServicePIDColumn, parent);

    // filters the row out if none of the service's name, description or pid matches the pattern
    bool rc = false;
    if (index0.isValid())
        rc |= sourceModel()->data(index0).toString().contains(filterRegExp());
    if (index1.isValid())
        rc |= sourceModel()->data(index1).toString().contains(filterRegExp());
    if (index2.isValid())
        rc |= sourceModel()->data(index2).toString().contains(filterRegExp());

    return rc;
}

// filters the column of specified parent with given pattern
bool SystemServiceSortFilterProxyModel::filterAcceptsColumn(int column,
                                                            const QModelIndex &parent) const
{
    // do nothing here, proxy the process to base class directly
    return QSortFilterProxyModel::filterAcceptsColumn(column, parent);
}

// compare two items with the specified index
bool SystemServiceSortFilterProxyModel::lessThan(const QModelIndex &left,
                                                 const QModelIndex &right) const
{
    switch (sortColumn()) {
    case SystemServiceTableModel::kSystemServicePIDColumn:
        // sort pid column with integer comparision
        return left.data().toUInt() < right.data().toUInt();
    case SystemServiceTableModel::kSystemServiceNameColumn:
    case SystemServiceTableModel::kSystemServiceDescriptionColumn: {
        const QString &lhs = left.data(Qt::DisplayRole).toString();
        const QString &rhs = right.data(Qt::DisplayRole).toString();

        bool lstartHz = common::startWithHanzi(lhs);
        bool rstartHz = common::startWithHanzi(rhs);
        if (!lstartHz && rstartHz)
            return true;

        if (lstartHz && !rstartHz)
            return false;

        return lhs.localeAwareCompare(rhs) < 0;
    }
    default:
        break;
    }

    // sort other columns with default algorithm
    return QSortFilterProxyModel::lessThan(left, right);
}
