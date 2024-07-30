/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2015 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef HOLIDAYREGIONSMODEL_H
#define HOLIDAYREGIONSMODEL_H

#include <QAbstractTableModel>
#include <QObject>

class HolidayRegionsDeclarativeModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Roles {
        RegionRole = Qt::UserRole + 1,
        NameRole,
        DescriptionRole,
    };
    enum Columns { RegionColumn, NameColumn, DescriptionColumn };
    explicit HolidayRegionsDeclarativeModel(QObject *parent = nullptr);
    ~HolidayRegionsDeclarativeModel() override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    class Private;
    Private *const d;
};

#endif
