/*
    SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "devicesstateproxymodel.h"

#include <BluezQt/DevicesModel>

#include <KLocalizedString>
#include <KNotification>

#include <algorithm>

DevicesStateProxyModel::DevicesStateProxyModel(QObject *parent)
    : QIdentityProxyModel(parent)
{
    connect(this, &QAbstractItemModel::rowsAboutToBeRemoved, this, &DevicesStateProxyModel::handleRowsAboutToBeRemoved);
}

bool DeviceState::isConnecting() const
{
    return !pendingCalls.isEmpty();
}

QHash<int, QByteArray> DevicesStateProxyModel::roleNames() const
{
    auto roleNames = QIdentityProxyModel::roleNames();
    roleNames.insert(ConnectingRole, "Connecting");
    roleNames.insert(ConnectionFailedRole, "ConnectionFailed");
    return roleNames;
}

QVariant DevicesStateProxyModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(index.isValid() ? index.model() == this : true);
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    switch (role) {
    case ConnectingRole:
    case ConnectionFailedRole: {
        // Don't fetch state for other roles that don't need it, such as
        // UbiRole which has to be fetched from state(), thus avoiding an
        // infinite recursion.
        const auto &state = this->state(index);

        switch (role) {
        case ConnectingRole:
            return state.isConnecting();
        case ConnectionFailedRole:
            return state.connectionFailed;
        default:
            Q_UNREACHABLE();
        }
    }
    default:
        return QIdentityProxyModel::data(index, role);
    }
}

bool DevicesStateProxyModel::isConnecting() const
{
    return std::ranges::any_of(m_state, [](const DeviceState &state) {
        return state.isConnecting();
    });
}

void DevicesStateProxyModel::registerPendingCallForDeviceUbi(BluezQt::PendingCall *call, const QString &ubi)
{
    const auto index = indexByUbi(ubi);
    if (call == nullptr || !index.isValid()) {
        return;
    }

    connect(call, &BluezQt::PendingCall::finished, this, &DevicesStateProxyModel::handlePendingCallFinished);

    auto &state = this->state(index);
    const auto wasConnectingGlobal = isConnecting();
    const auto wasConnecting = state.isConnecting();
    const auto wasFailed = state.connectionFailed;

    state.pendingCalls.insert(call);
    state.connectionFailed = false;

    QList<int> roles;

    if (wasFailed != state.connectionFailed) {
        roles.append(ConnectionFailedRole);
    }
    if (wasConnecting != state.isConnecting()) {
        roles.append(ConnectingRole);
    }
    if (!roles.isEmpty()) {
        Q_EMIT dataChanged(index, index, roles);
    }

    if (wasConnectingGlobal != isConnecting()) {
        Q_EMIT connectingChanged();
    }
}

void DevicesStateProxyModel::handlePendingCallFinished(BluezQt::PendingCall *call)
{
    const auto wasConnectingGlobal = isConnecting();

    const auto index = unregisterPendingCall(call);

    if (wasConnectingGlobal != isConnecting()) {
        Q_EMIT connectingChanged();
    }

    if (!index.isValid()) {
        return;
    }

    notifyIfConnectionFailed(call, index);
}

void DevicesStateProxyModel::handleRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    Q_ASSERT(parent.isValid() ? parent.model() == this : true);
    const auto wasConnectingGlobal = isConnecting();

    for (int i = first; i <= last; i++) {
        const auto index = this->index(i, 0, parent);
        const auto ubi = index.data(BluezQt::DevicesModel::UbiRole).toString();
        // Let's hope nothing inserts it back until rowsRemoved is called to confirm the transaction
        const auto state = m_state.take(ubi);
        for (const auto call : std::as_const(state.pendingCalls)) {
            disconnect(call, &BluezQt::PendingCall::finished, this, &DevicesStateProxyModel::handlePendingCallFinished);
        }
    }

    if (wasConnectingGlobal != isConnecting()) {
        Q_EMIT connectingChanged();
    }
}

QString DevicesStateProxyModel::ubi(const QModelIndex &index) const
{
    Q_ASSERT(index.isValid() ? index.model() == this : true);
    return index.data(BluezQt::DevicesModel::UbiRole).toString();
}

DeviceState &DevicesStateProxyModel::state(const QModelIndex &index) const
{
    Q_ASSERT(index.isValid() ? index.model() == this : true);
    return m_state[ubi(index)];
}

QModelIndex DevicesStateProxyModel::unregisterPendingCall(BluezQt::PendingCall *call)
{
    for (const auto &[ubi, state] : m_state.asKeyValueRange()) {
        const auto wasConnecting = state.isConnecting();
        const auto wasFailed = state.connectionFailed;

        const auto removed = state.pendingCalls.remove(call);
        if (removed) {
            QList<int> roles;

            // Find out which model index this UBI delongs to
            const auto index = indexByUbi(ubi);
            if (!index.isValid()) {
                return QModelIndex();
            }

            state.connectionFailed = call->error() != BluezQt::PendingCall::NoError;
            if (wasFailed != state.connectionFailed) {
                roles.append(ConnectionFailedRole);
            }
            if (wasConnecting != state.isConnecting()) {
                roles.append(ConnectingRole);
            }
            if (!roles.isEmpty()) {
                Q_EMIT dataChanged(index, index, roles);
            }
            return index;
        }
    }
    return QModelIndex();
}

QModelIndex DevicesStateProxyModel::indexByUbi(const QString &ubi)
{
    const auto count = rowCount();
    for (int i = 0; i < count; i++) {
        const auto index = this->index(i, 0);
        if (ubi == index.data(BluezQt::DevicesModel::UbiRole).toString()) {
            return index;
        }
    }
    return QModelIndex();
}

bool DevicesStateProxyModel::isDeviceAtIndexConnecting(const QModelIndex &index) const
{
    Q_ASSERT(index.isValid() ? index.model() == this : true);
    if (!index.isValid() || index.row() >= rowCount()) {
        return false;
    }

    const auto &state = this->state(index);
    return !state.pendingCalls.isEmpty();
}

void DevicesStateProxyModel::notifyIfConnectionFailed(const BluezQt::PendingCall *call, const QModelIndex &index)
{
    Q_ASSERT(index.isValid() ? index.model() == this : true);
    if (call->error() == BluezQt::PendingCall::NoError) {
        return;
    }

    const auto name = index.data(BluezQt::DevicesModel::NameRole).toString();
    const auto address = index.data(BluezQt::DevicesModel::AddressRole).toString();

    const auto title = i18nc("@label %1 is human-readable device name, %2 is low-level device address", "%1 (%2)", name, address);
    const auto text = errorText(call);

    KNotification *notification = new KNotification(QStringLiteral("ConnectionFailed"), KNotification::CloseOnTimeout, this);
    notification->setComponentName(QStringLiteral("bluedevil"));
    notification->setTitle(title);
    notification->setText(text);
    notification->sendEvent();
}

QString DevicesStateProxyModel::errorText(const BluezQt::PendingCall *call)
{
    switch (call->error()) {
    case BluezQt::PendingCall::Failed:
        return call->errorText() == QStringLiteral("Host is down")
            ? i18nc("Notification when the connection failed due to Failed:HostIsDown", "The device is unreachable")
            : i18nc("Notification when the connection failed due to Failed", "Connection to the device failed");

    case BluezQt::PendingCall::NotReady:
        return i18nc("Notification when the connection failed due to NotReady", "The device is not ready");

    default:
        return QString();
    }
}

#include "moc_devicesstateproxymodel.cpp"
