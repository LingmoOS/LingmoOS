// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "process_sort_filter_proxy_model.h"
#include "process/process_db.h"
#include "process_table_model.h"
#include "common/han_latin.h"
#include "common/common.h"

#include <QCollator>
#include <QDebug>
#include <QLocale>

// proxy model constructor
ProcessSortFilterProxyModel::ProcessSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

// set search pattern
void ProcessSortFilterProxyModel::setSortFilterString(const QString &search)
{
    m_search = search;

    // in chinese locale, we convert hanzi to pinyin words to help filter out processes named with pinyin
    if (QLocale::system().language() == QLocale::Chinese) {
        m_hanwords = util::common::convHanToLatin(search);
    }

    // set search pattern & do the filter
    setFilterRegExp(QRegExp(search, Qt::CaseInsensitive));
}

void ProcessSortFilterProxyModel::setFilterType(int type)
{
    m_fileterType = type;
    setFilterRegExp(filterRegExp());
}

int ProcessSortFilterProxyModel::rowCount(const QModelIndex &parent) const
{
    return 5;
}

// filters the row of specified parent with given pattern
bool ProcessSortFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    bool filter = false;
    const QModelIndex &pid = sourceModel()->index(row, ProcessTableModel::kProcessPIDColumn, parent);
    int apptype = pid.data(Qt::UserRole + 3).toInt();
    if (m_fileterType == kNoFilter) {
        filter = true;
    } else if (m_fileterType == kFilterCurrentUser && (apptype == kFilterApps || apptype == kFilterCurrentUser)) {
        filter = true;
    } else if (m_fileterType == kFilterApps && apptype == kFilterApps) {
        filter = true;
    }

    if (!filter) return false;

    bool rc = false;
    const QModelIndex &name = sourceModel()->index(row, ProcessTableModel::kProcessNameColumn, parent);
    // display name or name matches pattern
    if (name.isValid()) {
        rc |= name.data().toString().contains(filterRegExp());
        if (rc) return rc;

        rc |= name.data(Qt::UserRole).toString().contains(filterRegExp());
        if (rc) return rc;

        if (QLocale::system().language() == QLocale::Chinese) {
            // pinyin matches pattern
            rc |= name.data(Qt::UserRole).toString().contains(m_hanwords);
            if (rc)
                return rc;
        }
    }

    // pid matches pattern
    if (pid.isValid())
        rc |= pid.data().toString().contains(filterRegExp());
    if (rc) return rc;

    return false;
}

// compare two items with the specified index
bool ProcessSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    int sortcolumn = sortColumn();
    switch (sortcolumn) {
    case ProcessTableModel::kProcessNameColumn: {
        const QString &lhs = left.data(Qt::DisplayRole).toString();
        const QString &rhs = right.data(Qt::DisplayRole).toString();

        bool lstartHz = common::startWithHanzi(lhs);
        bool rstartHz = common::startWithHanzi(rhs);
        if (!lstartHz && rstartHz)
            return true;

        if (lstartHz && !rstartHz)
            return false;

        int rc = lhs.localeAwareCompare(rhs);

        if (rc == 0) {
            return left.sibling(left.row(), ProcessTableModel::kProcessCPUColumn)
                   .data(Qt::UserRole) <
                   right.sibling(right.row(), ProcessTableModel::kProcessCPUColumn)
                   .data(Qt::UserRole);
        } else
            return rc < 0;
    }
    case ProcessTableModel::kProcessCPUColumn: {
        const QVariant &lcpu = left.data(Qt::UserRole);
        const QVariant &rcpu = right.data(Qt::UserRole);

        const QVariant &lmem = left.sibling(left.row(), ProcessTableModel::kProcessNameColumn).data(Qt::UserRole);
        const QVariant &rmem = right.sibling(right.row(), ProcessTableModel::kProcessNameColumn).data(Qt::UserRole);

        // compare cpu time first, then by memory usage
        if (qFuzzyCompare(lcpu.toReal(), rcpu.toReal())) {
            return lmem < rmem;
        } else {
            return lcpu < rcpu;
        }
    }
    default:
        break;
    }

    return QSortFilterProxyModel::lessThan(left, right);
}
