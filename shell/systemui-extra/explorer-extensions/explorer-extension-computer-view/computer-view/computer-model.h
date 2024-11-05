/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#ifndef COMPUTERMODEL_H
#define COMPUTERMODEL_H

#include <QAbstractItemModel>

#include <QMap>

class AbstractComputerItem;

class ComputerModel : public QAbstractItemModel
{
    friend class ComputerNetworkItem;
    friend class ComputerVolumeItem;
    friend class ComputerRemoteVolumeItem;
    Q_OBJECT

public:
    explicit ComputerModel(QObject *parent = nullptr);

    void beginInsertItem(const QModelIndex &parent, int row);
    void endInsterItem();

    void beginRemoveItem(const QModelIndex &parent, int row);
    void endRemoveItem();

    QModelIndex createItemIndex(int row, QObject *item);

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    QString tryGetVolumeUriFromMountRoot(const QString &mountRootUri);
    QString tryGetVolumeRealUriFromUri(const QString &uri);
    void addRealUri(const QString &realUri);
    void removeRealUri(const QString &realUri);

    void refresh();

Q_SIGNALS:
    void updateLocationRequest(const QString &uri);
    void invalidateRequest();
    void updateRequest();

private:
    AbstractComputerItem *m_parentNode;

    QMap<QString, QString> m_volumeTargetMap;
    QList<QString> m_volumeRealUri;
};

#endif // COMPUTERMODEL_H
