/*
 * SPDX-FileCopyrightText: 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "QuotaListModel.h"

#include <QDebug>

QuotaListModel::QuotaListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

namespace
{
/**
 * QML data roles.
 */
enum {
    DetailsRole = Qt::UserRole,
    IconRole,
    FreeStringRole,
    UsedStringRole,
    MountPointRole,
    UsageRole,
};
}

QHash<int, QByteArray> QuotaListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DetailsRole] = "details";
    roles[IconRole] = "icon";
    roles[FreeStringRole] = "free";
    roles[UsedStringRole] = "used";
    roles[MountPointRole] = "mountPoint";
    roles[UsageRole] = "usage";

    return roles;
}

QVariant QuotaListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size()) {
        return QVariant();
    }

    const auto item = m_items[index.row()];

    switch (role) {
    case DetailsRole:
        return item.mountString();
    case IconRole:
        return item.iconName();
    case FreeStringRole:
        return item.freeString();
    case UsedStringRole:
        return item.usedString();
    case MountPointRole:
        return item.mountPoint();
    case UsageRole:
        return item.usage();
    }

    return QVariant();
}

int QuotaListModel::rowCount(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return m_items.size();
    }

    return 0;
}

bool QuotaListModel::setData(const QModelIndex &index, const QVariant &variant, int role)
{
    Q_UNUSED(role)

    const int row = index.row();
    if (index.isValid() && row < m_items.size()) {
        const QuotaItem item = variant.value<QuotaItem>();

        // This assert makes sure that changing items modify the correct item:
        // therefore, the unique identifier 'mountPoint()' is used. If that
        // is not the case, the newly inserted row must have an empty mountPoint().
        Q_ASSERT(item.mountPoint() == m_items[row].mountPoint() || m_items[row].mountPoint().isEmpty());

        if (m_items[row] != item) {
            m_items[row] = item;
            Q_EMIT dataChanged(index, index);
            return true;
        }
    }

    return false;
}

bool QuotaListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    // only top-level items are supported
    if (parent.isValid()) {
        return false;
    }

    beginInsertRows(QModelIndex(), row, row + count - 1);
    m_items.insert(row, count, QuotaItem());
    endInsertRows();

    return true;
}

bool QuotaListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    // only top-level items are valid
    if (parent.isValid() || (row + count) >= m_items.size()) {
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    m_items.remove(row, count);
    endRemoveRows();

    return true;
}

void QuotaListModel::clear()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}

namespace
{
QStringList mountPoints(const QList<QuotaItem> &items)
{
    QStringList list;
    for (auto &item : items) {
        list.append(item.mountPoint());
    }
    return list;
}

int indexOfMountPoint(const QString &mountPoint, const QList<QuotaItem> &items)
{
    for (int i = 0; i < items.size(); ++i) {
        if (mountPoint == items[i].mountPoint()) {
            return i;
        }
    }
    return -1;
}
}

void QuotaListModel::updateItems(const QList<QuotaItem> &items)
{
    QStringList unusedMountPoints = mountPoints(m_items);

    // merge existing and new mount points
    for (auto &item : items) {
        // remove still used item from unused list
        unusedMountPoints.removeOne(item.mountPoint());

        // insert or modify m_items
        int row = indexOfMountPoint(item.mountPoint(), m_items);
        if (row < 0) {
            // new item: append on end
            row = m_items.size();
            insertRow(row);
        }
        setData(createIndex(row, 0), QVariant::fromValue(item));
    }

    // remove mount points, that do not exist anymore
    for (const auto &mountPoint : unusedMountPoints) {
        const int row = indexOfMountPoint(mountPoint, m_items);
        Q_ASSERT(row >= 0);
        removeRow(row);
    }
}
