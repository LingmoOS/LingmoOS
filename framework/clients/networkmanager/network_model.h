// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "base/signal.h"
#include "lingmo/networkmanager/types.h"

namespace Lingmo::Networkmanager {

class Client;

// One row of the network lists.
struct NetworkItem
{
    // Stays the same while the row's access points come and go.
    std::string key;
    ConnectionType type = ConnectionType::Unknown;
    std::string name;
    // The name with its device added when two rows share it.
    std::string uniqueName;
    std::string ssid;
    std::string uuid;
    // The saved profile; empty for a Wi-Fi network never joined.
    std::string connectionPath;
    std::string devicePath;
    std::string deviceName;
    uint32_t deviceState = 0;
    // The access point to join.
    std::string specificPath;
    ConnectionStatus connectionState = ConnectionStatus::Deactivated;
    SecurityType securityType = SecurityType::Unknown;
    int signal = 0;
    // When the profile was last used, in seconds since the epoch; 0 for never.
    int64_t timestamp = 0;
    std::string icon;
    std::string ipV4Address;
    std::string ipV6Address;
    std::string gateway;
    std::string nameServer;
    std::string macAddress;
    std::string vpnType;
    // Why the VPN can't be used, or why its last attempt failed.
    std::string statusMessage;
    bool canActivate = true;
    bool vpnUsesUserName = false;
    std::string vpnUserName;
    // How the list names the security.
    std::string securityTypeString;

    bool operator==(const NetworkItem&) const = default;
};

// The wired profiles and Wi-Fi networks of every managed device, and the
// VPNs. Rows are worked out again from the client's cache after each change;
// a row keeps its place while it lasts and new ones come last.
class NetworkModel
{
public:
    NetworkModel();
    ~NetworkModel();

    NetworkModel(const NetworkModel&) = delete;
    NetworkModel& operator=(const NetworkModel&) = delete;

    const std::vector<NetworkItem>& items() const { return m_items; }

    // After the rows changed.
    base::Signal<> changed;

private:
    void rebuild();
    void addWired(const std::string& device, std::vector<NetworkItem>& items) const;
    void addWireless(const std::string& device, std::vector<NetworkItem>& items) const;
    void addVpns(std::vector<NetworkItem>& items) const;
    void fillDetails(NetworkItem& item) const;

    Client* m_client;
    std::vector<NetworkItem> m_items;
    // The last failure of each VPN, until it is tried again.
    std::map<std::string, std::string> m_vpnFailures;
    std::vector<base::Subscription> m_subscriptions;
};

// One type of row, sorted for a list: the connected network first, then by
// state, saved before others, the last used first, the strongest signal
// first, then by name. VPNs keep their place by name.
std::vector<NetworkItem> listItems(const std::vector<NetworkItem>& items, ConnectionType type);

// Locale-aware, as names are listed.
int compareNames(const std::string& a, const std::string& b);

} // namespace Lingmo::Networkmanager
