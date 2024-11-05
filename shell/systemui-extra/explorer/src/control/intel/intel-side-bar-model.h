/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef INTEL_SIDEBARMODEL_H
#define INTEL_SIDEBARMODEL_H

#include <QAbstractItemModel>
#include <QVector>

#include "explorer-core_global.h"

namespace Peony {

namespace Intel {

class SideBarAbstractItem;

/*!
 * \brief The SideBarModel class
 * \todo
 * Add dnd support and custom favorite items support.
 */
class PEONYCORESHARED_EXPORT SideBarModel : public QAbstractItemModel
{
    friend class SideBarFileSystemItem;
    friend class SideBarAbstractItem;
    Q_OBJECT

public:
    explicit SideBarModel(QObject *parent = nullptr);
    ~SideBarModel() override;

    QModelIndex firstColumnIndex(SideBarAbstractItem *item);
    QModelIndex lastColumnIndex(SideBarAbstractItem *item);

    SideBarAbstractItem *itemFromIndex(const QModelIndex &index);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Fetch data dynamically:
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent) override;

    Qt::DropActions supportedDropActions() const override;

Q_SIGNALS:
    void indexUpdated(const QModelIndex &index);

protected:
    QVector<SideBarAbstractItem*> *m_root_children = nullptr;

    void onIndexUpdated(const QModelIndex &index);

protected:
    QStringList m_bookmark_uris;
};

}

}

#endif // SIDEBARMODEL_H
