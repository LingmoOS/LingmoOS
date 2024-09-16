// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "kextracolumnsproxymodel.h"
#include <QQmlEngine>

class ListToTableProxyModel : public KExtraColumnsProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QList<int> roles MEMBER m_roles NOTIFY rolesChanged)
    Q_PROPERTY(int sourceColumn MEMBER m_sourceColumn NOTIFY sourceColumnChanged)
public:
    explicit ListToTableProxyModel(QObject *parent = nullptr);

    QVariant extraColumnData(const QModelIndex &parent, int row, int extraColumn, int role = Qt::DisplayRole) const override;

signals:
    void rolesChanged(QList<int> roles);
    void sourceColumnChanged(int sourceColum);

private:
    QList<int> m_roles;
    int m_sourceColumn = 0;
};
