/*
    SPDX-FileCopyrightText: 2013-2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "appletproxymodel.h"
#include "networkmodel.h"
#include "networkmodelitem.h"
#include "uiutils.h"

AppletProxyModel::AppletProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setFilterRole(NetworkModel::ItemUniqueNameRole);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    sort(0, Qt::DescendingOrder);
}

AppletProxyModel::~AppletProxyModel() = default;

bool AppletProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    const QString filter = filterRegularExpression().pattern();

    // slaves are filtered-out when not searching for a connection (makes the state of search results clear)
    const bool isSlave = sourceModel()->data(index, NetworkModel::SlaveRole).toBool();
    if (isSlave && filter.isEmpty()) {
        return false;
    }

    const NetworkManager::ConnectionSettings::ConnectionType type =
        (NetworkManager::ConnectionSettings::ConnectionType)sourceModel()->data(index, NetworkModel::TypeRole).toUInt();
    if (!UiUtils::isConnectionTypeSupported(type)) {
        return false;
    }

    NetworkModelItem::ItemType itemType = (NetworkModelItem::ItemType)sourceModel()->data(index, NetworkModel::ItemTypeRole).toUInt();

    if (itemType != NetworkModelItem::AvailableConnection && itemType != NetworkModelItem::AvailableAccessPoint) {
        return false;
    }

    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

bool AppletProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const bool leftAvailable =
        (NetworkModelItem::ItemType)sourceModel()->data(left, NetworkModel::ItemTypeRole).toUInt() != NetworkModelItem::UnavailableConnection;
    const bool leftConnected = sourceModel()->data(left, NetworkModel::ConnectionStateRole).toUInt() == NetworkManager::ActiveConnection::Activated;
    const int leftConnectionState = sourceModel()->data(left, NetworkModel::ConnectionStateRole).toUInt();
    const QString leftName = sourceModel()->data(left, NetworkModel::NameRole).toString();
    const UiUtils::SortedConnectionType leftType =
        UiUtils::connectionTypeToSortedType((NetworkManager::ConnectionSettings::ConnectionType)sourceModel()->data(left, NetworkModel::TypeRole).toUInt());
    const QString leftUuid = sourceModel()->data(left, NetworkModel::UuidRole).toString();
    const int leftSignal = sourceModel()->data(left, NetworkModel::SignalRole).toInt();
    const QDateTime leftDate = sourceModel()->data(left, NetworkModel::TimeStampRole).toDateTime();

    const bool rightAvailable =
        (NetworkModelItem::ItemType)sourceModel()->data(right, NetworkModel::ItemTypeRole).toUInt() != NetworkModelItem::UnavailableConnection;
    const bool rightConnected = sourceModel()->data(right, NetworkModel::ConnectionStateRole).toUInt() == NetworkManager::ActiveConnection::Activated;
    const int rightConnectionState = sourceModel()->data(right, NetworkModel::ConnectionStateRole).toUInt();
    const QString rightName = sourceModel()->data(right, NetworkModel::NameRole).toString();
    const UiUtils::SortedConnectionType rightType =
        UiUtils::connectionTypeToSortedType((NetworkManager::ConnectionSettings::ConnectionType)sourceModel()->data(right, NetworkModel::TypeRole).toUInt());
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

#include "moc_appletproxymodel.cpp"
