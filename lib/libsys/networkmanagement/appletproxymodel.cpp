/*
    Copyright 2013-2014 Jan Grulich <jgrulich@redhat.com>
    Copyright 2021 Reven Martin <aj@lingmoos.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "appletproxymodel.h"
#include "networkmodel.h"
#include "uiutils.h"

static NetworkManager::ConnectionSettings::ConnectionType convertType(AppletProxyModel::Type type)
{
    switch (type) {
    case AppletProxyModel::UnknownType:
        return NetworkManager::ConnectionSettings::ConnectionType::Unknown;
    case AppletProxyModel::WiredType:
        return NetworkManager::ConnectionSettings::ConnectionType::Wired;
    case AppletProxyModel::WirelessType:
        return NetworkManager::ConnectionSettings::ConnectionType::Wireless;
    default:
        break;
    }

    return NetworkManager::ConnectionSettings::ConnectionType::Unknown;
}

AppletProxyModel::AppletProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    sort(0, Qt::DescendingOrder);
}

AppletProxyModel::~AppletProxyModel()
{
}

AppletProxyModel::Type AppletProxyModel::type() const
{
    return m_type;
}

void AppletProxyModel::setType(Type type)
{
    if (m_type == type)
        return;
    m_type = type;
    Q_EMIT typeChanged();

    if (type == UnknownType)
        setFilterRole(0);
    else
        setFilterRole(NetworkModel::TypeRole);
}

bool AppletProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    // slaves are filtered-out when not searching for a connection (makes the state of search results clear)
    const bool isSlave = sourceModel()->data(index, NetworkModel::SlaveRole).toBool();
    if (isSlave && filterRegExp().isEmpty()) {
        return false;
    }

    const NetworkManager::ConnectionSettings::ConnectionType type = (NetworkManager::ConnectionSettings::ConnectionType) sourceModel()->data(index, NetworkModel::TypeRole).toUInt();
    if (!UiUtils::isConnectionTypeSupported(type)) {
        return false;
    }

    // Type Filter
    if (m_type == UnknownType || type != convertType(m_type))
        return false;

    NetworkModelItem::ItemType itemType = (NetworkModelItem::ItemType)sourceModel()->data(index, NetworkModel::ItemTypeRole).toUInt();

    if (itemType != NetworkModelItem::AvailableConnection &&
        itemType != NetworkModelItem::AvailableAccessPoint) {
        return false;
    }

    if (filterRegExp().isEmpty()) {
        return true;
    }

    return sourceModel()->data(index, NetworkModel::ItemUniqueNameRole).toString().contains(filterRegExp());
}

bool AppletProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const bool leftAvailable = (NetworkModelItem::ItemType)sourceModel()->data(left, NetworkModel::ItemTypeRole).toUInt() != NetworkModelItem::UnavailableConnection;
    const bool leftConnected = sourceModel()->data(left, NetworkModel::ConnectionStateRole).toUInt() == NetworkManager::ActiveConnection::Activated;
    const int leftConnectionState = sourceModel()->data(left, NetworkModel::ConnectionStateRole).toUInt();
    const QString leftName = sourceModel()->data(left, NetworkModel::NameRole).toString();
    const UiUtils::SortedConnectionType leftType = UiUtils::connectionTypeToSortedType((NetworkManager::ConnectionSettings::ConnectionType) sourceModel()->data(left, NetworkModel::TypeRole).toUInt());
    const QString leftUuid = sourceModel()->data(left, NetworkModel::UuidRole).toString();
    const int leftSignal = sourceModel()->data(left, NetworkModel::SignalRole).toInt();
    const QDateTime leftDate = sourceModel()->data(left, NetworkModel::TimeStampRole).toDateTime();

    const bool rightAvailable = (NetworkModelItem::ItemType)sourceModel()->data(right, NetworkModel::ItemTypeRole).toUInt() != NetworkModelItem::UnavailableConnection;
    const bool rightConnected = sourceModel()->data(right, NetworkModel::ConnectionStateRole).toUInt() == NetworkManager::ActiveConnection::Activated;
    const int rightConnectionState = sourceModel()->data(right, NetworkModel::ConnectionStateRole).toUInt();
    const QString rightName = sourceModel()->data(right, NetworkModel::NameRole).toString();
    const UiUtils::SortedConnectionType rightType = UiUtils::connectionTypeToSortedType((NetworkManager::ConnectionSettings::ConnectionType) sourceModel()->data(right, NetworkModel::TypeRole).toUInt());
    const QString rightUuid = sourceModel()->data(right, NetworkModel::UuidRole).toString();
    const int rightSignal = sourceModel()->data(right, NetworkModel::SignalRole).toInt();
    const QDateTime rightDate = sourceModel()->data(right, NetworkModel::TimeStampRole).toDateTime();

    if (leftAvailable < rightAvailable) {
        return true;
    } else if (leftAvailable > rightAvailable) {
        return false;
    }

    if (leftConnected < rightConnected) {
        return true;
    } else if (leftConnected > rightConnected) {
        return false;
    }

    if (leftConnectionState > rightConnectionState) {
        return true;
    } else if (leftConnectionState < rightConnectionState) {
        return false;
    }

    if (leftUuid.isEmpty() && !rightUuid.isEmpty()) {
        return true;
    } else if (!leftUuid.isEmpty() && rightUuid.isEmpty()) {
        return false;
    }

    if (leftType < rightType) {
        return false;
    } else if (leftType > rightType) {
        return true;
    }

    if (leftDate > rightDate) {
        return false;
    } else if (leftDate < rightDate) {
        return true;
    }

    if (leftSignal < rightSignal) {
        return true;
    } else if (leftSignal > rightSignal) {
        return false;
    }

    if (QString::localeAwareCompare(leftName, rightName) > 0) {
        return true;
    } else {
        return false;
    }
}
