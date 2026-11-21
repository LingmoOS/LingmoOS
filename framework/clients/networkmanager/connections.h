// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "lingmo/networkmanager/types.h"

namespace Lingmo::Networkmanager {

class Client;

// The switches of NetworkManager and its devices.
class EnabledConnections
{
public:
    EnabledConnections();
    ~EnabledConnections();

    EnabledConnections(const EnabledConnections&) = delete;
    EnabledConnections& operator=(const EnabledConnections&) = delete;

    // NetworkManager has been read; until then every switch reads off.
    bool isReady() const { return m_ready; }
    bool isNetworkingEnabled() const { return m_networkingEnabled; }
    bool isWirelessEnabled() const { return m_wirelessEnabled; }
    bool isWirelessHwEnabled() const { return m_wirelessHwEnabled; }
    bool isWiredAvailable() const { return m_wiredAvailable; }
    bool isWiredEnabled() const { return m_wiredEnabled; }

    // After any of the above changed.
    base::Signal<> changed;

private:
    void update();

    Client* m_client;
    base::Subscription m_subscription;
    bool m_ready = false;
    bool m_networkingEnabled = false;
    bool m_wirelessEnabled = false;
    bool m_wirelessHwEnabled = false;
    bool m_wiredAvailable = false;
    bool m_wiredEnabled = false;
};

// One saved profile's name and switches, written back to NetworkManager as
// they change.
class ConnectionSettings
{
public:
    explicit ConnectionSettings(std::string path = {});
    ~ConnectionSettings();

    ConnectionSettings(const ConnectionSettings&) = delete;
    ConnectionSettings& operator=(const ConnectionSettings&) = delete;

    const std::string& path() const { return m_path; }
    void setPath(const std::string& path);

    const std::string& name() const { return m_name; }
    void setName(const std::string& name);
    bool autoConnect() const { return m_autoConnect; }
    void setAutoConnect(bool value);
    bool availableToOtherUsers() const { return m_availableToOtherUsers; }
    void setAvailableToOtherUsers(bool value);

    base::Signal<> changed;

private:
    void update(const std::string& key, const base::dbus::Variant& value);
    // Values written but not yet read back; the stale cache must not undo them.
    base::dbus::Variant cached(const std::string& key, const base::dbus::Variant& defaultValue = {});
    void refresh();

    Client* m_client;
    CallContext m_context;
    base::Subscription m_subscription;
    std::string m_path;
    base::dbus::VariantMap m_pending;
    std::string m_name;
    bool m_autoConnect = true;
    bool m_availableToOtherUsers = false;
};

} // namespace Lingmo::Networkmanager
