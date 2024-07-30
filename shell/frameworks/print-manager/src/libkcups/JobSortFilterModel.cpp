/*
    SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "JobSortFilterModel.h"

#include "JobModel.h"
#include "kcupslib_log.h"

JobSortFilterModel::JobSortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    sort(0);

    connect(this, &JobSortFilterModel::dataChanged, this, &JobSortFilterModel::activeCountChanged);
    connect(this, &JobSortFilterModel::rowsInserted, this, &JobSortFilterModel::activeCountChanged);
    connect(this, &JobSortFilterModel::rowsRemoved, this, &JobSortFilterModel::activeCountChanged);
    connect(this, &JobSortFilterModel::modelReset, this, &JobSortFilterModel::activeCountChanged);
    connect(this, &JobSortFilterModel::dataChanged, this, &JobSortFilterModel::countChanged);
    connect(this, &JobSortFilterModel::rowsInserted, this, &JobSortFilterModel::countChanged);
    connect(this, &JobSortFilterModel::rowsRemoved, this, &JobSortFilterModel::countChanged);
    connect(this, &JobSortFilterModel::modelReset, this, &JobSortFilterModel::countChanged);
}

void JobSortFilterModel::setModel(QAbstractItemModel *model)
{
    if (model == sourceModel()) {
        return;
    }

    QSortFilterProxyModel::setSourceModel(model);
    Q_EMIT sourceModelChanged(model);
}

void JobSortFilterModel::setFilteredPrinters(const QString &printers)
{
    qCDebug(LIBKCUPS) << rowCount() << printers << printers.split(QLatin1Char('|'));
    if (printers.isEmpty()) {
        m_filteredPrinters.clear();
    } else {
        m_filteredPrinters = printers.split(QLatin1Char('|'));
    }
    invalidateFilter();
    Q_EMIT filteredPrintersChanged();
}

QString JobSortFilterModel::filteredPrinters() const
{
    return m_filteredPrinters.join(QLatin1String("|"));
}

int JobSortFilterModel::activeCount() const
{
    int active = 0;
    for (int i = 0; i < rowCount(); ++i) {
        QModelIndex item = index(i, 0);
        if (weightForState(item.data(JobModel::RoleJobState).toInt())) {
            ++active;
        }
    }
    return active;
}

int JobSortFilterModel::count() const
{
    return rowCount();
}

bool JobSortFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    // check if the printer is on the blacklist
    if (!m_filteredPrinters.isEmpty()) {
        return m_filteredPrinters.contains(index.data(JobModel::RoleJobPrinter).toString());
    }

    return true;
}

bool JobSortFilterModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.column() == 0) {
        int leftWeight = weightForState(left.data(JobModel::RoleJobState).toInt());
        int rightWeight = weightForState(right.data(JobModel::RoleJobState).toInt());

        if (leftWeight != rightWeight) {
            return leftWeight > rightWeight;
        } else {
            int leftId = left.data(JobModel::RoleJobId).toInt();
            int rightId = right.data(JobModel::RoleJobId).toInt();

            if (leftWeight) {
                return leftId < rightId;
            }

            return leftId > rightId;
        }
    }
    return QSortFilterProxyModel::lessThan(left, right);
}

int JobSortFilterModel::weightForState(int state) const
{
    switch (state) {
    case IPP_JOB_PROCESSING:
        return 3;
        break;
    case IPP_JOB_PENDING:
        return 2;
        break;
    case IPP_JOB_HELD:
        return 1;
        break;
    default:
        return 0;
        break;
    }
}

#include "moc_JobSortFilterModel.cpp"
