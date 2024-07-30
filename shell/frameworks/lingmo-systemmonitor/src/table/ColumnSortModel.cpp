/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "ColumnSortModel.h"

#include <QDebug>

ColumnSortModel::ColumnSortModel(QObject *parent)
    : QIdentityProxyModel(parent)
{
}

QVariant ColumnSortModel::data(const QModelIndex &index, int role) const
{
    return QIdentityProxyModel::data(mapToSource(index), role);
}

void ColumnSortModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    m_rowMapping.clear();
    if (newSourceModel) {
        for (int i = 0; i < newSourceModel->rowCount(); ++i) {
            m_rowMapping.append(i);
        }
    }

    QIdentityProxyModel::setSourceModel(newSourceModel);
}

QModelIndex ColumnSortModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) {
        return QModelIndex();
    }

    return createIndex(m_rowMapping.indexOf(sourceIndex.row()), sourceIndex.column());
}

QModelIndex ColumnSortModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid()) {
        return QModelIndex();
    }

    return sourceModel()->index(m_rowMapping.at(proxyIndex.row()), proxyIndex.column());
}

void ColumnSortModel::move(int fromRow, int toRow)
{
    beginMoveRows(QModelIndex(), fromRow, fromRow, QModelIndex(), fromRow < toRow ? toRow + 1 : toRow);
    m_rowMapping.move(fromRow, toRow);
    endMoveRows();
    Q_EMIT sortedColumnsChanged();
}

QStringList ColumnSortModel::sortedColumns() const
{
    QStringList result;
    auto source = sourceModel();
    for (auto sourceRow : std::as_const(m_rowMapping)) {
        result.append(source->data(source->index(sourceRow, 0), idRoleNumber()).toString());
    }
    return result;
}

void ColumnSortModel::setSortedColumns(const QStringList &newSortedColumns)
{
    auto source = sourceModel();
    QHash<QString, int> sourceRowMapping;
    for (int i = 0; i < rowCount(); ++i) {
        auto id = source->data(source->index(i, 0), idRoleNumber()).toString();
        sourceRowMapping.insert(id, i);
    }

    beginResetModel();
    m_rowMapping.clear();
    for (const auto &id : newSortedColumns) {
        auto row = sourceRowMapping.value(id, -1);
        if (row != -1) {
            m_rowMapping.append(row);
        }
    }

    for (int i = 0; i < rowCount(); ++i) {
        if (!m_rowMapping.contains(i)) {
            m_rowMapping.append(i);
        }
    }
    endResetModel();
    Q_EMIT sortedColumnsChanged();
}

QString ColumnSortModel::idRole() const
{
    return m_idRole;
}

void ColumnSortModel::setIdRole(const QString &newIdRole)
{
    if (newIdRole == m_idRole) {
        return;
    }

    m_idRole = newIdRole;
    m_idRoleNumber = -1;
    Q_EMIT idRoleChanged();
}

int ColumnSortModel::idRoleNumber() const
{
    if (m_idRoleNumber == -1 && !m_idRole.isEmpty() && sourceModel()) {
        m_idRoleNumber = sourceModel()->roleNames().key(m_idRole.toUtf8());
    }

    return m_idRoleNumber;
}

#include "moc_ColumnSortModel.cpp"
