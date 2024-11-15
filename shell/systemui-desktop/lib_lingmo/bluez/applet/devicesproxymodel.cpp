/*
    SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "devicesproxymodel.h"

#include <BluezQt/Adapter>
#include <BluezQt/Device>
#include <BluezQt/Manager>

DevicesProxyModel::DevicesProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);

    m_manager = new BluezQt::Manager(this);
    connect(m_manager, &BluezQt::Manager::bluetoothBlockedChanged, this, &DevicesProxyModel::bluetoothBlockedChanged);
}

void DevicesProxyModel::bluetoothBlockedChanged(bool blocked)
{
    if (blocked){
        m_connectedName = "";
        emit connectedNameChanged(m_connectedName);
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
        // if (index.data(BluezQt::DevicesModel::ConnectedRole).toBool()) {
        //     return QStringLiteral("Connected");
        // }
        // return QStringLiteral("Available");

        if (index.data(BluezQt::DevicesModel::PairedRole).toBool()) {
            return QStringLiteral("My devices");
        }
        return QStringLiteral("Other devices");

    case DeviceFullNameRole:
        if (duplicateIndexAddress(index)) {
            const QString &name = QSortFilterProxyModel::data(index, BluezQt::DevicesModel::NameRole).toString();
            const QString &ubi = QSortFilterProxyModel::data(index, BluezQt::DevicesModel::UbiRole).toString();
            const QString &hci = adapterHciString(ubi);

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
    bool leftPaired = left.data(BluezQt::DevicesModel::PairedRole).toBool();
    bool rightPaired = right.data(BluezQt::DevicesModel::PairedRole).toBool();

    if (leftPaired < rightPaired) {
        return true;
    } else if (leftPaired > rightPaired) {
        return false;
    }

    qint16 leftRssi = left.data(BluezQt::DevicesModel::RssiRole).toInt();
    qint16 rightRssi = right.data(BluezQt::DevicesModel::RssiRole).toInt();

    if (!leftPaired && leftRssi < rightRssi) {
        return true;
    } else if (!leftPaired && leftRssi > rightRssi) {
        return false;
    }

    const QString &leftName = left.data(BluezQt::DevicesModel::NameRole).toString();
    const QString &rightName = right.data(BluezQt::DevicesModel::NameRole).toString();

    return QString::localeAwareCompare(leftName, rightName) > 0;
}

// Returns "hciX" part from UBI "/org/bluez/hciX/dev_xx_xx_xx_xx_xx_xx"
QString DevicesProxyModel::adapterHciString(const QString &ubi) const
{
    int startIndex = ubi.indexOf(QLatin1String("/hci")) + 1;

    if (startIndex < 1) {
        return QString();
    }

    int endIndex = ubi.indexOf(QLatin1Char('/'), startIndex);

    if (endIndex == -1) {
        return ubi.mid(startIndex);
    }
    return ubi.mid(startIndex, endIndex - startIndex);
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

    if (index.data(BluezQt::DevicesModel::ConnectedRole).toBool() && index.data(BluezQt::DevicesModel::PairedRole).toBool()){
        m_connectedName = index.data(BluezQt::DevicesModel::NameRole).toString();
        m_connectedAdress = index.data(BluezQt::DevicesModel::AddressRole).toString();
        emit connectedNameChanged(m_connectedName);
        emit connectedAdressChanged(m_connectedAdress);
    }

    if (index.data(BluezQt::DevicesModel::TypeRole).toInt() == 18){
        return false;
    }

    if (index.data(BluezQt::DevicesModel::NameRole).toString().replace("-","") == 
        index.data(BluezQt::DevicesModel::AddressRole).toString().replace(":","")) {
        return false;
    }

    if (!index.data(BluezQt::DevicesModel::PairedRole).toBool() 
            && index.data(BluezQt::DevicesModel::RssiRole).toInt() == -32768) {
        return false;
    }

    return index.data(BluezQt::DevicesModel::AdapterPoweredRole).toBool() &&
           index.data(BluezQt::DevicesModel::AdapterPairableRole).toBool();
}
