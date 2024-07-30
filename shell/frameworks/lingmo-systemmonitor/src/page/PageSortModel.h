/*
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 * SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QAbstractProxyModel>
#include <qqmlregistration.h>

#include "PagesModel.h"

class PageSortModel : public QAbstractProxyModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        ShouldRemoveFilesRole = PagesModel::FilesWriteableRole + 1,
    };
    Q_ENUM(Roles)

    explicit PageSortModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void setSourceModel(QAbstractItemModel *newSourceModel) override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    Q_INVOKABLE void move(int fromRow, int toRow);

    Q_INVOKABLE void applyChangesToSourceModel() const;

private:
    QList<int> m_rowMapping;
    QList<bool> m_hiddenProxy;
    QList<bool> m_removeFiles;
};
