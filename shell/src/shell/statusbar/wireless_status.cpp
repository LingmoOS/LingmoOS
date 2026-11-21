// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/wireless_status.h"

#include "base/dbus.h"
#include "lingmo/networkmanager/client.h"
#include "lingmo/networkmanager/handler.h"
#include "lingmo/networkmanager/wifi_utils.h"

namespace Lingmo::Shell {

namespace dbus = base::dbus;
namespace nm = networkmanager;

WirelessStatus::WirelessStatus()
    : m_client(nm::Client::instance())
    , m_handler(std::make_unique<nm::Handler>())
{
    m_subscriptions.push_back(m_client->changed.connect([this] {
        updateActiveConnection();
        changed();
    }));
    m_subscriptions.push_back(m_client->readyChanged.connect([this] {
        updateActiveConnection();
        changed();
    }));
    m_subscriptions.push_back(m_handler->hotspotActiveChanged.connect([this] { changed(); }));
    updateActiveConnection();
}

WirelessStatus::~WirelessStatus()
{
    m_subscriptions.clear();
}

bool WirelessStatus::ready() const
{
    return m_client->ready();
}

StatusBarIndicators::Network WirelessStatus::network() const
{
    StatusBarIndicators::Network network;
    network.wirelessHwEnabled = ready() && m_client->manager("WirelessHardwareEnabled").value<bool>(false);
    network.wirelessEnabled = ready() && m_client->manager("WirelessEnabled").value<bool>(false);
    network.hotspotActive = m_handler->hotspotActive();
    network.wirelessName = m_wirelessName;
    network.wirelessIcon = m_wirelessIcon;
    return network;
}

// NM.ActiveConnection: the Wi-Fi connection that is up, the primary one if
// there are several; a hotspot is not a network this machine is on.
void WirelessStatus::updateActiveConnection()
{
    std::string name;
    std::string icon;
    if (ready()) {
        const auto primary = m_client->manager("PrimaryConnection").get<dbus::ObjectPath>();
        std::string chosen;
        std::string device;
        for (const std::string &active : m_client->activeConnections()) {
            const auto devices = m_client->property(active, nm::activeInterface, "Devices")
                                         .value<std::vector<dbus::ObjectPath>>();
            if (m_client->property(active, nm::activeInterface, "Type").value<std::string>() != "802-11-wireless"
                || m_client->property(active, nm::activeInterface, "State").value<uint32_t>()
                        != nm::ActiveActivated
                || devices.empty()
                || m_client->property(devices.front().path, nm::wirelessInterface, "Mode").value<uint32_t>()
                        == nm::ModeAp)
                continue;
            if (chosen.empty() || (primary && active == primary->path)) {
                chosen = active;
                device = devices.front().path;
            }
        }
        if (!chosen.empty()) {
            const auto accessPoint =
                    m_client->property(device, nm::wirelessInterface, "ActiveAccessPoint").get<dbus::ObjectPath>();
            const std::string path = accessPoint ? accessPoint->path : std::string();
            const auto bytes = m_client->property(path, nm::accessPointInterface, "Ssid")
                                       .value<std::vector<uint8_t>>();
            const std::string ssid(bytes.begin(), bytes.end());
            // A hidden network's access point has no name of its own; its profile does.
            name = nm::wifi::ssidHidden(ssid)
                    ? m_client->property(chosen, nm::activeInterface, "Id").value<std::string>()
                    : nm::wifi::ssidLabel(ssid);
            icon = nm::wifi::strengthIcon(
                    int(m_client->property(path, nm::accessPointInterface, "Strength").value<uint8_t>()));
        }
    }
    m_wirelessName = name;
    m_wirelessIcon = icon;
}

} // namespace Lingmo::Shell
