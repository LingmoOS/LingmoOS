/*
 * SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "ReverseColumnsProxyModel.h"

ReverseColumnsProxyModel::ReverseColumnsProxyModel(QObject *parent)
    : KRearrangeColumnsProxyModel(parent)
{
}

void ReverseColumnsProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    auto oldModel = this->sourceModel();
    if (oldModel == sourceModel) {
        return;
    }
    if (oldModel) {
        oldModel->disconnect(this);
    }
    KRearrangeColumnsProxyModel::setSourceModel(sourceModel);
    if (sourceModel) {
        connect(sourceModel, &QAbstractItemModel::columnsInserted, this, &ReverseColumnsProxyModel::reverseColumns);
        connect(sourceModel, &QAbstractItemModel::columnsRemoved, this, &ReverseColumnsProxyModel::reverseColumns);
        connect(sourceModel, &QAbstractItemModel::modelReset, this, &ReverseColumnsProxyModel::reverseColumns);
        reverseColumns();
    }
}
void ReverseColumnsProxyModel::reverseColumns()
{
    if (!sourceModel()) {
        return;
    }
    QList<int> columns(sourceModel()->columnCount());
    std::iota(columns.rbegin(), columns.rend(), 0);
    setSourceColumns(columns);
}
