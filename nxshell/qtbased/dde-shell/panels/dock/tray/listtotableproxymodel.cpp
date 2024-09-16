// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "listtotableproxymodel.h"

ListToTableProxyModel::ListToTableProxyModel(QObject *parent)
    : KExtraColumnsProxyModel(parent)
{
    connect(this, &ListToTableProxyModel::rolesChanged, this, [this](){
        for (int role : std::as_const(m_roles)) {
            QByteArray fallbackName(QByteArray::number(role));
            QByteArray roleName(sourceModel() ? sourceModel()->roleNames().value(role, fallbackName) : fallbackName);
            appendColumn(QString(roleName));
        }
    });

    connect(this, &ListToTableProxyModel::sourceModelChanged, this, [this](){
        for (int idx = 0; idx < m_roles.count(); idx++) {
            int role = m_roles[idx];
            QByteArray fallbackName(QByteArray::number(role));
            QByteArray roleName(sourceModel() ? sourceModel()->roleNames().value(role, fallbackName) : fallbackName);
            setExtraColumnTitle(idx, QString(roleName));
        }
    });

    connect(this, &ListToTableProxyModel::dataChanged, this,
            [this](const QModelIndex &topLeft, const QModelIndex &bottomRight,
                   const QList<int> &roles = QList<int>()){
        // TODO: lazy solution, we should use dataChanged() to notify extra column changed;
        beginResetModel();
        endResetModel();
    });
}

QVariant ListToTableProxyModel::extraColumnData(const QModelIndex &parent, int row, int extraColumn, int role) const
{
    QVariant result(data(index(row, m_sourceColumn, parent), m_roles[extraColumn]));
    if (!result.isValid()) return QStringLiteral("<invalid>");
    return data(index(row, m_sourceColumn, parent), m_roles[extraColumn]);
}
