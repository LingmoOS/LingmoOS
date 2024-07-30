/*
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpluginproxymodel.h"
#include "kpluginmodel.h"

KPluginProxyModel::KPluginProxyModel(QObject *parent)
    : KCategorizedSortFilterProxyModel(parent)
{
    sort(0);
    setCategorizedModel(true);
}

KPluginProxyModel::~KPluginProxyModel() = default;

QString KPluginProxyModel::query() const
{
    return m_query;
}

void KPluginProxyModel::setQuery(const QString &query)
{
    if (m_query != query) {
        m_query = query;
        invalidate();
        Q_EMIT queryChanged();
    }
}

bool KPluginProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex & /*sourceParent*/) const
{
    if (m_query.isEmpty()) {
        return true;
    }

    const QModelIndex index = sourceModel()->index(sourceRow, 0);

    const QString name = index.data(KPluginModel::NameRole).toString();

    if (name.contains(m_query, Qt::CaseInsensitive)) {
        return true;
    }

    const QString description = index.data(KPluginModel::DescriptionRole).toString();

    if (description.contains(m_query, Qt::CaseInsensitive)) {
        return true;
    }

    return false;
}

bool KPluginProxyModel::subSortLessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.data(KPluginModel::SortableRole).toBool() && right.data(KPluginModel::SortableRole).toBool()) {
        return left.data(KPluginModel::NameRole).toString().compare(right.data(KPluginModel::NameRole).toString(), Qt::CaseInsensitive) < 0;
    }
    return 0;
}

int KPluginProxyModel::compareCategories(const QModelIndex &left, const QModelIndex &right) const
{
    const QStringList orderedCategoryLabels = m_model->getOrderedCategoryLabels();
    const QString leftLabel = left.data(KCategorizedSortFilterProxyModel::CategorySortRole).toString();
    const QString rightLabel = right.data(KCategorizedSortFilterProxyModel::CategorySortRole).toString();
    // Preserve the order in which they were passed in the model from consumers
    return orderedCategoryLabels.indexOf(leftLabel) - orderedCategoryLabels.indexOf(rightLabel);
}

#include "moc_kpluginproxymodel.cpp"
