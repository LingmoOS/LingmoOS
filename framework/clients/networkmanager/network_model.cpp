// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/networkmanager/network_model.h"
#include "lingmo/networkmanager/client.h"
#include "lingmo/networkmanager/inet.h"
#include "lingmo/networkmanager/text.h"
#include "lingmo/networkmanager/variants.h"
#include "lingmo/networkmanager/vpn_utils.h"
#include "lingmo/networkmanager/wifi_utils.h"

#include <algorithm>
#include <cstdio>
#include <endian.h>
#include <iterator>
#include <memory>
#include <tuple>

#include "base/locale.h"
#include "framework_i18n.h"

namespace Lingmo::Networkmanager {

using base::dbus::Variant;
using base::dbus::VariantMap;
using namespace variants;
using Lingmo::framework::tr;
using Lingmo::i18n::format;

static const std::string connectionSetting = "connection";
static const std::string wirelessSetting = "802-11-wireless";
static const std::string wirelessSecurity = "802-11-wireless-security";

namespace {

// How networks are told apart: by what joining one asks for, not by the
// exact scheme, so a router whose bands differ only in WPA2 and WPA3 is still
// one network.
enum class SecurityClass {
    Open,
    Wep,
    Personal,
    Enterprise,
};

} // namespace

static SecurityClass classOf(SecurityType type)
{
    switch (type) {
    case SecurityType::StaticWep:
    case SecurityType::DynamicWep:
    case SecurityType::Leap:
        return SecurityClass::Wep;
    case SecurityType::WpaPsk:
    case SecurityType::Wpa2Psk:
    case SecurityType::Sae:
        return SecurityClass::Personal;
    case SecurityType::WpaEap:
    case SecurityType::Wpa2Eap:
    case SecurityType::Wpa3SuiteB192:
        return SecurityClass::Enterprise;
    default:
        return SecurityClass::Open;
    }
}

static SecurityClass profileClass(const Client& client, const std::string& profile)
{
    if (!client.settings(profile).count(wirelessSecurity))
        return SecurityClass::Open;

    const std::string keyMgmt = toString(client.setting(profile, wirelessSecurity, "key-mgmt"));
    if (keyMgmt == "none" || keyMgmt == "ieee8021x")
        return SecurityClass::Wep;
    if (keyMgmt == "wpa-eap" || keyMgmt == "wpa-eap-suite-b-192")
        return SecurityClass::Enterprise;
    if (keyMgmt == "owe")
        return SecurityClass::Open;
    return SecurityClass::Personal;
}

static bool isPort(const Client& client, const std::string& profile)
{
    return !toString(client.setting(profile, connectionSetting, "controller")).empty()
           || !toString(client.setting(profile, connectionSetting, "master")).empty();
}

static int64_t lastUsed(const Client& client, const std::string& profile)
{
    return int64_t(toUInt64(client.setting(profile, connectionSetting, "timestamp")));
}

static std::string profileOf(const Client& client, const std::string& active)
{
    return objectPath(client.property(active, activeInterface, "Connection"));
}

static bool contains(const std::vector<std::string>& list, const std::string& value)
{
    return std::find(list.begin(), list.end(), value) != list.end();
}

// The profiles NetworkManager would activate on the device, and the one it has active.
static std::vector<std::string> profilesFor(const Client& client, const std::string& device, const std::string& type)
{
    std::vector<std::string> profiles;
    std::vector<std::string> candidates = toStringList(client.property(device, deviceInterface, "AvailableConnections"));
    const std::string active = client.activeConnection(device);
    if (!active.empty())
        candidates.push_back(profileOf(client, active));

    for (const std::string& profile : candidates) {
        if (contains(profiles, profile) || !client.readable(profile) || isPort(client, profile)
            || toString(client.setting(profile, connectionSetting, "type")) != type)
            continue;
        profiles.push_back(profile);
    }
    return profiles;
}

static std::string hexAddress(const std::string& bytes)
{
    std::string text;
    for (size_t i = 0; i < bytes.size(); ++i) {
        char part[4];
        std::snprintf(part, sizeof(part), i ? ":%02x" : "%02x", static_cast<unsigned char>(bytes[i]));
        text += part;
    }
    return text;
}

// A Wi-Fi profile NetworkManager would use on the device once it lists the network.
static std::vector<std::string> laterProfilesFor(const Client& client, const std::string& device,
                                                 const std::vector<std::string>& offered)
{
    const std::string interface = toString(client.property(device, deviceInterface, "Interface"));
    const std::string hwAddress = toString(client.property(device, deviceInterface, "HwAddress"));

    std::vector<std::string> profiles;
    for (const std::string& profile : client.connections()) {
        if (contains(offered, profile) || isPort(client, profile)
            || toString(client.setting(profile, connectionSetting, "type")) != wirelessSetting)
            continue;
        const std::string boundInterface = toString(client.setting(profile, connectionSetting, "interface-name"));
        const std::string boundAddress = toBytes(client.setting(profile, wirelessSetting, "mac-address"));
        if ((!boundInterface.empty() && boundInterface != interface)
            || (!boundAddress.empty() && !text::equalsIgnoringCase(hexAddress(boundAddress), hwAddress)))
            continue;
        profiles.push_back(profile);
    }
    return profiles;
}

static std::string firstAddress(const std::vector<VariantMap>& addressData, bool skipLinkLocal)
{
    std::string fallback;
    for (const VariantMap& entry : addressData) {
        const std::string address = toString(value(entry, "address"));
        if (!skipLinkLocal || !inet::isLinkLocal(address))
            return address;
        if (fallback.empty())
            fallback = address;
    }
    return fallback;
}

static std::string hexBytes(const std::string& bytes)
{
    std::string text;
    for (const unsigned char byte : bytes) {
        char part[3];
        std::snprintf(part, sizeof(part), "%02x", byte);
        text += part;
    }
    return text;
}

NetworkModel::NetworkModel()
    : m_client(Client::instance())
{
    m_subscriptions.push_back(m_client->changed.connect([this] { rebuild(); }));
    m_subscriptions.push_back(m_client->activeStateChanged.connect(
        [this](const std::string& active, uint32_t state, uint32_t reason) {
            const std::string profile = profileOf(*m_client, active);
            if (!vpn::isVpn(*m_client, profile))
                return;
            if (state == ActiveActivating) {
                m_vpnFailures.erase(profile);
            } else if (state == ActiveDeactivated) {
                const std::string message = vpn::failureMessage(reason);
                if (!message.empty())
                    m_vpnFailures[profile] = message;
            }
        }));
    rebuild();
}

NetworkModel::~NetworkModel() = default;

void NetworkModel::rebuild()
{
    std::vector<NetworkItem> items;
    if (m_client->ready()) {
        for (const std::string& device : m_client->devices()) {
            if (!toBool(m_client->property(device, deviceInterface, "Managed")))
                continue;
            if (m_client->deviceType(device) == DeviceEthernet)
                addWired(device, items);
            else if (m_client->deviceType(device) == DeviceWifi)
                addWireless(device, items);
        }
        addVpns(items);
    }

    // A name shown twice, say on two adapters, gets its device added.
    std::map<std::string, int> names;
    for (const NetworkItem& item : items)
        ++names[item.name];
    for (NetworkItem& item : items) {
        item.uniqueName = names[item.name] > 1 && !item.deviceName.empty()
                              // TRANSLATORS: a network and the adapter it is on, such as "Home (wlan0)".
                              ? format(tr("{name} ({device})"), {{"name", item.name}, {"device", item.deviceName}})
                              : item.name;
        item.securityTypeString = wifi::securityLabel(item.securityType);
        fillDetails(item);
    }

    // Rows that stay keep their place; new ones come last.
    std::vector<NetworkItem> next;
    next.reserve(items.size());
    for (const NetworkItem& old : m_items) {
        const auto found = std::find_if(items.begin(), items.end(),
                                        [&old](const NetworkItem& item) { return item.key == old.key; });
        if (found != items.end())
            next.push_back(*found);
    }
    for (const NetworkItem& item : items) {
        const bool present = std::any_of(next.begin(), next.end(),
                                         [&item](const NetworkItem& kept) { return kept.key == item.key; });
        if (!present)
            next.push_back(item);
    }

    if (next == m_items)
        return;
    m_items = std::move(next);
    changed.emit();
}

void NetworkModel::addWired(const std::string& device, std::vector<NetworkItem>& items) const
{
    const std::string active = m_client->activeConnection(device);
    const std::string activeProfile = profileOf(*m_client, active);

    for (const std::string& profile : profilesFor(*m_client, device, "802-3-ethernet")) {
        NetworkItem item;
        item.type = ConnectionType::Wired;
        item.uuid = toString(m_client->setting(profile, connectionSetting, "uuid"));
        item.key = device + "|" + item.uuid;
        item.name = toString(m_client->setting(profile, connectionSetting, "id"));
        item.connectionPath = profile;
        item.devicePath = device;
        item.deviceName = toString(m_client->property(device, deviceInterface, "Interface"));
        item.deviceState = m_client->deviceState(device);
        item.securityType = SecurityType::None;
        item.timestamp = lastUsed(*m_client, profile);
        if (profile == activeProfile)
            item.connectionState = ConnectionStatus(toInt(m_client->property(active, activeInterface, "State")));
        item.icon = item.connectionState == ConnectionStatus::Activated ? "network-wired-activated" : "network-wired";
        items.push_back(item);
    }
}

void NetworkModel::addWireless(const std::string& device, std::vector<NetworkItem>& items) const
{
    struct Network
    {
        std::string ssid;
        SecurityClass securityClass = SecurityClass::Open;
        SecurityType security = SecurityType::Unknown;
        std::string accessPoint;
        int strength = -1;
        std::string profile;
        int64_t timestamp = 0;
        // NetworkManager lists one of its access points, rather than only remembering one.
        bool listed = false;
    };

    const auto ap = [this](const std::string& path, const char* name) {
        return m_client->property(path, accessPointInterface, name);
    };

    const std::string deviceName = toString(m_client->property(device, deviceInterface, "Interface"));
    const std::string hwAddress = toString(m_client->property(device, deviceInterface, "HwAddress"));
    const uint32_t capabilities = toUInt(m_client->property(device, wirelessInterface, "WirelessCapabilities"));
    const std::string activeAp = objectPath(m_client->property(device, wirelessInterface, "ActiveAccessPoint"));
    const std::string active = m_client->activeConnection(device);
    const std::string activeProfile = profileOf(*m_client, active);
    const ConnectionStatus activeState =
        active.empty() ? ConnectionStatus::Deactivated
                       : ConnectionStatus(toInt(m_client->property(active, activeInterface, "State")));

    std::vector<Network> networks;
    int activeNetwork = -1;
    const std::vector<std::string> listed = m_client->accessPoints(device);
    std::vector<std::string> accessPoints = listed;
    for (const std::string& remembered : m_client->rememberedAccessPoints(device))
        accessPoints.push_back(remembered);
    for (size_t i = 0; i < accessPoints.size(); ++i) {
        const std::string& path = accessPoints[i];
        const bool remembered = i >= listed.size();
        const std::string ssid = wifi::ssidKey(toBytes(ap(path, "Ssid")));
        const uint32_t rsn = toUInt(ap(path, "RsnFlags"));
        // Hidden networks have no name to show; our own hotspot shows up as an
        // access point with this adapter's address; and an OWE transition
        // network's encrypted half is joined through its open one.
        if (wifi::ssidHidden(ssid) || toUInt(ap(path, "Mode")) != ModeInfrastructure
            || (!hwAddress.empty() && text::equalsIgnoringCase(toString(ap(path, "HwAddress")), hwAddress))
            || ((rsn & KeyMgmtOwe) && (rsn & KeyMgmtOweTm)))
            continue;

        const SecurityType security = wifi::security(capabilities, toUInt(ap(path, "Flags")),
                                                     toUInt(ap(path, "WpaFlags")), rsn, ModeInfrastructure);
        const SecurityClass securityClass = classOf(security);
        const int strength = toInt(ap(path, "Strength"));

        int index = -1;
        for (size_t n = 0; n < networks.size(); ++n) {
            if (networks[n].ssid == ssid && networks[n].securityClass == securityClass) {
                index = int(n);
                break;
            }
        }
        if (index < 0) {
            networks.push_back({ssid, securityClass});
            index = int(networks.size()) - 1;
        }

        // The access point in use stands for its network; otherwise the
        // strongest listed one does, and a remembered one only while none is listed.
        Network& network = networks[size_t(index)];
        if (path == activeAp)
            activeNetwork = index;
        else if (activeNetwork == index || (remembered && network.listed) || strength <= network.strength)
            continue;
        network.accessPoint = path;
        network.strength = strength;
        network.security = security;
        network.listed = !remembered;
    }

    bool activeUnlisted = false;
    const std::vector<std::string> offered = profilesFor(*m_client, device, wirelessSetting);
    // NetworkManager offers no profile for a network it no longer lists.
    std::vector<std::string> profiles = offered;
    for (const std::string& later : laterProfilesFor(*m_client, device, offered))
        profiles.push_back(later);
    for (const std::string& profile : profiles) {
        const std::string mode = toString(m_client->setting(profile, wirelessSetting, "mode"));
        if (!mode.empty() && mode != "infrastructure")
            continue;

        const std::string ssid = wifi::ssidKey(toBytes(m_client->setting(profile, wirelessSetting, "ssid")));
        const SecurityClass securityClass = profileClass(*m_client, profile);
        const bool isActive = profile == activeProfile;
        const bool isOffered = contains(offered, profile);

        int index = isActive ? activeNetwork : -1;
        int sameName = -1;
        for (size_t n = 0; index < 0 && n < networks.size(); ++n) {
            if (networks[n].ssid != ssid || (!isOffered && networks[n].listed))
                continue;
            if (networks[n].securityClass == securityClass)
                index = int(n);
            else
                sameName = sameName == -1 ? int(n) : -2;
        }
        // A lone network of that name is the profile's, whatever its scheme says now.
        if (index < 0 && sameName >= 0)
            index = sameName;
        if (index < 0) {
            if (isActive)
                activeUnlisted = true;
            continue;
        }

        // Several profiles for one network: the active one, else the one used last.
        Network& network = networks[size_t(index)];
        const int64_t timestamp = lastUsed(*m_client, profile);
        if (!network.profile.empty() && !isActive
            && (network.profile == activeProfile || network.timestamp >= timestamp))
            continue;
        network.profile = profile;
        network.timestamp = timestamp;
    }

    const auto addItem = [&](const Network& network, const std::string& key) {
        NetworkItem item;
        item.type = ConnectionType::Wireless;
        item.key = key;
        item.ssid = wifi::ssidLabel(network.ssid);
        item.devicePath = device;
        item.deviceName = deviceName;
        item.deviceState = m_client->deviceState(device);
        item.specificPath = network.accessPoint;
        item.securityType = network.security;
        item.signal = std::max(network.strength, 0);
        item.icon = wifi::strengthIcon(item.signal);
        if (network.profile.empty()) {
            item.name = item.ssid;
        } else {
            item.connectionPath = network.profile;
            item.uuid = toString(m_client->setting(network.profile, connectionSetting, "uuid"));
            item.name = toString(m_client->setting(network.profile, connectionSetting, "id"));
            item.timestamp = network.timestamp;
        }
        // Another user's private profile is still in use here, just not ours to change.
        const bool inUse = !active.empty()
                           && (network.profile.empty() ? !network.accessPoint.empty() && network.accessPoint == activeAp
                                                       : network.profile == activeProfile);
        if (inUse)
            item.connectionState = activeState;
        items.push_back(item);
    };

    for (const Network& network : networks) {
        const char securityClass = char('0' + int(network.securityClass));
        addItem(network, device + "|" + hexBytes(network.ssid) + "|" + std::string(1, securityClass));
    }

    // Connected to a network no scan has listed, a hidden one for instance.
    if (activeUnlisted) {
        Network network;
        network.ssid = wifi::ssidKey(toBytes(m_client->setting(activeProfile, wirelessSetting, "ssid")));
        network.profile = activeProfile;
        network.timestamp = lastUsed(*m_client, activeProfile);
        network.accessPoint = activeAp;
        switch (profileClass(*m_client, activeProfile)) {
        case SecurityClass::Open:
            network.security = SecurityType::None;
            break;
        case SecurityClass::Wep:
            network.security = SecurityType::StaticWep;
            break;
        case SecurityClass::Personal:
            network.security = SecurityType::Wpa2Psk;
            break;
        case SecurityClass::Enterprise:
            network.security = SecurityType::Wpa2Eap;
            break;
        }
        if (!activeAp.empty())
            network.strength = toInt(ap(activeAp, "Strength"));
        addItem(network, device + "|" + activeProfile);
    }
}

void NetworkModel::addVpns(std::vector<NetworkItem>& items) const
{
    for (const std::string& profile : m_client->connections()) {
        if (!vpn::isVpn(*m_client, profile))
            continue;

        NetworkItem item;
        item.type = ConnectionType::Vpn;
        item.uuid = toString(m_client->setting(profile, connectionSetting, "uuid"));
        item.key = "vpn|" + item.uuid;
        item.name = toString(m_client->setting(profile, connectionSetting, "id"));
        item.connectionPath = profile;
        item.securityType = SecurityType::None;
        item.timestamp = lastUsed(*m_client, profile);
        item.connectionState = vpn::state(*m_client, vpn::activeConnection(*m_client, profile));
        item.icon = "network-vpn";

        const std::string kind = vpn::kind(*m_client, profile);
        item.vpnType = vpn::kindLabel(kind);
        const std::string service = toString(m_client->setting(profile, "vpn", "service-type"));
        const bool installed = kind == "wireguard" || vpn::pluginInstalled(service);

        const std::map<std::string, std::string> data = toStringMap(m_client->setting(profile, "vpn", "data"));
        const std::string userNameKey = vpn::userNameKey(service, data);
        item.vpnUsesUserName = !userNameKey.empty();
        const auto userName = data.find(userNameKey);
        item.vpnUserName = userName == data.end() ? std::string() : userName->second;
        if (!installed) {
            item.canActivate = false;
            item.statusMessage = format(tr("Install {package} to use this VPN."), {{"package", vpn::package(kind)}});
        } else if (kind == "openconnect") {
            item.canActivate = false;
            item.statusMessage = tr("Signing in to OpenConnect VPNs isn't supported yet.");
        } else {
            const auto failure = m_vpnFailures.find(profile);
            item.statusMessage = failure == m_vpnFailures.end() ? std::string() : failure->second;
        }
        items.push_back(item);
    }
}

void NetworkModel::fillDetails(NetworkItem& item) const
{
    // Addresses are those of the device the network is up on; a VPN's come
    // from its active connection, as its row has no device.
    if (item.connectionState != ConnectionStatus::Activated)
        return;

    const std::string device = item.devicePath;
    const std::string vpnActive =
        item.type == ConnectionType::Vpn ? vpn::activeConnection(*m_client, item.connectionPath) : std::string();
    const auto config = [&](const char* name) {
        return objectPath(vpnActive.empty() ? m_client->property(device, deviceInterface, name)
                                            : m_client->property(vpnActive, activeInterface, name));
    };
    const std::string ip4 = config("Ip4Config");
    const std::string ip6 = config("Ip6Config");

    item.ipV4Address = firstAddress(toMapList(m_client->property(ip4, ip4ConfigInterface, "AddressData")), false);
    item.ipV6Address = firstAddress(toMapList(m_client->property(ip6, ip6ConfigInterface, "AddressData")), true);

    item.gateway = toString(m_client->property(ip4, ip4ConfigInterface, "Gateway"));
    if (item.gateway.empty())
        item.gateway = toString(m_client->property(ip6, ip6ConfigInterface, "Gateway"));

    item.nameServer = firstAddress(toMapList(m_client->property(ip4, ip4ConfigInterface, "NameserverData")), false);
    if (item.nameServer.empty()) {
        // Network byte order, as NetworkManager before 1.14 only offered it.
        const std::vector<uint32_t> legacy = toUIntList(m_client->property(ip4, ip4ConfigInterface, "Nameservers"));
        if (!legacy.empty())
            item.nameServer = inet::formatIpv4(be32toh(legacy.front()));
    }
    if (item.nameServer.empty()) {
        const std::vector<std::string> servers = toBytesList(m_client->property(ip6, ip6ConfigInterface, "Nameservers"));
        if (!servers.empty())
            item.nameServer = inet::formatIpv6Bytes(servers.front());
    }

    item.macAddress = toString(m_client->property(device, deviceInterface, "HwAddress"));
}

int compareNames(const std::string& a, const std::string& b)
{
    return base::Locale::system().compare(a, b);
}

// Greater ranks are listed first. The signal counts by the icon's step, so
// rows move only when what they show changes; a lower state is further along.
static auto rank(const NetworkItem& item)
{
    return std::make_tuple(item.connectionState == ConnectionStatus::Activated, -int(item.connectionState),
                           !item.uuid.empty(), item.timestamp, wifi::strengthStep(item.signal));
}

std::vector<NetworkItem> listItems(const std::vector<NetworkItem>& items, ConnectionType type)
{
    std::vector<NetworkItem> list;
    if (type == ConnectionType::Unknown)
        return list;
    std::copy_if(items.begin(), items.end(), std::back_inserter(list),
                 [type](const NetworkItem& item) { return item.type == type; });

    // A VPN keeps its place by name, whether it is up or not.
    const bool byNameOnly = type == ConnectionType::Vpn;
    std::stable_sort(list.begin(), list.end(), [byNameOnly](const NetworkItem& a, const NetworkItem& b) {
        if (!byNameOnly && rank(a) != rank(b))
            return rank(a) > rank(b);
        return compareNames(a.name, b.name) < 0;
    });
    return list;
}

} // namespace Lingmo::Networkmanager
