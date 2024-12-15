// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractProxyModel>
#include <QAbstractListModel>

typedef QModelIndex (*CombineFunc)(QVariant, QAbstractItemModel*);

class RoleCombineModel : public QAbstractProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel majorModel READ majorModel CONSTANT FINAL)
    Q_PROPERTY(QAbstractItemModel minorModel READ minorModel CONSTANT FINAL)

public:
    RoleCombineModel(QAbstractItemModel* major, QAbstractItemModel* minor, int majorRoles, CombineFunc func, QObject* parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool hasIndex(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE virtual QModelIndex parent(const QModelIndex &child) const override;

    QAbstractItemModel* majorModel() const;
    QAbstractItemModel* minorModel() const;

    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

private:
    QAbstractItemModel* m_minor;

    // Hash table used to map this QModelIndex row & column 2 origin QModelIndex row & column.
    QMap<QPair<int, int> ,QPair<int, int>> m_indexMap;
    // Hash table map role in this model to role in origin model.
    QHash<int, int> m_minorRolesMap;
};
