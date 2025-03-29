/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLACESMODEL_H
#define PLACESMODEL_H

#include <QAbstractItemModel>
#include <Solid/Predicate>
#include "placesitem.h"

class PlacesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum DataRole {
        NameRole = Qt::UserRole + 1,
        IconNameRole,
        IconPathRole,
        UrlRole,
        PathRole,
        IsDeviceRole,
        IsOpticalDisc,
        setupNeededRole,
        CategoryRole
    };
    Q_ENUMS(DataRole);

    explicit PlacesModel(QObject *parent = nullptr);
    ~PlacesModel() override;

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    Q_INVOKABLE QVariantMap get(const int &index) const;
    Q_INVOKABLE void requestSetup(const int &index);
    Q_INVOKABLE void requestEject(const int &index);
    Q_INVOKABLE void requestTeardown(const int &index);

signals:
    void deviceSetupDone(const QString &filePath);

private slots:
    void onDeviceAdded(const QString &udi);
    void onDeviceRemoved(const QString &udi);
    void onItemChanged(PlacesItem *);

private:
    QList<PlacesItem *> m_items;
    Solid::Predicate m_predicate;
};

#endif
