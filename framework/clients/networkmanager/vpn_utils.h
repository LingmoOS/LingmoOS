// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "lingmo/networkmanager/types.h"

namespace Lingmo::Networkmanager {

class Client;

namespace vpn {

// A VPN profile this user may read: a NetworkManager VPN plugin's or WireGuard.
bool isVpn(const Client& client, const std::string& connection);
// "wireguard", or the plugin's short name, e.g. "openvpn" for
// org.freedesktop.NetworkManager.openvpn.
std::string kind(const Client& client, const std::string& connection);
std::string kindLabel(const std::string& kind);
// The Debian package that brings the plugin.
std::string package(const std::string& kind);

// Whether NetworkManager has a plugin for the service, as its .name files say.
bool pluginInstalled(const std::string& serviceType);
// Short names of the installed plugins, for importing a file.
std::vector<std::string> installedPlugins();
// Where the .name files are looked for; tests point it elsewhere.
void setPluginDirectories(const std::vector<std::string>& directories);

// The active connection of the profile, if it is up or on its way.
std::string activeConnection(const Client& client, const std::string& connection);
// The status a row shows for the active connection, which for a plugin VPN
// follows the VPN's own state.
ConnectionStatus state(const Client& client, const std::string& active);
// What to tell the user about an active connection that ended for this
// reason; empty when they ended it themselves or cancelled its password.
std::string failureMessage(uint32_t reason);
// The vpn.data key of the user name the plugin signs in with, if this kind of
// VPN has one; NetworkManager's auth dialogs never ask for it, the profile has
// to hold it.
std::string userNameKey(const std::string& serviceType, const std::map<std::string, std::string>& data);

struct Kind
{
    std::string kind;
    std::string label;
    bool installed = false;
    std::string package;
};
// The kinds of VPN a form exists for, WireGuard first.
std::vector<Kind> manualKinds();

// Whether a file's start reads as a wg-quick configuration.
bool isWireGuardConfig(const std::string& content);
// A WireGuard profile from a wg-quick configuration, read as NetworkManager's
// nm_conn_wireguard_import() reads it; nothing, with `error` set, when it
// isn't one. The id is `name`, and the interface is named after it.
std::optional<Settings> wireGuardProfile(const std::string& content, const std::string& name, std::string* error);

struct WireGuardPeer
{
    std::string publicKey;
    // Empty: the stored one, if any, is kept while hasPresharedKey says so.
    std::string presharedKey;
    bool hasPresharedKey = false;
    std::string endpoint;
    // Comma-separated.
    std::string allowedIps;
    // Seconds; empty for off.
    std::string keepalive;
};

// A WireGuard profile as its form shows it, the text as typed: numbers empty
// for automatic or off, addresses and DNS comma-separated.
struct WireGuardForm
{
    // The profile's name, applied when not empty.
    std::string name;
    std::string interfaceName;
    // A secret: empty keeps the stored one.
    std::string privateKey;
    std::string addresses;
    std::string dns;
    std::string listenPort;
    std::string fwmark;
    std::string mtu;
    bool peerRoutes = true;
    std::vector<WireGuardPeer> peers;
};

WireGuardForm wireGuardForm(const Settings& settings);
// The profile the form describes, over `existing` when a profile is edited.
// Nothing, with `error` set, when a value is invalid.
std::optional<Settings> wireGuardSettings(const WireGuardForm& form, const Settings& existing, std::string* error);
// The profile as a wg-quick file, with its secrets when `settings` holds them.
std::string wireGuardConfig(const Settings& settings);

} // namespace vpn
} // namespace Lingmo::Networkmanager
