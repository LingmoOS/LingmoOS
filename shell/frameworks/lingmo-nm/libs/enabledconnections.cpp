/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "enabledconnections.h"

EnabledConnections::EnabledConnections(QObject *parent)
    : QObject(parent)
    , m_networkingEnabled(NetworkManager::isNetworkingEnabled())
    , m_wirelessEnabled(NetworkManager::isWirelessEnabled())
    , m_wirelessHwEnabled(NetworkManager::isWirelessHardwareEnabled())
    , m_wwanEnabled(NetworkManager::isWwanEnabled())
    , m_wwanHwEnabled(NetworkManager::isWwanHardwareEnabled())
{
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::networkingEnabledChanged, this, &EnabledConnections::onNetworkingEnabled);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wirelessEnabledChanged, this, &EnabledConnections::onWirelessEnabled);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wirelessHardwareEnabledChanged, this, &EnabledConnections::onWirelessHwEnabled);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wwanEnabledChanged, this, &EnabledConnections::onWwanEnabled);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wwanHardwareEnabledChanged, this, &EnabledConnections::onWwanHwEnabled);
}

EnabledConnections::~EnabledConnections() = default;

bool EnabledConnections::isNetworkingEnabled() const
{
    return m_networkingEnabled;
}

bool EnabledConnections::isWirelessEnabled() const
{
    return m_wirelessEnabled;
}

bool EnabledConnections::isWirelessHwEnabled() const
{
    return m_wirelessHwEnabled;
}

bool EnabledConnections::isWwanEnabled() const
{
    return m_wwanEnabled;
}

bool EnabledConnections::isWwanHwEnabled() const
{
    return m_wwanHwEnabled;
}

void EnabledConnections::onNetworkingEnabled(bool enabled)
{
    m_networkingEnabled = enabled;
    Q_EMIT networkingEnabled(enabled);
}

void EnabledConnections::onWirelessEnabled(bool enabled)
{
    m_wirelessEnabled = enabled;
    Q_EMIT wirelessEnabled(enabled);
}

void EnabledConnections::onWirelessHwEnabled(bool enabled)
{
    m_wirelessHwEnabled = enabled;
    Q_EMIT wirelessHwEnabled(enabled);
}

void EnabledConnections::onWwanEnabled(bool enabled)
{
    m_wwanEnabled = enabled;
    Q_EMIT wwanEnabled(enabled);
}

void EnabledConnections::onWwanHwEnabled(bool enabled)
{
    m_wwanHwEnabled = enabled;
    Q_EMIT wwanHwEnabled(enabled);
}

#include "moc_enabledconnections.cpp"
