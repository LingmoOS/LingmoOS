// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/networkmanager/vpn_utils.h"
#include "lingmo/networkmanager/client.h"
#include "lingmo/networkmanager/inet.h"
#include "lingmo/networkmanager/text.h"
#include "lingmo/networkmanager/variants.h"
#include "lingmo/networkmanager/vpn_schema.h"
#include "framework_i18n.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <endian.h>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace Lingmo::Networkmanager::vpn {

using base::dbus::Variant;
using base::dbus::VariantMap;
using namespace variants;
using Lingmo::framework::tr;
using Lingmo::i18n::format;

static const std::string servicePrefix = "org.freedesktop.NetworkManager.";

namespace {

struct Plugin
{
    std::string name;
    std::vector<std::string> services;
};

struct PluginCache
{
    std::vector<std::string> directories{"/etc/NetworkManager/VPN", "/usr/lib/NetworkManager/VPN"};
    std::vector<std::filesystem::file_time_type> stamps;
    bool read = false;
    std::vector<Plugin> plugins;
};

} // namespace

static PluginCache& cache()
{
    static PluginCache cache;
    return cache;
}

static const VariantMap& group(const Settings& settings, const std::string& name)
{
    static const VariantMap empty;
    const auto found = settings.find(name);
    return found == settings.end() ? empty : found->second;
}

static bool contains(const std::vector<std::string>& list, const std::string& value)
{
    return std::find(list.begin(), list.end(), value) != list.end();
}

// The [VPN Connection] group of a plugin's .name file.
static Plugin readNameFile(const std::filesystem::path& path)
{
    Plugin plugin;
    std::ifstream file(path);
    if (!file)
        return plugin;

    bool inGroup = false;
    std::string line;
    while (std::getline(file, line)) {
        line = text::trimmed(line);
        if (!line.empty() && line[0] == '[') {
            inGroup = line == "[VPN Connection]";
            continue;
        }
        const size_t equals = line.find('=');
        if (!inGroup || equals == std::string::npos)
            continue;

        const std::string key = text::trimmed(line.substr(0, equals));
        const std::string value = text::trimmed(line.substr(equals + 1));
        if (key == "name") {
            plugin.name = value;
        } else if (key == "service") {
            plugin.services.insert(plugin.services.begin(), value);
        } else if (key == "aliases") {
            for (const std::string& alias : text::split(value, ';'))
                plugin.services.push_back(alias);
        }
    }
    return plugin;
}

// Read again only when a directory changed: plugins come and go with packages.
static const std::vector<Plugin>& plugins()
{
    PluginCache& cache = ::Lingmo::Networkmanager::vpn::cache();
    std::vector<std::filesystem::file_time_type> stamps;
    for (const std::string& directory : cache.directories) {
        std::error_code error;
        stamps.push_back(std::filesystem::last_write_time(directory, error));
    }
    if (cache.read && stamps == cache.stamps)
        return cache.plugins;

    cache.read = true;
    cache.stamps = stamps;
    cache.plugins.clear();
    for (const std::string& directory : cache.directories) {
        std::vector<std::filesystem::path> files;
        std::error_code error;
        for (const auto& entry : std::filesystem::directory_iterator(directory, error)) {
            if (entry.is_regular_file() && entry.path().extension() == ".name")
                files.push_back(entry.path());
        }
        std::sort(files.begin(), files.end());
        for (const auto& file : files) {
            Plugin plugin = readNameFile(file);
            if (!plugin.name.empty() && !plugin.services.empty())
                cache.plugins.push_back(std::move(plugin));
        }
    }
    return cache.plugins;
}

static std::string shortName(const std::string& serviceType)
{
    for (const Plugin& plugin : plugins()) {
        if (contains(plugin.services, serviceType))
            return plugin.name;
    }
    if (text::startsWith(serviceType, servicePrefix))
        return serviceType.substr(servicePrefix.size());
    const size_t dot = serviceType.rfind('.');
    return dot == std::string::npos ? serviceType : serviceType.substr(dot + 1);
}

static bool decodeBase64(const std::string& text, std::string* out)
{
    static const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string data = text;
    while (!data.empty() && data.back() == '=')
        data.pop_back();
    if (text.size() - data.size() > 2 || data.size() % 4 == 1)
        return false;
    std::string decoded;
    uint32_t buffer = 0;
    int bits = 0;
    for (const char c : data) {
        const size_t index = alphabet.find(c);
        if (index == std::string::npos)
            return false;
        buffer = (buffer << 6) | uint32_t(index);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            decoded += char((buffer >> bits) & 0xff);
        }
    }
    *out = decoded;
    return true;
}

static bool isWireGuardKey(const std::string& value)
{
    std::string decoded;
    return decodeBase64(value, &decoded) && decoded.size() == 32;
}

static bool parseUInt(const std::string& text, uint32_t* value, int base = 10)
{
    if (text.empty() || text[0] == '-' || text[0] == '+' || std::isspace(static_cast<unsigned char>(text[0])))
        return false;
    char* end = nullptr;
    const unsigned long long number = std::strtoull(text.c_str(), &end, base);
    if (!end || *end != '\0' || number > 0xffffffffull)
        return false;
    *value = uint32_t(number);
    return true;
}

// "address" or "address/prefix" into NetworkManager's address-data entry.
static bool parseWireGuardAddress(const std::string& word, VariantMap& out, bool& v4)
{
    const size_t slash = word.find('/');
    const std::string address = inet::normalized(word.substr(0, slash));
    if (address.empty())
        return false;
    v4 = inet::family(address) == 4;
    int prefix = v4 ? 32 : 128;
    if (slash != std::string::npos) {
        const std::string length = word.substr(slash + 1);
        char* end = nullptr;
        const long parsed = std::strtol(length.c_str(), &end, 10);
        if (length.empty() || !end || *end != '\0' || parsed < 0 || parsed > (v4 ? 32 : 128))
            return false;
        prefix = int(parsed);
    }
    out = {{"address", Variant(address)}, {"prefix", Variant(uint32_t(prefix))}};
    return true;
}

// Servers go by family, anything else is a search domain.
static void parseWireGuardDns(const std::vector<std::string>& words, std::vector<uint32_t>& dns4,
                              std::vector<std::vector<uint8_t>>& dns6, std::vector<std::string>& search)
{
    for (const std::string& word : words) {
        if (const auto v4 = inet::parseIpv4(word))
            dns4.push_back(htobe32(*v4));
        else if (const auto v6 = inet::parseIpv6(word))
            dns6.emplace_back(v6->begin(), v6->end());
        else
            search.push_back(word);
    }
}

// host[:port] as the setting wants it: a bare IPv6 address gets brackets, as
// NetworkManager would take its last group for the port, and the port
// defaults to 51820.
static std::string wireGuardEndpoint(const std::string& server)
{
    if (server.empty())
        return {};
    std::string endpoint = server;
    const size_t lastColon = server.rfind(':');
    if (inet::family(server) == 6
        && (lastColon == std::string::npos || inet::family(server.substr(0, lastColon)) == 0))
        endpoint = "[" + server + "]";
    if (endpoint.find(':') == std::string::npos || text::endsWith(endpoint, "]"))
        endpoint += ":51820";
    return endpoint;
}

// Interface names are at most 15 characters of a few kinds.
static std::string wireGuardInterfaceName(const std::string& name)
{
    std::string interface;
    for (const char c : name) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')
            interface += c;
    }
    interface = interface.substr(0, 15);
    return interface.empty() ? std::string("wg0") : interface;
}

static std::vector<std::string> trimmedWords(const std::string& value)
{
    std::vector<std::string> words;
    for (const std::string& word : text::split(value, ',')) {
        std::string trimmed = text::trimmed(word);
        if (!trimmed.empty())
            words.push_back(std::move(trimmed));
    }
    return words;
}

bool isVpn(const Client& client, const std::string& connection)
{
    if (!client.readable(connection))
        return false;
    const std::string type = toString(client.setting(connection, "connection", "type"));
    if (type != "vpn" && type != "wireguard")
        return false;
    // A port belongs to the profile it is a port of.
    return toString(client.setting(connection, "connection", "controller")).empty()
           && toString(client.setting(connection, "connection", "master")).empty();
}

std::string kind(const Client& client, const std::string& connection)
{
    if (toString(client.setting(connection, "connection", "type")) == "wireguard")
        return "wireguard";
    return shortName(toString(client.setting(connection, "vpn", "service-type")));
}

std::string kindLabel(const std::string& kind)
{
    if (kind == "wireguard")
        return "WireGuard";
    if (kind == "openvpn")
        return "OpenVPN";
    if (kind == "l2tp")
        return "L2TP";
    if (kind == "pptp")
        return "PPTP";
    if (kind == "sstp")
        return "SSTP";
    if (kind == "vpnc")
        return "Cisco VPN";
    if (kind == "openconnect")
        return "OpenConnect";
    if (kind == "strongswan" || kind == "libreswan")
        return "IPsec";
    if (kind == "fortisslvpn")
        return "Fortinet SSL VPN";
    return kind.empty() ? std::string("VPN") : kind;
}

std::string package(const std::string& kind)
{
    return "network-manager-" + kind;
}

bool pluginInstalled(const std::string& serviceType)
{
    for (const Plugin& plugin : plugins()) {
        if (contains(plugin.services, serviceType))
            return true;
    }
    return false;
}

std::vector<std::string> installedPlugins()
{
    std::vector<std::string> names;
    for (const Plugin& plugin : plugins()) {
        if (!contains(names, plugin.name))
            names.push_back(plugin.name);
    }
    return names;
}

void setPluginDirectories(const std::vector<std::string>& directories)
{
    cache() = PluginCache();
    cache().directories = directories;
}

std::string activeConnection(const Client& client, const std::string& connection)
{
    for (const std::string& active : client.activeConnections()) {
        if (objectPath(client.property(active, activeInterface, "Connection")) == connection)
            return active;
    }
    return {};
}

ConnectionStatus state(const Client& client, const std::string& active)
{
    if (active.empty() || !client.has(active, activeInterface))
        return ConnectionStatus::Deactivated;

    // NM_VPN_CONNECTION_STATE_*: the tunnel is up only once the plugin says so.
    if (client.has(active, vpnConnectionInterface)) {
        const uint32_t vpnState = toUInt(client.property(active, vpnConnectionInterface, "VpnState"));
        if (vpnState >= 1 && vpnState <= 4)
            return ConnectionStatus::Activating;
        if (vpnState == 5)
            return ConnectionStatus::Activated;
        if (vpnState >= 6)
            return ConnectionStatus::Deactivated;
    }
    return ConnectionStatus(toInt(client.property(active, activeInterface, "State")));
}

std::string failureMessage(uint32_t reason)
{
    // NM_ACTIVE_CONNECTION_STATE_REASON_*.
    switch (reason) {
    case 1:
    case 2:
    case 9:
    case 11:
        return {};
    case 3:
        return tr("The network connection was interrupted.");
    case 4:
        return tr("The VPN service stopped unexpectedly.");
    case 5:
        return tr("The VPN service returned invalid configuration.");
    case 6:
        return tr("The connection attempt timed out.");
    case 7:
        return tr("The VPN service did not start in time.");
    case 8:
        return tr("The VPN service failed to start.");
    case 10:
        return tr("Sign-in failed. Check the user name and password.");
    default:
        return tr("The VPN service reported an error.");
    }
}

std::string userNameKey(const std::string& serviceType, const std::map<std::string, std::string>& data)
{
    const auto valueOf = [&data](const std::string& key) {
        const auto found = data.find(key);
        return found == data.end() ? std::string() : found->second;
    };
    const std::string kind = shortName(serviceType);
    if (kind == "openvpn") {
        const std::string type = valueOf("connection-type");
        return type == "password" || type == "password-tls" ? std::string("username") : std::string();
    }
    if (kind == "l2tp") {
        const std::string type = valueOf("user-auth-type");
        return type.empty() || type == "password" ? std::string("user") : std::string();
    }
    if (kind == "pptp" || kind == "sstp" || kind == "fortisslvpn")
        return "user";
    if (kind == "vpnc")
        return "Xauth username";
    if (kind == "strongswan" && valueOf("method") == "eap")
        return "user";
    return {};
}

std::vector<Kind> manualKinds()
{
    std::vector<Kind> kinds;
    kinds.push_back({"wireguard", "WireGuard", true, package("wireguard")});
    for (const std::string& kind : schema::kinds()) {
        // The two IPsec plugins share a label on the list of VPNs, but not here.
        std::string label = kindLabel(kind);
        if (kind == "strongswan")
            label = "IPsec (strongSwan)";
        else if (kind == "libreswan")
            label = "IPsec (Libreswan)";
        else if (kind == "ssh")
            label = "SSH";
        else if (kind == "iodine")
            label = tr("DNS tunnel (iodine)");
        kinds.push_back({kind, label, pluginInstalled(servicePrefix + kind), package(kind)});
    }
    return kinds;
}

bool isWireGuardConfig(const std::string& content)
{
    return content.find("[Interface]") != std::string::npos && content.find("PrivateKey") != std::string::npos;
}

std::optional<Settings> wireGuardProfile(const std::string& content, const std::string& name, std::string* error)
{
    const auto fail = [error](const std::string& message) -> std::optional<Settings> {
        if (error)
            *error = message;
        return std::nullopt;
    };

    enum { None, Interface, Peer } section = None;
    std::string privateKey;
    uint32_t listenPort = 0;
    uint32_t fwmark = 0;
    uint32_t mtu = 0;
    bool peerRoutes = true;
    std::vector<VariantMap> addresses[2];
    std::vector<uint32_t> dns4;
    std::vector<std::vector<uint8_t>> dns6;
    std::vector<std::string> search;
    std::vector<VariantMap> peers;

    // Like `wg setconf`: whitespace anywhere is dropped and '#' starts a comment.
    std::vector<std::string> lines;
    {
        std::string line;
        std::istringstream stream(content);
        while (std::getline(stream, line))
            lines.push_back(line);
        if (!content.empty() && content.back() == '\n')
            lines.emplace_back();
    }
    for (size_t number = 1; number <= lines.size(); ++number) {
        std::string line;
        for (const char c : lines[number - 1].substr(0, lines[number - 1].find('#'))) {
            if (!std::isspace(static_cast<unsigned char>(c)))
                line += c;
        }
        if (line.empty())
            continue;

        if (text::equalsIgnoringCase(line, "[Interface]")) {
            section = Interface;
            continue;
        }
        if (text::equalsIgnoringCase(line, "[Peer]")) {
            section = Peer;
            peers.emplace_back();
            continue;
        }

        const size_t equals = line.find('=');
        const std::string field = line.substr(0, equals);
        const std::string value = equals == std::string::npos ? std::string() : line.substr(equals + 1);
        const std::vector<std::string> words = text::split(value, ',');
        const std::string invalid
            // TRANSLATORS: {key} is a WireGuard setting as written in the file, such as "PrivateKey".
            = format(tr("Line {line} has an invalid {key}."), {{"line", std::to_string(number)}, {"key", field}});
        const auto is = [&field](const char* name) { return text::equalsIgnoringCase(field, name); };
        bool ok = true;

        if (section == Interface && is("PrivateKey")) {
            if (!isWireGuardKey(value))
                return fail(invalid);
            privateKey = value;
        } else if (section == Interface && is("Address")) {
            for (const std::string& word : words) {
                bool v4 = false;
                VariantMap address;
                if (!parseWireGuardAddress(word, address, v4))
                    return fail(invalid);
                addresses[v4 ? 0 : 1].push_back(address);
            }
        } else if (section == Interface && is("DNS")) {
            parseWireGuardDns(words, dns4, dns6, search);
        } else if (section == Interface && is("MTU")) {
            ok = parseUInt(value, &mtu);
        } else if (section == Interface && is("ListenPort")) {
            ok = parseUInt(value, &listenPort) && listenPort <= 0xffff;
        } else if (section == Interface && is("FwMark")) {
            fwmark = 0;
            ok = value == "off" || parseUInt(value, &fwmark, 0);
        } else if (section == Interface && is("Table")) {
            // Route tables by number or name aren't carried over; "off" means no routes.
            peerRoutes = value != "off";
        } else if (section == Interface
                   && (is("PreUp") || is("PreDown") || is("PostUp") || is("PostDown") || is("SaveConfig"))) {
            // No scripts are run.
        } else if (section == Peer && is("PublicKey")) {
            ok = isWireGuardKey(value);
            peers.back()["public-key"] = Variant(value);
        } else if (section == Peer && is("PresharedKey")) {
            ok = isWireGuardKey(value);
            peers.back()["preshared-key"] = Variant(value);
            peers.back()["preshared-key-flags"] = Variant(uint32_t(SecretSystemOwned));
        } else if (section == Peer && is("Endpoint")) {
            ok = value.find(':') != std::string::npos;
            peers.back()["endpoint"] = Variant(value);
        } else if (section == Peer && is("AllowedIPs")) {
            std::vector<std::string> allowed = toStringList(variants::value(peers.back(), "allowed-ips"));
            allowed.insert(allowed.end(), words.begin(), words.end());
            peers.back()["allowed-ips"] = Variant(allowed);
        } else if (section == Peer && is("PersistentKeepalive")) {
            uint32_t seconds = 0;
            ok = value == "off" || parseUInt(value, &seconds);
            ok = ok && seconds <= 0xffff;
            peers.back()["persistent-keepalive"] = Variant(seconds);
        } else {
            return fail(format(tr("Line {line} isn't part of a WireGuard configuration."),
                               {{"line", std::to_string(number)}}));
        }
        if (!ok)
            return fail(invalid);
    }

    if (privateKey.empty())
        return fail(tr("The configuration has no private key."));
    for (const VariantMap& peer : peers) {
        if (!variants::contains(peer, "public-key"))
            return fail(tr("A peer has no public key."));
    }

    Settings profile;
    profile["connection"] = {
        {"id", Variant(name)},
        {"uuid", Variant(text::newUuid())},
        {"type", Variant(std::string("wireguard"))},
        {"interface-name", Variant(wireGuardInterfaceName(name))},
    };
    profile["wireguard"] = {
        {"private-key", Variant(privateKey)},
        {"private-key-flags", Variant(uint32_t(SecretSystemOwned))},
        {"listen-port", Variant(listenPort)},
        {"fwmark", Variant(fwmark)},
        {"mtu", Variant(mtu)},
        {"peer-routes", Variant(peerRoutes)},
        {"peers", Variant(peers)},
    };

    for (int family = 0; family < 2; ++family) {
        VariantMap ip;
        ip["method"] = Variant(std::string(addresses[family].empty() ? "disabled" : "manual"));
        if (!addresses[family].empty()) {
            ip["address-data"] = Variant(addresses[family]);
            // DNS needs an address of its family; without search domains, the
            // server answers for every name, as wg-quick has it.
            const bool hasDns = family == 0 ? !dns4.empty() : !dns6.empty();
            if (hasDns) {
                ip["dns"] = family == 0 ? Variant(dns4) : Variant(dns6);
                ip["dns-search"] = Variant(search.empty() ? std::vector<std::string>{"~"} : search);
            }
        }
        profile[family == 0 ? "ipv4" : "ipv6"] = ip;
    }
    return profile;
}

WireGuardForm wireGuardForm(const Settings& settings)
{
    const VariantMap& wireguard = group(settings, "wireguard");
    std::vector<std::string> addresses;
    std::vector<std::string> dns;
    for (const char* family : {"ipv4", "ipv6"}) {
        const VariantMap& ip = group(settings, family);
        for (const VariantMap& address : toMapList(value(ip, "address-data")))
            addresses.push_back(toString(value(address, "address")) + "/" + std::to_string(toUInt(value(address, "prefix"))));
        if (std::string(family) == "ipv4") {
            for (const uint32_t server : toUIntList(value(ip, "dns")))
                dns.push_back(inet::formatIpv4(be32toh(server)));
        } else {
            for (const std::string& server : toBytesList(value(ip, "dns"))) {
                std::string bytes = server.substr(0, 16);
                bytes.resize(16, '\0');
                dns.push_back(inet::formatIpv6Bytes(bytes));
            }
        }
        // Both families carry the same search domains.
        for (const std::string& domain : toStringList(value(ip, "dns-search"))) {
            if (domain != "~" && std::find(dns.begin(), dns.end(), domain) == dns.end())
                dns.push_back(domain);
        }
    }

    WireGuardForm form;
    for (const VariantMap& peer : toMapList(value(wireguard, "peers"))) {
        WireGuardPeer row;
        row.publicKey = toString(value(peer, "public-key"));
        row.hasPresharedKey = variants::contains(peer, "preshared-key")
                              || (variants::contains(peer, "preshared-key-flags")
                                  && !(toUInt(value(peer, "preshared-key-flags")) & SecretNotRequired));
        row.endpoint = toString(value(peer, "endpoint"));
        row.allowedIps = text::join(toStringList(value(peer, "allowed-ips")), ", ");
        const uint32_t keepalive = toUInt(value(peer, "persistent-keepalive"));
        row.keepalive = keepalive > 0 ? std::to_string(keepalive) : std::string();
        form.peers.push_back(std::move(row));
    }

    const auto number = [&wireguard](const char* key) {
        const uint32_t number = toUInt(value(wireguard, key));
        return number > 0 ? std::to_string(number) : std::string();
    };
    form.interfaceName = toString(value(group(settings, "connection"), "interface-name"));
    form.addresses = text::join(addresses, ", ");
    form.dns = text::join(dns, ", ");
    form.listenPort = number("listen-port");
    form.fwmark = number("fwmark");
    form.mtu = number("mtu");
    form.peerRoutes = !variants::contains(wireguard, "peer-routes") || toBool(value(wireguard, "peer-routes"));
    return form;
}

std::optional<Settings> wireGuardSettings(const WireGuardForm& form, const Settings& existing, std::string* error)
{
    const auto fail = [error](const std::string& message) -> std::optional<Settings> {
        if (error)
            *error = message;
        return std::nullopt;
    };

    const VariantMap& stored = group(existing, "wireguard");
    const std::string privateKey = text::trimmed(form.privateKey);
    if (privateKey.empty() && !variants::contains(stored, "private-key-flags"))
        return fail(tr("Enter the private key."));
    if (!privateKey.empty() && !isWireGuardKey(privateKey))
        return fail(tr("The private key isn't a WireGuard key."));

    std::vector<VariantMap> addresses[2];
    for (const std::string& word : trimmedWords(form.addresses)) {
        bool v4 = false;
        VariantMap address;
        if (!parseWireGuardAddress(word, address, v4))
            return fail(format(tr("{address} isn't an address with a prefix, like 10.0.0.2/24."), {{"address", word}}));
        addresses[v4 ? 0 : 1].push_back(address);
    }
    if (addresses[0].empty() && addresses[1].empty())
        return fail(tr("Enter the address."));

    std::vector<uint32_t> dns4;
    std::vector<std::vector<uint8_t>> dns6;
    std::vector<std::string> search;
    parseWireGuardDns(trimmedWords(form.dns), dns4, dns6, search);

    uint32_t listenPort = 0;
    if (!form.listenPort.empty() && (!parseUInt(form.listenPort, &listenPort) || listenPort > 0xffff))
        return fail(tr("The listen port must be a number up to 65535."));
    uint32_t fwmark = 0;
    if (!form.fwmark.empty() && !parseUInt(form.fwmark, &fwmark, 0))
        return fail(tr("The firewall mark must be a number."));
    uint32_t mtu = 0;
    if (!form.mtu.empty() && !parseUInt(form.mtu, &mtu))
        return fail(tr("The MTU must be a number."));

    const std::vector<VariantMap> storedPeers = toMapList(value(stored, "peers"));
    std::vector<VariantMap> peers;
    if (form.peers.empty())
        return fail(tr("Add the server as a peer."));
    for (const WireGuardPeer& formPeer : form.peers) {
        const std::string publicKey = text::trimmed(formPeer.publicKey);
        if (!isWireGuardKey(publicKey))
            return fail(tr("A peer's public key isn't a WireGuard key."));
        VariantMap peer{{"public-key", Variant(publicKey)}};

        const std::string presharedKey = text::trimmed(formPeer.presharedKey);
        bool keepPresharedKey = false;
        for (const VariantMap& storedPeer : storedPeers)
            keepPresharedKey |= toString(value(storedPeer, "public-key")) == publicKey
                                && variants::contains(storedPeer, "preshared-key-flags");
        if (!presharedKey.empty()) {
            if (!isWireGuardKey(presharedKey))
                return fail(tr("A peer's preshared key isn't a WireGuard key."));
            peer["preshared-key"] = Variant(presharedKey);
            peer["preshared-key-flags"] = Variant(uint32_t(SecretSystemOwned));
        } else if (keepPresharedKey && formPeer.hasPresharedKey) {
            peer["preshared-key-flags"] = Variant(uint32_t(SecretSystemOwned));
        }

        const std::string endpoint = wireGuardEndpoint(text::trimmed(formPeer.endpoint));
        if (!endpoint.empty())
            peer["endpoint"] = Variant(endpoint);
        const std::vector<std::string> allowed = trimmedWords(formPeer.allowedIps);
        for (const std::string& word : allowed) {
            bool v4 = false;
            VariantMap address;
            if (!parseWireGuardAddress(word, address, v4))
                return fail(format(tr("{address} isn't a network, like 10.0.0.0/24."), {{"address", word}}));
        }
        peer["allowed-ips"] = Variant(allowed.empty() ? std::vector<std::string>{"0.0.0.0/0", "::/0"} : allowed);
        uint32_t seconds = 0;
        if (!formPeer.keepalive.empty() && (!parseUInt(formPeer.keepalive, &seconds) || seconds > 0xffff))
            return fail(tr("The keepalive must be a number of seconds up to 65535."));
        peer["persistent-keepalive"] = Variant(seconds);
        peers.push_back(std::move(peer));
    }

    Settings profile = existing;
    VariantMap connection = profile["connection"];
    if (!variants::contains(connection, "uuid")) {
        connection["uuid"] = Variant(text::newUuid());
        connection["type"] = Variant(std::string("wireguard"));
    }
    const std::string name = text::trimmed(form.name);
    if (!name.empty())
        connection["id"] = Variant(name);
    const std::string interfaceName = text::trimmed(form.interfaceName);
    connection["interface-name"] = Variant(
        wireGuardInterfaceName(interfaceName.empty() ? toString(value(connection, "id")) : interfaceName));
    profile["connection"] = connection;

    VariantMap wireguard = stored;
    if (!privateKey.empty())
        wireguard["private-key"] = Variant(privateKey);
    wireguard["private-key-flags"] = Variant(uint32_t(SecretSystemOwned));
    wireguard["listen-port"] = Variant(listenPort);
    wireguard["fwmark"] = Variant(fwmark);
    wireguard["mtu"] = Variant(mtu);
    wireguard["peer-routes"] = Variant(form.peerRoutes);
    wireguard["peers"] = Variant(peers);
    profile["wireguard"] = wireguard;

    for (int family = 0; family < 2; ++family) {
        VariantMap ip = profile[family == 0 ? "ipv4" : "ipv6"];
        for (const char* key : {"address-data", "addresses", "dns", "dns-search", "gateway"})
            ip.erase(key);
        ip["method"] = Variant(std::string(addresses[family].empty() ? "disabled" : "manual"));
        if (!addresses[family].empty()) {
            ip["address-data"] = Variant(addresses[family]);
            const bool hasDns = family == 0 ? !dns4.empty() : !dns6.empty();
            if (hasDns) {
                ip["dns"] = family == 0 ? Variant(dns4) : Variant(dns6);
                ip["dns-search"] = Variant(search.empty() ? std::vector<std::string>{"~"} : search);
            }
        }
        profile[family == 0 ? "ipv4" : "ipv6"] = ip;
    }
    return profile;
}

std::string wireGuardConfig(const Settings& settings)
{
    const WireGuardForm form = wireGuardForm(settings);
    const VariantMap& wireguard = group(settings, "wireguard");
    std::string out = "[Interface]\n";
    out += "PrivateKey = " + toString(value(wireguard, "private-key")) + "\n";
    out += "Address = " + form.addresses + "\n";
    if (!form.dns.empty())
        out += "DNS = " + form.dns + "\n";
    if (!form.listenPort.empty())
        out += "ListenPort = " + form.listenPort + "\n";
    if (!form.fwmark.empty())
        out += "FwMark = " + form.fwmark + "\n";
    if (!form.mtu.empty())
        out += "MTU = " + form.mtu + "\n";
    if (!form.peerRoutes)
        out += "Table = off\n";

    for (const VariantMap& peer : toMapList(value(wireguard, "peers"))) {
        out += "\n[Peer]\nPublicKey = " + toString(value(peer, "public-key")) + "\n";
        if (variants::contains(peer, "preshared-key"))
            out += "PresharedKey = " + toString(value(peer, "preshared-key")) + "\n";
        if (variants::contains(peer, "endpoint"))
            out += "Endpoint = " + toString(value(peer, "endpoint")) + "\n";
        const std::vector<std::string> allowed = toStringList(value(peer, "allowed-ips"));
        if (!allowed.empty())
            out += "AllowedIPs = " + text::join(allowed, ", ") + "\n";
        const uint32_t keepalive = toUInt(value(peer, "persistent-keepalive"));
        if (keepalive)
            out += "PersistentKeepalive = " + std::to_string(keepalive) + "\n";
    }
    return out;
}

} // namespace Lingmo::Networkmanager::vpn
