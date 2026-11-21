// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/network/common/networklist.h"

#include <ctime>

#include "window/options.h"

using namespace Lingmo::Networkmanager;

namespace settings::network {

NetworkList::NetworkList()
{
    if (options().mockNetwork) {
        m_mock = mockNetworks();
        return;
    }
    m_model = std::make_unique<NetworkModel>();
    m_subscription = m_model->changed.connect([this] { changed.emit(); });
}

NetworkList::~NetworkList() = default;

const std::vector<NetworkItem>& NetworkList::items() const
{
    return m_model ? m_model->items() : m_mock;
}

std::vector<NetworkItem> NetworkList::rows(ConnectionType type) const
{
    return listItems(items(), type);
}

std::optional<NetworkItem> NetworkList::find(const std::string& connectionPath, const std::string& ssid) const
{
    for (const NetworkItem& item : items()) {
        if (!connectionPath.empty() ? item.connectionPath == connectionPath
                                    : !ssid.empty() && item.connectionPath.empty() && item.ssid == ssid)
            return item;
    }
    return std::nullopt;
}

std::vector<NetworkItem> mockNetworks()
{
    const int64_t now = int64_t(std::time(nullptr));
    const auto wireless = [](std::string name, ConnectionStatus state, SecurityType security, std::string securityLabel,
                             int signal, std::string icon) {
        NetworkItem item;
        item.type = ConnectionType::Wireless;
        item.name = std::move(name);
        item.connectionState = state;
        item.securityType = security;
        item.securityTypeString = std::move(securityLabel);
        item.signal = signal;
        item.icon = std::move(icon);
        return item;
    };

    std::vector<NetworkItem> items;
    NetworkItem studio = wireless("Lingmo Studio", ConnectionStatus::Activated, SecurityType::Wpa2Psk,
                                  "WPA2 Personal", 92, "network-wireless-connected-100");
    studio.uuid = "mock-wifi-connected";
    studio.ipV4Address = "192.168.1.86";
    studio.ipV6Address = "fe80::1c2b:3aff:fe4d:5e6f";
    studio.gateway = "192.168.1.1";
    studio.nameServer = "192.168.1.1";
    studio.macAddress = "80:30:49:12:34:56";
    studio.timestamp = now;
    items.push_back(studio);

    NetworkItem coffee = wireless("Coffee Shop", ConnectionStatus::Deactivated, SecurityType::Wpa2Psk,
                                  "WPA2 Personal", 74, "network-wireless-connected-100");
    coffee.uuid = "mock-wifi-saved";
    coffee.timestamp = now - 3600;
    items.push_back(coffee);

    NetworkItem guest = wireless("Guest Network", ConnectionStatus::Deactivated, SecurityType::None, "None", 51,
                                 "network-wireless-connected-66");
    guest.timestamp = now - 7200;
    items.push_back(guest);

    NetworkItem neighbor = wireless("Neighbor Wi-Fi", ConnectionStatus::Deactivated, SecurityType::Sae, "WPA3 Personal",
                                    28, "network-wireless-connected-33");
    neighbor.timestamp = now - 10800;
    items.push_back(neighbor);

    const auto wired = [](std::string name, ConnectionStatus state, std::string uuid, int64_t timestamp) {
        NetworkItem item;
        item.type = ConnectionType::Wired;
        item.name = std::move(name);
        item.connectionState = state;
        item.securityType = SecurityType::None;
        item.signal = 100;
        item.icon = "network-wired";
        item.uuid = std::move(uuid);
        item.timestamp = timestamp;
        return item;
    };
    items.push_back(wired("Wired connection 1", ConnectionStatus::Activated, "mock-wired-connected", now));
    items.push_back(wired("USB Ethernet", ConnectionStatus::Deactivated, "mock-wired-usb", now - 18000));

    for (size_t i = 0; i < items.size(); ++i) {
        NetworkItem& item = items[i];
        item.key = "mock|" + std::to_string(i);
        item.uniqueName = item.name;
        item.ssid = item.name;
        // Saved networks need a path: the details dialog keys Auto-Join and Forget on it.
        item.connectionPath = item.uuid.empty() ? std::string() : "/mock/connection/" + std::to_string(i);
        item.devicePath = "/mock/device/" + std::to_string(i);
        item.specificPath = "/mock/network/" + std::to_string(i);
    }
    return items;
}

} // namespace settings::network
