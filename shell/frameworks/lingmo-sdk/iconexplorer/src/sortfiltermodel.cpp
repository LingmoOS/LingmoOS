/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sortfiltermodel.h"
#include "iconmodel.h"

using namespace CuttleFish;

SortFilterModel::SortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortRole(IconModel::IconName);
    setSortCaseSensitivity(Qt::CaseInsensitive);
}

bool SortFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!(m_category.isEmpty() || m_category == QLatin1String("all")) && (m_category != sourceIndex.data(IconModel::Category))) {
        return false;
    }
    if (!m_filter.isEmpty()) {
        return sourceIndex.data(IconModel::IconName).toString().contains(m_filter, Qt::CaseInsensitive);
    }
    return true;
}

int SortFilterModel::currentIndex()
{
    const QModelIndex index = mapFromSource(m_currentSourceIndex);
    if (index.isValid()) {
        return index.row();
    }
    m_currentSourceIndex = mapToSource(this->index(0, 0));
    return 0;
}

void SortFilterModel::setCurrentIndex(int index)
{
    if (mapFromSource(m_currentSourceIndex).row() != index) {
        m_currentSourceIndex = mapToSource(this->index(index, 0));
        Q_EMIT currentIndexChanged();
    }
}

QString SortFilterModel::category() const
{
    return m_category;
}

void SortFilterModel::setCategory(const QString &category)
{
    if (category == m_category) {
        return;
    }
    int oldIndex = currentIndex();
    m_category = category;
    invalidateFilter();
    Q_EMIT categoryChanged();
    if (currentIndex() != oldIndex) {
        Q_EMIT currentIndexChanged();
    }
}

QString SortFilterModel::filter() const
{
    return m_filter;
}

void SortFilterModel::setFilter(const QString &filter)
{
    if (filter == m_filter) {
        return;
    }
    int oldIndex = currentIndex();
    m_filter = filter;
    invalidateFilter();
    Q_EMIT filterChanged();
    if (currentIndex() != oldIndex) {
        Q_EMIT currentIndexChanged();
    }
}

#include "moc_sortfiltermodel.cpp"
