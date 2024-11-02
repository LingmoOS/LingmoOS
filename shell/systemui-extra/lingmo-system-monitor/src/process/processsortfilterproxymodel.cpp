/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "processsortfilterproxymodel.h"
#include "process_list.h"
#include "processtablemodel.h"

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
    if (search == m_search)
        return ;
    m_search = search;

    // in chinese locale, we convert hanzi to pinyin words to help filter out processes named with pinyin
    if (QLocale::system().language() == QLocale::Chinese) {
        //m_hanwords = util::common::convHanToLatin(search);
    }

    // set search pattern & do the filter
    setFilterRegExp(QRegExp(search, Qt::CaseInsensitive));
}

void ProcessSortFilterProxyModel::setFilterType(int type)
{
    m_fileterType = type;
    setFilterRegExp(filterRegExp());
}

// filters the row of specified parent with given pattern
bool ProcessSortFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    bool filter = false;
    const QModelIndex &pid = sourceModel()->index(row, ProcessTableModel::ProcessIdColumn, parent);
    int apptype = pid.data(Qt::UserRole + 3).toInt();
    if (m_fileterType == NoFilter)
        filter = true;
    else if (m_fileterType == FilterCurrentUser && apptype > NoFilter)
        filter = true;
    else if (m_fileterType == FilterActiced && apptype == FilterActiced)
        filter = true;

    if (!filter) return false;

    if (filterRegExp().isEmpty()) {
        return true;
    }
    
    bool rc = false;
    const QModelIndex &name = sourceModel()->index(row, ProcessTableModel::ProcessNameColumn, parent);
    // display name or name matches pattern
    if (name.isValid()) {
        rc |= name.data().toString().contains(filterRegExp());
        if (rc) return rc;

        rc |= name.data(Qt::UserRole).toString().contains(filterRegExp());
        if (rc) return rc;

        if (!m_hanwords.isEmpty() && QLocale::system().language() == QLocale::Chinese) {
            // pinyin matches pattern
            rc |= name.data(Qt::UserRole).toString().contains(m_hanwords);
            if (rc) return rc;
        }
    }

    // pid matches pattern
    if (pid.isValid())
        rc |= pid.data().toString().contains(filterRegExp());
    if (rc) return rc;

    // user name matches pattern
    const QModelIndex &user = sourceModel()->index(row, ProcessTableModel::ProcessUserColumn, parent);
    if (user.isValid())
        rc |= user.data().toString().contains(filterRegExp());

    return rc;
}

// compare two items with the specified index
bool ProcessSortFilterProxyModel::lessThan(const QModelIndex &m_left, const QModelIndex &m_right) const
{
    const QModelIndex &right = m_left;
    const QModelIndex &left = m_right;
    int sortcolumn = sortColumn();
    switch (sortcolumn) {
    case ProcessTableModel::ProcessNameColumn: {
        const QString &lhs = left.data(Qt::UserRole + 1).toString();
        const QString &rhs = right.data(Qt::UserRole + 1).toString();
        int rc = lhs.localeAwareCompare(rhs);

        if (rc == 0) {
            return left.sibling(left.row(), ProcessTableModel::ProcessCpuColumn)
                   .data(Qt::UserRole) <
                   right.sibling(right.row(), ProcessTableModel::ProcessCpuColumn)
                   .data(Qt::UserRole);
        } else
            return rc < 0;
    }
    case ProcessTableModel::ProcessUserColumn: {
        const QString &lhs = left.data(Qt::DisplayRole).toString();
        const QString &rhs = right.data(Qt::DisplayRole).toString();
        return lhs.localeAwareCompare(rhs) < 0;
    }
    case ProcessTableModel::ProcessMemoryColumn: {
        const QVariant &lmem = left.data(Qt::UserRole);
        const QVariant &rmem = right.data(Qt::UserRole);

        // compare memory usage first, then by cpu time
        if (lmem == rmem) {
            return left.sibling(left.row(), ProcessTableModel::ProcessCpuColumn)
                   .data(Qt::UserRole) <
                   right.sibling(right.row(), ProcessTableModel::ProcessCpuColumn)
                   .data(Qt::UserRole);
        } else {
            return lmem < rmem;
        }
    }
    case ProcessTableModel::ProcessCpuColumn: {
        const QVariant &lcpu = left.data(Qt::UserRole);
        const QVariant &rcpu = right.data(Qt::UserRole);

        const QVariant &lmem = left.sibling(left.row(), ProcessTableModel::ProcessMemoryColumn).data(Qt::UserRole);
        const QVariant &rmem = right.sibling(right.row(), ProcessTableModel::ProcessMemoryColumn).data(Qt::UserRole);

        // compare cpu time first, then by memory usage
        if (qFuzzyCompare(lcpu.toReal(), rcpu.toReal())) {
            return lmem < rmem;
        } else {
            return lcpu < rcpu;
        }
    }
    case ProcessTableModel::ProcessFlowNetColumn: {
        qreal lkbs, rkbs;
        lkbs = left.data(Qt::UserRole).toReal();
        rkbs = right.data(Qt::UserRole).toReal();

        // compare upload speed first, then by total send bytes
        if (qFuzzyCompare(lkbs, rkbs)) {
            return left.data(Qt::UserRole + 1).toULongLong() < right.data(Qt::UserRole + 1).toULongLong();
        } else {
            return lkbs < rkbs;
        }
    }
    case ProcessTableModel::ProcessIdColumn: {
        // compare pid
        return left.data(Qt::UserRole) < right.data(Qt::UserRole);
    }
    case ProcessTableModel::ProcessDiskIoColumn: {
        // compare disk read speed
        return left.data(Qt::UserRole) < right.data(Qt::UserRole);
    }
    case ProcessTableModel::ProcessNiceColumn: {
        // higher priority has negative number
        return !(left.data(Qt::UserRole) < right.data(Qt::UserRole));
    }
    default:
        break;
    }

    return QSortFilterProxyModel::lessThan(left, right);
}
