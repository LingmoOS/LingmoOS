// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "base/dbus.h"

namespace Lingmo::Networkmanager {

// A profile as GetSettings sends it: a{sa{sv}}, by setting name.
using Settings = std::map<std::string, base::dbus::VariantMap>;

inline const std::string service = "org.freedesktop.NetworkManager";
inline const std::string rootPath = "/org/freedesktop";
inline const std::string managerPath = "/org/freedesktop/NetworkManager";
inline const std::string settingsPath = "/org/freedesktop/NetworkManager/Settings";

inline const std::string managerInterface = "org.freedesktop.NetworkManager";
inline const std::string settingsInterface = "org.freedesktop.NetworkManager.Settings";
inline const std::string connectionInterface = "org.freedesktop.NetworkManager.Settings.Connection";
inline const std::string activeInterface = "org.freedesktop.NetworkManager.Connection.Active";
inline const std::string vpnConnectionInterface = "org.freedesktop.NetworkManager.VPN.Connection";
inline const std::string deviceInterface = "org.freedesktop.NetworkManager.Device";
inline const std::string wirelessInterface = "org.freedesktop.NetworkManager.Device.Wireless";
inline const std::string statisticsInterface = "org.freedesktop.NetworkManager.Device.Statistics";
inline const std::string accessPointInterface = "org.freedesktop.NetworkManager.AccessPoint";
inline const std::string ip4ConfigInterface = "org.freedesktop.NetworkManager.IP4Config";
inline const std::string ip6ConfigInterface = "org.freedesktop.NetworkManager.IP6Config";
inline const std::string propertiesInterface = "org.freedesktop.DBus.Properties";
inline const std::string objectManagerInterface = "org.freedesktop.DBus.ObjectManager";

// Values from NetworkManager's nm-dbus-interface.h and nm-setting.h.
enum DeviceType : uint32_t {
    DeviceEthernet = 1,
    DeviceWifi = 2,
};

enum DeviceState : uint32_t {
    DeviceUnknown = 0,
    DeviceUnmanaged = 10,
    DeviceUnavailable = 20,
    DeviceDisconnected = 30,
    DevicePrepare = 40,
    DeviceActivated = 100,
    DeviceDeactivating = 110,
    DeviceFailed = 120,
};

enum ActiveState : uint32_t {
    ActiveUnknown = 0,
    ActiveActivating = 1,
    ActiveActivated = 2,
    ActiveDeactivating = 3,
    ActiveDeactivated = 4,
};

enum WifiMode : uint32_t {
    ModeAdhoc = 1,
    ModeInfrastructure = 2,
    ModeAp = 3,
    ModeMesh = 4,
};

enum ApFlags : uint32_t {
    ApPrivacy = 0x1,
};

enum ApSecurity : uint32_t {
    PairWep40 = 0x1,
    PairWep104 = 0x2,
    PairTkip = 0x4,
    PairCcmp = 0x8,
    GroupWep40 = 0x10,
    GroupWep104 = 0x20,
    GroupTkip = 0x40,
    GroupCcmp = 0x80,
    KeyMgmtPsk = 0x100,
    KeyMgmt8021x = 0x200,
    KeyMgmtSae = 0x400,
    KeyMgmtOwe = 0x800,
    KeyMgmtOweTm = 0x1000,
    KeyMgmtEapSuiteB192 = 0x2000,
};

enum WifiCapabilities : uint32_t {
    CapWep40 = 0x1,
    CapWep104 = 0x2,
    CapTkip = 0x4,
    CapCcmp = 0x8,
    CapWpa = 0x10,
    CapRsn = 0x20,
    CapAp = 0x40,
    CapAdhoc = 0x80,
    CapIbssRsn = 0x2000,
};

enum SecretFlags : uint32_t {
    SecretSystemOwned = 0x0,
    SecretAgentOwned = 0x1,
    SecretNotSaved = 0x2,
    SecretNotRequired = 0x4,
};

// How a row shows a connection; ActiveState's values.
enum class ConnectionStatus {
    Unknown = 0,
    Activating,
    Activated,
    Deactivating,
    Deactivated,
};

enum class ConnectionType {
    Unknown = 0,
    Wired,
    Wireless,
    Vpn,
};

enum class SecurityType {
    Unknown = -1,
    None = 0,
    StaticWep,
    DynamicWep,
    Leap,
    WpaPsk,
    WpaEap,
    Wpa2Psk,
    Wpa2Eap,
    Sae,
    Wpa3SuiteB192,
    Owe,
};

// Calls made for an object hand their replies over only while the object's
// context lives, so a reply arriving after it is gone goes nowhere.
class CallContext
{
public:
    CallContext()
        : m_token(std::make_shared<char>())
    {
    }

    std::weak_ptr<char> token() const { return m_token; }

private:
    std::shared_ptr<char> m_token;
};

} // namespace Lingmo::Networkmanager
