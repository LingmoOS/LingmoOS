// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "base/timer.h"
#include "lingmo/networkmanager/ip_utils.h"
#include "lingmo/networkmanager/types.h"
#include "lingmo/networkmanager/vpn_schema.h"
#include "lingmo/networkmanager/vpn_utils.h"

namespace Lingmo::base {
class Process;
}

namespace Lingmo::Networkmanager {

class Client;
class SecretReader;

// A Wi-Fi network in range: its adapter, name and security.
struct WirelessNetwork
{
    std::string device;
    std::string name;
    SecurityType security = SecurityType::Unknown;
};

// A VPN's form, filled from its profile when one is edited.
struct VpnForm
{
    std::string kind;
    std::string label;
    bool installed = false;
    std::string package;
    std::string name;
    bool shared = false;
    bool ownNetworkOnly = false;
    bool exportable = false;
    std::vector<std::string> exportPatterns;
    // A plugin VPN's fields.
    std::vector<vpn::schema::FormField> fields;
    // WireGuard's.
    vpn::WireGuardForm wireguard;
};

// What saveVpn() is given back from a form.
struct VpnValues
{
    std::string kind;
    std::string name;
    bool shared = false;
    bool ownNetworkOnly = false;
    vpn::schema::Values values;
    std::map<std::string, uint32_t> storage;
    std::map<std::string, std::string> secrets;
    vpn::WireGuardForm wireguard;
};

// The secrets a VPN profile keeps, for its form.
struct VpnSecrets
{
    // A plugin VPN's, by key.
    std::map<std::string, std::string> values;
    // WireGuard's.
    std::string privateKey;
    // By peer public key.
    std::map<std::string, std::string> presharedKeys;
};

// Actions on NetworkManager. Each returns at once; the result shows up in
// the models as NetworkManager reports it.
class Handler
{
public:
    Handler();
    ~Handler();

    Handler(const Handler&) = delete;
    Handler& operator=(const Handler&) = delete;

    // A Wi-Fi adapter can run an access point, or at least an ad-hoc network.
    bool hotspotCapable() const { return m_hotspotCapable; }
    // Wi-Fi is on and system policy lets this user share a connection.
    bool hotspotAllowed() const { return m_hotspotAllowed; }
    // createHotspot() can start one now.
    bool hotspotSupported() const { return m_hotspotCapable && m_hotspotAllowed; }
    // The Wi-Fi network that starting the hotspot would disconnect from, if any.
    const std::string& hotspotReplaces() const { return m_hotspotReplaces; }
    bool hotspotActive() const { return m_hotspotActive; }

    void activateConnection(const std::string& connection, const std::string& device,
                            const std::string& specificObject);
    // Joins the access point `specificObject` with a new profile. Without a
    // password, a network that needs one has it asked for by the session's
    // secret agent before anything is added, and a new profile that never gets
    // to connect is removed again.
    void addAndActivateConnection(const std::string& device, const std::string& specificObject,
                                  const std::string& password = {});
    // Joins the WPA or WPA2 Enterprise network `specificObject` with a user name and password.
    void addAndActivateEnterprise(const std::string& device, const std::string& specificObject,
                                  const std::string& userName, const std::string& password);
    // Joins a network that doesn't broadcast its name. `type` is one of
    // "none", "wep", "wpa", "wpa2-wpa3", "wpa3", "wpa-eap", "wpa2-eap" and "wpa3-eap".
    bool addOtherConnection(const std::string& ssid, const std::string& userName, const std::string& password,
                            const std::string& type);
    // Nothing once the access point is gone.
    std::optional<WirelessNetwork> wirelessNetwork(const std::string& accessPoint) const;
    void deactivateConnection(const std::string& connection, const std::string& device);
    void removeConnection(const std::string& connection);
    void requestScan(const std::string& interface = {});

    void enableWireless(bool enable, std::function<void(const base::dbus::Reply&)> completion = {});
    // Connects or disconnects every managed Ethernet device and keeps it that
    // way, as NetworkManager has no global switch for wired networking.
    void enableWired(bool enable);

    void createHotspot();
    void stopHotspot();

    // Adds the VPN a provider's configuration file describes: WireGuard's
    // .conf, or anything an installed NetworkManager plugin can read, such as
    // OpenVPN's .ovpn.
    void importVpn(const std::string& file);
    std::vector<vpn::Kind> vpnKinds() const;
    // The form for a kind of VPN, filled from `connection` when one is edited.
    VpnForm vpnForm(const std::string& kind, const std::string& connection) const;
    // The secrets a profile keeps; vpnSecretsLoaded answers.
    void loadVpnSecrets(const std::string& connection);
    // Adds the VPN the form describes, or saves it over `connection`.
    // vpnAdded or vpnSaved follows.
    void saveVpn(const std::string& connection, const VpnValues& form);
    // Writes the profile as the file its plugin reads, or a wg-quick file.
    void exportVpn(const std::string& connection, const std::string& file);

    // A profile's addressing as its form shows it.
    ip::Form ipForm(const std::string& connection) const;
    // The addresses it has while up, for filling in a manual setup.
    ip::Form currentAddresses(const std::string& connection) const;
    // Saves the form's addressing over the profile and applies it to the
    // device the profile is up on, without disconnecting. ipSaved or
    // ipSaveFailed follows.
    void saveIp(const std::string& connection, const ip::Form& form);

    base::Signal<const std::string& /*uuid*/> vpnAdded;
    base::Signal<const std::string& /*message*/> vpnAddFailed;
    base::Signal<const std::string& /*connection*/> vpnSaved;
    base::Signal<const std::string& /*message*/> vpnSaveFailed;
    base::Signal<const std::string& /*connection*/, const VpnSecrets&> vpnSecretsLoaded;
    base::Signal<const std::string& /*file*/> vpnExported;
    base::Signal<const std::string& /*message*/> vpnExportFailed;
    base::Signal<const std::string& /*connection*/> ipSaved;
    base::Signal<const std::string& /*connection*/, const std::string& /*message*/> ipSaveFailed;
    // A plugin VPN went down on its own with `message` saying why; the name is the profile's.
    base::Signal<const std::string& /*name*/, const std::string& /*message*/> vpnFailed;
    base::Signal<> hotspotSupportedChanged;
    base::Signal<> hotspotActiveChanged;
    // A hotspot this handler started failed to come up.
    base::Signal<const std::string& /*message*/> hotspotFailed;

private:
    struct PendingJoin
    {
        std::string accessPoint;
        std::string device;
        base::dbus::VariantMap properties;
        std::function<void(const std::string&)> join;
    };

    void withScannedAccessPoint(const std::string& accessPoint, std::function<void(const std::string&)> join);
    void joinScanned();
    void addAndActivateWireless(const std::string& device, const std::string& specificObject,
                                const std::string& password, bool askForPassword);
    SecurityType accessPointSecurity(const std::string& device, const std::string& accessPoint) const;
    void addAndActivate(const Settings& settings, const std::string& device, const std::string& specificObject);
    void forgetUnlessActivated(const std::string& connection, const std::string& active);
    void importVpnAs(const std::string& file, std::vector<std::string> types, const std::string& error);
    void addPrivateVpn(Settings profile);
    void getSecrets(const std::string& connection, const std::string& setting,
                    std::function<void(const Settings&)> done);
    void activeStateChanged(const std::string& active, uint32_t state);
    void reportVpnFailure(const std::string& active, uint32_t reason);
    void reapply(const std::string& connection);
    void update();
    std::string hotspotActiveConnection() const;
    std::string hotspotDevice(uint32_t* capabilities = nullptr) const;
    void hotspotRequestFinished(const base::dbus::Reply& reply, size_t activePathArgument);
    // Runs nmcli; done gets whether it started, then its exit code and output.
    void runNmcli(const std::vector<std::string>& arguments,
                  std::function<void(bool started, bool succeeded, const std::string& output,
                                     const std::string& errorOutput)>
                      done);

    Client* m_client;
    CallContext m_context;
    std::unique_ptr<SecretReader> m_secrets;
    std::vector<base::Subscription> m_subscriptions;
    std::vector<std::unique_ptr<base::Process>> m_processes;
    std::map<uint64_t, base::dbus::PendingCall> m_sessionCalls;
    uint64_t m_nextSessionCall = 0;
    bool m_scanRequested = false;
    std::string m_scanInterface;
    bool m_hotspotCapable = false;
    bool m_hotspotAllowed = false;
    std::string m_hotspotReplaces;
    bool m_hotspotActive = false;
    // The hotspot activation this handler started, until it is up or has failed.
    std::string m_hotspotActivation;
    // New profiles by the active connection of their first activation.
    std::map<std::string, std::string> m_forget;
    std::vector<std::unique_ptr<base::Timer>> m_forgetTimers;
    // Remembered networks clicked before a scan listed them again.
    std::vector<PendingJoin> m_pendingJoins;
};

} // namespace Lingmo::Networkmanager
