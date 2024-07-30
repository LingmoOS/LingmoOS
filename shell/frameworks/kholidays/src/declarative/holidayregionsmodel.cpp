/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2015 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "holidayregionsmodel.h"

#include "../holidayregion.h"

class HolidayRegionsDeclarativeModel::Private
{
public:
    QStringList regionCodes;
};

HolidayRegionsDeclarativeModel::HolidayRegionsDeclarativeModel(QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private())
{
    d->regionCodes = KHolidays::HolidayRegion::regionCodes();
    // Make sure we don't add the same regions twice.
    // This can happen if two copies of the data exist
    // in the system and both are read by KHolidays.
    d->regionCodes.removeDuplicates();
}

HolidayRegionsDeclarativeModel::~HolidayRegionsDeclarativeModel()
{
    delete d;
}

int HolidayRegionsDeclarativeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

int HolidayRegionsDeclarativeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->regionCodes.size();
}

QVariant HolidayRegionsDeclarativeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const QString regionCode = d->regionCodes.at(index.row());

    switch (index.column()) {
    case RegionColumn:
        switch (role) {
        case Qt::DisplayRole:
        case HolidayRegionsDeclarativeModel::RegionRole:
            return regionCode;
        case HolidayRegionsDeclarativeModel::NameRole:
            return KHolidays::HolidayRegion::name(regionCode);
        case HolidayRegionsDeclarativeModel::DescriptionRole:
            return KHolidays::HolidayRegion::description(regionCode);
        }
        break;
    case NameColumn:
        if (role == Qt::DisplayRole) {
            return KHolidays::HolidayRegion::name(regionCode);
        }
        break;
    case DescriptionColumn:
        if (role == Qt::DisplayRole) {
            return KHolidays::HolidayRegion::description(regionCode);
        }
        break;
    }

    return QVariant();
}

QVariant HolidayRegionsDeclarativeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case RegionColumn:
            return tr("Region");
        case NameColumn:
            return tr("Name");
        case DescriptionColumn:
            return tr("Description");
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QHash<int, QByteArray> HolidayRegionsDeclarativeModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractTableModel::roleNames();
    roles.insert(HolidayRegionsDeclarativeModel::RegionRole, QByteArrayLiteral("region"));
    roles.insert(HolidayRegionsDeclarativeModel::NameRole, QByteArrayLiteral("name"));
    roles.insert(HolidayRegionsDeclarativeModel::DescriptionRole, QByteArrayLiteral("description"));

    return roles;
}

#include "moc_holidayregionsmodel.cpp"
