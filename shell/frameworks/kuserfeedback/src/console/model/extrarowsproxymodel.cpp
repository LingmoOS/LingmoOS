/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "extrarowsproxymodel.h"

using namespace KUserFeedback::Console;

ExtraRowsProxyModel::ExtraRowsProxyModel(QObject* parent) :
    QIdentityProxyModel(parent)
{
}

ExtraRowsProxyModel::~ExtraRowsProxyModel() = default;

int ExtraRowsProxyModel::rowCount(const QModelIndex& parent) const
{
    if (!sourceModel())
        return 0;
    if (parent.isValid())
        return sourceModel()->rowCount(mapToSource(parent));
    return sourceModel()->rowCount() + m_extraRowCount;
}

QVariant ExtraRowsProxyModel::data(const QModelIndex& index, int role) const
{
    if (!sourceModel())
        return {};
    if (!index.isValid() || index.parent().isValid() || index.row() < sourceModel()->rowCount())
        return QIdentityProxyModel::data(index, role);
    return extraData(index.row() - sourceModel()->rowCount(), index.column(), role);
}

QModelIndex ExtraRowsProxyModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!sourceModel())
        return {};
    if (parent.isValid() || row < sourceModel()->rowCount())
        return QIdentityProxyModel::index(row, column, parent);
    if (row >= rowCount())
        return {};
    return createIndex(row, column);
}

QModelIndex ExtraRowsProxyModel::parent(const QModelIndex& child) const
{
    if (!sourceModel())
        return {};
    if (child.internalId() == 0 && child.row() >= sourceModel()->rowCount())
        return {};
    return QIdentityProxyModel::parent(child);
}


QVariant ExtraRowsProxyModel::extraData(int row, int column, int role) const
{
    // TODO split into separate model!
    Q_UNUSED(row);
    if (role == Qt::DisplayRole)
        return headerData(column, Qt::Horizontal, Qt::DisplayRole);
    return {};
}

#include "moc_extrarowsproxymodel.cpp"
