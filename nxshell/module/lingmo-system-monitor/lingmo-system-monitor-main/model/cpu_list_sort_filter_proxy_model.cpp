// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cpu_list_sort_filter_proxy_model.h"

#include "cpu_list_model.h"

CPUListSortFilterProxyModel::CPUListSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_sortKey {kSortByIndex}
{
}

bool CPUListSortFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &) const
{
    bool ok = false;
    ;

    QModelIndex idx = sourceModel()->index(row, 0);

    if (idx.isValid()) {
        QString lgIdx = QString("%1").arg(idx.data(CPUListModel::kPropCPUIndex).toInt());
        // search format (list of cpu indices with comma separated): 1,3,...,10,...,...
        ok |= filterRegExp().pattern().contains(lgIdx);
    }

    return ok;
}

bool CPUListSortFilterProxyModel::filterAcceptsColumn(int column, const QModelIndex &parent) const
{
    return QSortFilterProxyModel::filterAcceptsColumn(column, parent);
}

bool CPUListSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QModelIndex idx;

    switch (m_sortKey) {
    case kSortByIndex: {
        int lhs {0}, rhs {0};

        idx = sourceModel()->index(left.row(), 0);
        if (idx.isValid())
            lhs = idx.data(CPUListModel::kPropCPUIndex).toInt();

        idx = sourceModel()->index(right.row(), 0);
        if (idx.isValid())
            rhs = idx.data(CPUListModel::kPropCPUIndex).toInt();

        return lhs < rhs;
    }
    case kSortByUsage: {
        qreal lhs {.0}, rhs {.0};

        idx = sourceModel()->index(left.row(), CPUListModel::kPropCPUUsage);
        if (idx.isValid())
            lhs = idx.data(CPUListModel::kValueRole).toReal();

        idx = sourceModel()->index(right.row(), CPUListModel::kPropCPUUsage);
        if (idx.isValid())
            rhs = idx.data(CPUListModel::kValueRole).toReal();

        return lhs < rhs;
    }
    default:
        break;
    } // ::switch

    return QSortFilterProxyModel::lessThan(left, right);
}
