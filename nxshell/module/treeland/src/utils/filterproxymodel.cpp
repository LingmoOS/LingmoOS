// Copyright (C) 2024 Yicheng Zhong <zhongyicheng@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "filterproxymodel.h"

#include <QQmlEngine>

FilterProxyModel::FilterProxyModel(QObject *parent)
    : QSortFilterProxyModel{ parent }
{
    initConnections();
    connect(this, &QSortFilterProxyModel::modelReset, this, &FilterProxyModel::initConnections);
    connect(this,
            &QSortFilterProxyModel::sourceModelChanged,
            this,
            &FilterProxyModel::initConnections);
}

void FilterProxyModel::initConnections()
{
    connect(this, &QSortFilterProxyModel::layoutChanged, this, &FilterProxyModel::countChanged);
    connect(this, &QSortFilterProxyModel::rowsInserted, this, &FilterProxyModel::countChanged);
    connect(this, &QSortFilterProxyModel::rowsRemoved, this, &FilterProxyModel::countChanged);
}

bool FilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);
    auto rowData = QVariantMap();
    for (auto role : roleNames().asKeyValueRange())
        rowData.insert(role.second,
                       sourceModel()->data(sourceModel()->index(source_row, 0), role.first));
    auto jsData = qjsEngine(this)->toScriptValue(rowData);
    return m_filterAcceptsRow.call(QJSValueList({ jsData })).toBool();
}

void FilterProxyModel::setFilterAcceptsRow(const QJSValue &val)
{
    if (val.equals(m_filterAcceptsRow))
        return;
    m_filterAcceptsRow = val;
    emit filterAcceptsRowChanged();
}

QVariantMap FilterProxyModel::get(const int index) const
{
    auto rowData = QVariantMap();
    for (auto role : roleNames().asKeyValueRange())
        rowData.insert(role.second, data(this->index(index, 0), role.first));
    return rowData;
}

void FilterProxyModel::invalidate()
{
    invalidateFilter();
}

int FilterProxyModel::count() const
{
    return rowCount();
}