/*
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "devicesproxymodel.h"
#include "utils.h"

#include <BluezQt/Adapter>
#include <BluezQt/Device>

DevicesProxyModel::DevicesProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);
}

bool DevicesProxyModel::hideBlockedDevices() const
{
    return m_hideBlockedDevices;
}

void DevicesProxyModel::setHideBlockedDevices(bool shouldHide)
{
    if (m_hideBlockedDevices != shouldHide) {
        m_hideBlockedDevices = shouldHide;

        invalidateFilter();

        Q_EMIT hideBlockedDevicesChanged();
    }
}

QHash<int, QByteArray> DevicesProxyModel::roleNames() const
{
    QHash<int, QByteArray> roles = QSortFilterProxyModel::roleNames();
    roles[SectionRole] = QByteArrayLiteral("Section");
    roles[DeviceFullNameRole] = QByteArrayLiteral("DeviceFullName");
    return roles;
}

QVariant DevicesProxyModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case SectionRole:
        if (index.data(BluezQt::DevicesModel::BlockedRole).toBool()) {
            return QStringLiteral("Blocked");
        }
        if (index.data(BluezQt::DevicesModel::ConnectedRole).toBool()) {
            return QStringLiteral("Connected");
        }
        return QStringLiteral("Available");

    case DeviceFullNameRole:
        if (duplicateIndexAddress(index)) {
            const QString &name = QSortFilterProxyModel::data(index, BluezQt::DevicesModel::NameRole).toString();
            const QString &ubi = QSortFilterProxyModel::data(index, BluezQt::DevicesModel::UbiRole).toString();
            const QString &hci = Utils::adapterHciString(ubi);

            if (!hci.isEmpty()) {
                return QStringLiteral("%1 - %2").arg(name, hci);
            }
        }
        return QSortFilterProxyModel::data(index, BluezQt::DevicesModel::NameRole);

    default:
        return QSortFilterProxyModel::data(index, role);
    }
}

bool DevicesProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    bool leftBlocked = left.data(BluezQt::DevicesModel::BlockedRole).toBool();
    bool rightBlocked = right.data(BluezQt::DevicesModel::BlockedRole).toBool();

    // Blocked are checked first, but they go last.
    if (!leftBlocked && rightBlocked) {
        return false;
    } else if (leftBlocked && !rightBlocked) {
        return true;
    }

    bool leftConnected = left.data(BluezQt::DevicesModel::ConnectedRole).toBool();
    bool rightConnected = right.data(BluezQt::DevicesModel::ConnectedRole).toBool();

    // Conencted go above disconnected but available (not blocked)
    if (!leftConnected && rightConnected) {
        return true;
    } else if (leftConnected && !rightConnected) {
        return false;
    }

    const QString &leftName = left.data(BluezQt::DevicesModel::NameRole).toString();
    const QString &rightName = right.data(BluezQt::DevicesModel::NameRole).toString();

    return QString::localeAwareCompare(leftName, rightName) > 0;
}

bool DevicesProxyModel::duplicateIndexAddress(const QModelIndex &idx) const
{
    const QModelIndexList &list = match(index(0, 0), //
                                        BluezQt::DevicesModel::AddressRole,
                                        idx.data(BluezQt::DevicesModel::AddressRole).toString(),
                                        2,
                                        Qt::MatchExactly);
    return list.size() > 1;
}

bool DevicesProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    if (m_hideBlockedDevices && index.data(BluezQt::DevicesModel::BlockedRole).toBool()) {
        return false;
    }
    // Only show paired and connected devices in the KCM and applet
    return index.data(BluezQt::DevicesModel::PairedRole).toBool() || index.data(BluezQt::DevicesModel::ConnectedRole).toBool();
}

#include "moc_devicesproxymodel.cpp"
