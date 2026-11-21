// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/networkmanager/handler.h"
#include "lingmo/networkmanager/client.h"
#include "lingmo/networkmanager/hotspot.h"
#include "lingmo/networkmanager/secret_reader.h"
#include "lingmo/networkmanager/text.h"
#include "lingmo/networkmanager/variants.h"
#include "lingmo/networkmanager/wifi_utils.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <regex>
#include <sys/stat.h>

#include "base/file.h"
#include "base/process.h"
#include "framework_i18n.h"

namespace Lingmo::Networkmanager {

using namespace std::chrono_literals;
using base::dbus::Reply;
using base::dbus::Variant;
using base::dbus::VariantMap;
using namespace variants;
using Lingmo::framework::tr;

// The shell's secret agent, which asks for a new network's password up front.
static const std::string promptService = "org.lingmo.SecretAgent";
static const std::string promptPath = "/SecretAgent";
static const std::string promptInterface = "org.lingmo.SecretAgent";

static const std::string wirelessSecurity = "802-11-wireless-security";

static constexpr auto kForgetWatchTimeout = 2min;

static Variant path(const std::string& objectPath)
{
    return objectPathValue(objectPath);
}

// An object path of the reply, empty for NetworkManager's "/".
static std::string replyPath(const Reply& reply, size_t index)
{
    const auto path = reply.value<base::dbus::ObjectPath>(index);
    return path && path->path != "/" ? path->path : std::string();
}

static bool contains(const std::vector<std::string>& list, const std::string& value)
{
    return std::find(list.begin(), list.end(), value) != list.end();
}

// The profile for running a hotspot as an access point, or as an ad-hoc
// network on an adapter that cannot.
static Settings hotspotSettings(const std::string& name, const std::string& uuid, const std::string& password,
                                uint32_t capabilities)
{
    const bool accessPoint = capabilities & CapAp;

    Settings settings;
    settings["connection"] = {{"id", Variant(name)},
                              {"uuid", Variant(uuid)},
                              {"type", Variant(std::string("802-11-wireless"))},
                              {"autoconnect", Variant(false)}};
    // NetworkManager picks a channel that is valid where the machine is.
    settings["802-11-wireless"] = {{"ssid", bytesValue(wifi::ssidBytes(name))},
                                   {"mode", Variant(std::string(accessPoint ? "ap" : "adhoc"))}};
    settings["ipv4"] = {{"method", Variant(std::string("shared"))}};
    settings["ipv6"] = {{"method", Variant(std::string("ignore"))}};
    if (password.empty())
        return settings;

    VariantMap& security = settings[wirelessSecurity];
    if (!accessPoint) {
        // Ad-hoc networks only know WEP; type 2 hashes the password into the key.
        security["key-mgmt"] = Variant(std::string("none"));
        security["wep-key-type"] = Variant(uint32_t(2));
        security["wep-key0"] = Variant(password);
        return settings;
    }

    security["key-mgmt"] = Variant(std::string("wpa-psk"));
    security["psk"] = Variant(password);
    // WPA2 with AES alone when the adapter has it.
    if (capabilities & CapRsn) {
        const Variant ccmp(std::vector<std::string>{"ccmp"});
        security["proto"] = Variant(std::vector<std::string>{"rsn"});
        security["pairwise"] = ccmp;
        security["group"] = ccmp;
    }
    return settings;
}

Handler::Handler()
    : m_client(Client::instance())
{
    m_subscriptions.push_back(m_client->changed.connect([this] { update(); }));
    m_subscriptions.push_back(
        m_client->activeStateChanged.connect([this](const std::string& active, uint32_t state, uint32_t reason) {
            activeStateChanged(active, state);
            if (state == ActiveDeactivated)
                reportVpnFailure(active, reason);
        }));
    m_subscriptions.push_back(m_client->objectRemoved.connect([this](const std::string& path) {
        // Gone without a final state: the activation never made it.
        if (m_forget.count(path) || path == m_hotspotActivation)
            activeStateChanged(path, ActiveDeactivated);
    }));
    m_subscriptions.push_back(m_client->readyChanged.connect([this] {
        if (!m_client->ready()) {
            m_forget.clear();
            return;
        }
        if (m_scanRequested) {
            m_scanRequested = false;
            requestScan(m_scanInterface);
        }
    }));
    update();
}

Handler::~Handler() = default;

void Handler::activateConnection(const std::string& connection, const std::string& device,
                                 const std::string& specificObject)
{
    // Without an access point NetworkManager waits for a scan to find the profile's network.
    const std::string accessPoint =
        m_client->remembers(specificObject) ? m_client->scannedAccessPoint(specificObject) : specificObject;
    m_client->call(managerPath, managerInterface, "ActivateConnection",
                   {path(connection), path(device), path(accessPoint)}, &m_context, {}, Client::authorizedTimeout);
}

void Handler::addAndActivateConnection(const std::string& device, const std::string& specificObject,
                                       const std::string& password)
{
    addAndActivateWireless(device, specificObject, password, true);
}

void Handler::addAndActivateWireless(const std::string& device, const std::string& specificObject,
                                     const std::string& password, bool askForPassword)
{
    // A profile added without an access point is saved as a hidden network.
    if (m_client->remembers(specificObject)) {
        withScannedAccessPoint(specificObject, [this, device, password, askForPassword](const std::string& accessPoint) {
            addAndActivateWireless(device, accessPoint, password, askForPassword);
        });
        return;
    }
    if (!m_client->has(specificObject, accessPointInterface))
        return;

    const std::string name = wifi::ssidLabel(toBytes(m_client->property(specificObject, accessPointInterface, "Ssid")));
    const SecurityType security = accessPointSecurity(device, specificObject);

    // Enterprise networks need a user name; see addAndActivateEnterprise().
    if (wifi::enterprise(security))
        return;

    // Asked for before anything is added: activating first would save the
    // network and show it connecting while the user is still typing, or has
    // cancelled.
    base::dbus::Connection* session = base::dbus::Connection::session();
    if (askForPassword && password.empty() && wifi::needsPassword(security)) {
        const auto retry = [this, device, specificObject] {
            // Without the shell there is no one to ask first; NetworkManager
            // asks whichever agent there is once activation needs the secret.
            addAndActivateWireless(device, specificObject, {}, false);
        };
        if (!session) {
            retry();
            return;
        }
        base::dbus::Message message = session->newMethodCall(promptService, promptPath, promptInterface, "ConnectWireless");
        message.append(device, specificObject, name, security == SecurityType::StaticWep);
        const uint64_t id = ++m_nextSessionCall;
        auto finished = std::make_shared<bool>(false);
        base::dbus::PendingCall pending = session->callAsync(message, [this, id, finished, retry](const Reply& reply) {
            *finished = true;
            m_sessionCalls.erase(id);
            if (reply.isError())
                retry();
        });
        if (!*finished)
            m_sessionCalls.emplace(id, std::move(pending));
        return;
    }

    // NetworkManager completes the rest from the access point: the SSID's own
    // bytes, the mode, and the key management, which for a WPA2/WPA3 network
    // is wpa-psk so drivers without SAE still connect.
    VariantMap connection{{"id", Variant(name)}};
    // A shared profile keeps its password in NetworkManager for every user; a
    // private one keeps it in the user's keyring through the secret agent.
    const bool shared = m_client->sharesNewConnections();
    if (!shared)
        connection["permissions"] = Variant(std::vector<std::string>{Client::userPermission()});

    Settings settings;
    settings["connection"] = connection;
    if (wifi::needsPassword(security)) {
        const uint32_t secretFlags = shared ? SecretSystemOwned : SecretAgentOwned;
        VariantMap secrets;
        if (security == SecurityType::StaticWep) {
            secrets["wep-key-flags"] = Variant(secretFlags);
            if (!password.empty()) {
                secrets["wep-key0"] = Variant(password);
                // A 5 or 13 character, or 10 or 26 digit key; not a passphrase.
                secrets["wep-key-type"] = Variant(uint32_t(1));
            }
        } else {
            secrets["psk-flags"] = Variant(secretFlags);
            if (!password.empty())
                secrets["psk"] = Variant(password);
        }
        settings[wirelessSecurity] = secrets;
    }

    addAndActivate(settings, device, specificObject);
}

void Handler::addAndActivateEnterprise(const std::string& device, const std::string& specificObject,
                                       const std::string& userName, const std::string& password)
{
    if (m_client->remembers(specificObject)) {
        withScannedAccessPoint(specificObject, [this, device, userName, password](const std::string& accessPoint) {
            addAndActivateEnterprise(device, accessPoint, userName, password);
        });
        return;
    }
    if (!m_client->has(specificObject, accessPointInterface))
        return;

    // A 192-bit network signs in with EAP-TLS, which needs certificates.
    const SecurityType security = accessPointSecurity(device, specificObject);
    if (security != SecurityType::WpaEap && security != SecurityType::Wpa2Eap)
        return;

    VariantMap connection{
        {"id", Variant(wifi::ssidLabel(toBytes(m_client->property(specificObject, accessPointInterface, "Ssid"))))},
    };
    const bool shared = m_client->sharesNewConnections();
    if (!shared)
        connection["permissions"] = Variant(std::vector<std::string>{Client::userPermission()});

    // NetworkManager completes the SSID and wpa-eap key management from the
    // access point; only the 802.1X sign-in has to be spelt out.
    Settings settings;
    settings["connection"] = connection;
    settings["802-1x"] = wifi::passwordEap(userName, password, shared ? SecretSystemOwned : SecretAgentOwned);
    addAndActivate(settings, device, specificObject);
}

bool Handler::addOtherConnection(const std::string& ssid, const std::string& userName, const std::string& password,
                                 const std::string& type)
{
    if (ssid.empty() || ssid.size() > 32)
        return false;

    // Shared like a network joined from the list; see addAndActivateWireless().
    const bool shared = m_client->sharesNewConnections();
    const uint32_t secretFlags = shared ? SecretSystemOwned : SecretAgentOwned;

    VariantMap connection{
        {"id", Variant(ssid)},
        {"uuid", Variant(text::newUuid())},
        {"type", Variant(std::string("802-11-wireless"))},
    };
    if (!shared)
        connection["permissions"] = Variant(std::vector<std::string>{Client::userPermission()});

    Settings settings;
    settings["connection"] = connection;
    // Nothing can be learnt from an access point that hides its name, so everything is spelt out.
    settings["802-11-wireless"] = {
        {"ssid", bytesValue(ssid)},
        {"mode", Variant(std::string("infrastructure"))},
        {"hidden", Variant(true)},
    };
    settings["ipv4"] = {{"method", Variant(std::string("auto"))}};

    VariantMap security;
    if (type == "wep") {
        // The "key" type takes both 5/13-character ASCII and 10/26-digit hex keys.
        security = {{"key-mgmt", Variant(std::string("none"))},
                    {"auth-alg", Variant(std::string("open"))},
                    {"wep-key-type", Variant(uint32_t(1))},
                    {"wep-tx-keyidx", Variant(uint32_t(0))},
                    {"wep-key0", Variant(password)},
                    {"wep-key-flags", Variant(secretFlags)}};
    } else if (type == "wpa" || type == "wpa2-wpa3") {
        // wpa-psk also joins WPA2/WPA3 transition networks.
        security = {{"key-mgmt", Variant(std::string("wpa-psk"))},
                    {"proto", Variant(std::vector<std::string>{type == "wpa" ? "wpa" : "rsn"})},
                    {"psk", Variant(password)},
                    {"psk-flags", Variant(secretFlags)}};
    } else if (type == "wpa3") {
        security = {{"key-mgmt", Variant(std::string("sae"))},
                    {"psk", Variant(password)},
                    {"psk-flags", Variant(secretFlags)}};
    } else if (text::endsWith(type, "-eap")) {
        security = {{"key-mgmt", Variant(std::string("wpa-eap"))},
                    {"proto", Variant(std::vector<std::string>{type == "wpa-eap" ? "wpa" : "rsn"})}};
        // NM_SETTING_WIRELESS_SECURITY_PMF_REQUIRED, an int32 property.
        if (type == "wpa3-eap")
            security["pmf"] = Variant(int32_t(3));

        settings["802-1x"] = wifi::passwordEap(userName, password, secretFlags);
    }
    if (!security.empty())
        settings[wirelessSecurity] = security;

    std::string device;
    for (const std::string& candidate : m_client->devices()) {
        if (m_client->deviceType(candidate) == DeviceWifi && m_client->deviceState(candidate) >= DeviceDisconnected
            && toUInt(m_client->property(candidate, wirelessInterface, "Mode")) != ModeAp) {
            device = candidate;
            break;
        }
    }

    const auto report = [](const Reply& reply) {
        if (reply.isError())
            std::fprintf(stderr, "Cannot add the network: %s\n", reply.error().message.c_str());
    };

    // With no adapter to join it on now, the profile waits for one.
    if (device.empty()) {
        m_client->call(settingsPath, settingsInterface, "AddConnection", {Variant(settings)}, &m_context, report,
                       Client::authorizedTimeout);
        return true;
    }

    // A hidden network answers only probes that name it.
    VariantMap options{{"ssids", Variant(std::vector<std::vector<uint8_t>>{std::vector<uint8_t>(ssid.begin(), ssid.end())})}};
    m_client->call(device, wirelessInterface, "RequestScan", {Variant(options)}, &m_context);
    m_client->call(managerPath, managerInterface, "AddAndActivateConnection",
                   {Variant(settings), path(device), path({})}, &m_context, report, Client::authorizedTimeout);
    return true;
}

std::optional<WirelessNetwork> Handler::wirelessNetwork(const std::string& accessPoint) const
{
    const std::string device = m_client->accessPointDevice(accessPoint);
    if (device.empty())
        return std::nullopt;
    return WirelessNetwork{device, wifi::ssidLabel(toBytes(m_client->property(accessPoint, accessPointInterface, "Ssid"))),
                           accessPointSecurity(device, accessPoint)};
}

void Handler::withScannedAccessPoint(const std::string& accessPoint, std::function<void(const std::string&)> join)
{
    const std::string scanned = m_client->scannedAccessPoint(accessPoint);
    if (!scanned.empty()) {
        join(scanned);
        return;
    }
    if (!m_client->remembers(accessPoint))
        return;

    // The device and properties outlive the remembered access point, which
    // goes as soon as a scan lists its network again.
    const std::string device = m_client->accessPointDevice(accessPoint);
    m_pendingJoins.push_back({accessPoint, device, m_client->properties(accessPoint, accessPointInterface), std::move(join)});
    requestScan(toString(m_client->property(device, deviceInterface, "Interface")));
}

void Handler::joinScanned()
{
    for (size_t i = 0; i < m_pendingJoins.size();) {
        const PendingJoin& pending = m_pendingJoins[i];
        const std::string scanned = m_client->listedAccessPoint(pending.device, pending.properties);
        if (scanned.empty() && m_client->remembers(pending.accessPoint)) {
            ++i;
            continue;
        }
        const PendingJoin joined = pending;
        m_pendingJoins.erase(m_pendingJoins.begin() + std::ptrdiff_t(i));
        if (!scanned.empty())
            joined.join(scanned);
    }
}

SecurityType Handler::accessPointSecurity(const std::string& device, const std::string& accessPoint) const
{
    const auto ap = [&](const char* name) { return toUInt(m_client->property(accessPoint, accessPointInterface, name)); };
    return wifi::security(toUInt(m_client->property(device, wirelessInterface, "WirelessCapabilities")), ap("Flags"),
                          ap("WpaFlags"), ap("RsnFlags"), ap("Mode"));
}

void Handler::addAndActivate(const Settings& settings, const std::string& device, const std::string& specificObject)
{
    m_client->call(managerPath, managerInterface, "AddAndActivateConnection",
                   {Variant(settings), path(device), path(specificObject)}, &m_context,
                   [this](const Reply& reply) {
                       if (reply.isError()) {
                           std::fprintf(stderr, "Cannot join the network: %s\n", reply.error().message.c_str());
                           return;
                       }
                       forgetUnlessActivated(replyPath(reply, 0), replyPath(reply, 1));
                   },
                   Client::authorizedTimeout);
}

void Handler::forgetUnlessActivated(const std::string& connection, const std::string& active)
{
    // A wrong password the user then gave up on would otherwise stay behind
    // as a known network. The active connection arrived before this reply did.
    if (connection.empty())
        return;
    if (active.empty() || !m_client->has(active, activeInterface)) {
        removeConnection(connection);
        return;
    }

    m_forget[active] = connection;
    activeStateChanged(active, toUInt(m_client->property(active, activeInterface, "State")));
    // An activation NetworkManager drops without a final state is let go of.
    auto timer = std::make_unique<base::Timer>();
    base::Timer* raw = timer.get();
    timer->setSingleShot(true);
    timer->onTimeout = [this, active, raw] {
        m_forget.erase(active);
        std::erase_if(m_forgetTimers, [raw](const std::unique_ptr<base::Timer>& timer) { return timer.get() == raw; });
    };
    timer->start(kForgetWatchTimeout);
    m_forgetTimers.push_back(std::move(timer));
}

void Handler::activeStateChanged(const std::string& active, uint32_t state)
{
    if (!m_hotspotActivation.empty() && active == m_hotspotActivation) {
        if (state == ActiveDeactivated) {
            m_hotspotActivation.clear();
            hotspotFailed.emit(tr("The hotspot could not be started. The Wi-Fi adapter may not support it on this "
                                  "channel or with this password."));
        } else if (state == ActiveActivated) {
            m_hotspotActivation.clear();
        }
    }

    const auto it = m_forget.find(active);
    if (it == m_forget.end())
        return;

    if (state == ActiveDeactivated) {
        const std::string connection = it->second;
        m_forget.erase(it);
        removeConnection(connection);
    } else if (state == ActiveActivated) {
        m_forget.erase(it);
    }
}

void Handler::reportVpnFailure(const std::string& active, uint32_t reason)
{
    if (!toBool(m_client->property(active, activeInterface, "Vpn")))
        return;
    // A lost uplink (DEVICE_DISCONNECTED) takes the VPN down without a failure of its own.
    if (reason == 3)
        return;
    const std::string message = vpn::failureMessage(reason);
    if (message.empty())
        return;
    vpnFailed.emit(toString(m_client->property(active, activeInterface, "Id")), message);
}

void Handler::deactivateConnection(const std::string& connection, const std::string& device)
{
    const auto takeDown = [this](const std::string& active) {
        m_client->call(managerPath, managerInterface, "DeactivateConnection", {path(active)}, &m_context, {},
                       Client::authorizedTimeout);
    };
    // Unlike taking the connection down, this keeps the device from connecting again by itself.
    const auto disconnect = [this](const std::string& device) {
        m_client->call(device, deviceInterface, "Disconnect", {}, &m_context, {}, Client::authorizedTimeout);
    };

    for (const std::string& active : m_client->activeConnections()) {
        const std::vector<std::string> devices = toStringList(m_client->property(active, activeInterface, "Devices"));
        const bool onDevice = contains(devices, device);
        const std::string profile = objectPath(m_client->property(active, activeInterface, "Connection"));
        if (connection.empty() ? !onDevice : profile != connection)
            continue;

        // A VPN rides on another device's connection, and a profile named
        // without a device (WireGuard's) has an interface of its own.
        if (device.empty() || toBool(m_client->property(active, activeInterface, "Vpn")))
            takeDown(active);
        else if (onDevice)
            disconnect(device);
    }
}

void Handler::removeConnection(const std::string& connection)
{
    const auto remove = [this](const std::string& profile) {
        m_client->call(profile, connectionInterface, "Delete", {}, &m_context, {}, Client::authorizedTimeout);
    };

    // A bond's or bridge's ports name it as their controller ("master" in
    // older NetworkManager) and would be left without it.
    const std::string uuid = toString(m_client->setting(connection, "connection", "uuid"));
    const auto isPort = [this, &uuid](const std::string& profile) {
        return toString(m_client->setting(profile, "connection", "controller")) == uuid
               || toString(m_client->setting(profile, "connection", "master")) == uuid;
    };
    if (!uuid.empty()) {
        for (const std::string& profile : m_client->connections()) {
            if (isPort(profile))
                remove(profile);
        }
    }

    remove(connection);
}

void Handler::requestScan(const std::string& interface)
{
    if (!m_client->ready()) {
        m_scanRequested = true;
        m_scanInterface = interface;
        return;
    }
    if (!toBool(m_client->manager("WirelessEnabled")))
        return;

    // NetworkManager rate-limits and defers scans itself. It refuses only
    // devices that cannot scan, and a scan on one running a hotspot would drop
    // the hotspot's clients.
    const auto scans = [this, &interface](const std::string& device) {
        if (!interface.empty() && toString(m_client->property(device, deviceInterface, "Interface")) != interface)
            return false;
        return m_client->deviceType(device) == DeviceWifi && m_client->deviceState(device) >= DeviceDisconnected
               && toUInt(m_client->property(device, wirelessInterface, "Mode")) != ModeAp;
    };
    for (const std::string& device : m_client->devices()) {
        if (scans(device))
            m_client->call(device, wirelessInterface, "RequestScan", {Variant(VariantMap())}, &m_context);
    }
}

void Handler::enableWireless(bool enable, std::function<void(const base::dbus::Reply&)> completion)
{
    m_client->setProperty(managerPath, managerInterface, "WirelessEnabled", Variant(enable), &m_context,
                          std::move(completion));
}

void Handler::enableWired(bool enable)
{
    for (const std::string& device : m_client->devices()) {
        if (m_client->deviceType(device) != DeviceEthernet
            || !toBool(m_client->property(device, deviceInterface, "Managed")))
            continue;

        // Disconnecting clears the device's autoconnect flag and activating
        // sets it again, so the flag is written only when neither applies;
        // writing it next to either races the other.
        const uint32_t state = m_client->deviceState(device);
        if (!enable && state >= DevicePrepare && state <= DeviceActivated) {
            m_client->call(device, deviceInterface, "Disconnect", {}, &m_context, {}, Client::authorizedTimeout);
        } else if (enable && state == DeviceDisconnected) {
            // "/" lets NetworkManager pick the device's best connection, even one that doesn't autoconnect.
            activateConnection({}, device, {});
        } else {
            m_client->setProperty(device, deviceInterface, "Autoconnect", Variant(enable), &m_context);
        }
    }
}

void Handler::createHotspot()
{
    uint32_t capabilities = 0;
    const std::string device = hotspotDevice(&capabilities);
    if (device.empty()) {
        hotspotFailed.emit(tr("No Wi-Fi adapter can run a hotspot."));
        return;
    }

    const HotspotConfiguration* configuration = HotspotConfiguration::instance();
    const std::string existing = m_client->hotspotConnection();
    const std::string uuid = existing.empty() ? text::newUuid()
                                              : toString(m_client->setting(existing, "connection", "uuid"));
    const Settings settings = hotspotSettings(configuration->name(), uuid, configuration->password(), capabilities);

    // One profile, kept and updated, rather than a new one each time.
    if (existing.empty()) {
        m_client->call(managerPath, managerInterface, "AddAndActivateConnection",
                       {Variant(settings), path(device), path({})}, &m_context,
                       [this](const Reply& reply) { hotspotRequestFinished(reply, 1); }, Client::authorizedTimeout);
        return;
    }
    m_client->call(existing, connectionInterface, "Update", {Variant(settings)}, &m_context,
                   [this, existing, device](const Reply& reply) {
                       if (reply.isError()) {
                           hotspotRequestFinished(reply, 0);
                           return;
                       }
                       m_client->call(managerPath, managerInterface, "ActivateConnection",
                                      {path(existing), path(device), path({})}, &m_context,
                                      [this](const Reply& reply) { hotspotRequestFinished(reply, 0); },
                                      Client::authorizedTimeout);
                   },
                   Client::authorizedTimeout);
}

void Handler::hotspotRequestFinished(const Reply& reply, size_t activePathArgument)
{
    if (reply.isError()) {
        // Cancelling the administrator password prompt is no failure to report.
        if (reply.error().name != "org.freedesktop.NetworkManager.PermissionDenied")
            hotspotFailed.emit(reply.error().message);
        return;
    }

    const std::string active = replyPath(reply, activePathArgument);
    const uint32_t state = toUInt(m_client->property(active, activeInterface, "State"));
    if (state == ActiveActivated)
        return;
    m_hotspotActivation = active;
    // Its state may have changed, or NetworkManager removed it, before this reply arrived.
    const bool gone = m_client->ready() && !m_client->has(active, activeInterface);
    if (gone || state == ActiveDeactivated || state == ActiveDeactivating)
        activeStateChanged(active, ActiveDeactivated);
}

void Handler::stopHotspot()
{
    m_hotspotActivation.clear();
    const std::string active = hotspotActiveConnection();
    if (active.empty())
        return;
    m_client->call(managerPath, managerInterface, "DeactivateConnection", {path(active)}, &m_context, {},
                   Client::authorizedTimeout);
}

void Handler::runNmcli(const std::vector<std::string>& arguments,
                       std::function<void(bool started, bool succeeded, const std::string& output,
                                          const std::string& errorOutput)>
                           done)
{
    // Finished ones go here, not from their own onFinished.
    std::erase_if(m_processes, [](const std::unique_ptr<base::Process>& process) { return !process->isRunning(); });
    auto process = std::make_unique<base::Process>();
    base::Process* started = process.get();
    started->onFinished = [started, done](int exitCode, bool crashed) {
        // Copies: done may start nmcli again, which lets this process go.
        const std::string output = started->standardOutput();
        const std::string errorOutput = started->standardError();
        done(true, !crashed && exitCode == 0, output, errorOutput);
    };
    if (!started->start("nmcli", arguments)) {
        done(false, false, {}, {});
        return;
    }
    m_processes.push_back(std::move(process));
}

void Handler::importVpn(const std::string& file)
{
    const std::string fileName = std::filesystem::path(file).filename().string();
    std::error_code error;
    const auto size = std::filesystem::file_size(file, error);
    if (!error && size < 1024 * 1024) {
        if (const auto content = base::file::read(file); content && vpn::isWireGuardConfig(*content)) {
            // The complete base name: everything before the last dot.
            const size_t dot = fileName.rfind('.');
            const std::string name = dot == std::string::npos || dot == 0 ? fileName : fileName.substr(0, dot);
            std::string message;
            const auto profile = vpn::wireGuardProfile(*content, name, &message);
            if (!profile)
                vpnAddFailed.emit(message);
            else
                addPrivateVpn(*profile);
            return;
        }
    }

    // NetworkManager's own readers, the ones nmcli and both big desktops use;
    // whichever reads the file first wins.
    std::vector<std::string> plugins = vpn::installedPlugins();
    const size_t dot = fileName.rfind('.');
    const std::string suffix = dot == std::string::npos ? std::string() : text::lower(fileName.substr(dot + 1));
    const std::string likely = suffix == "ovpn" || suffix == "conf" ? "openvpn" : suffix == "pcf" ? "vpnc" : "";
    const auto found = std::find(plugins.begin(), plugins.end(), likely);
    if (!likely.empty() && found != plugins.end()) {
        plugins.erase(found);
        plugins.insert(plugins.begin(), likely);
    }

    if (plugins.empty()) {
        vpnAddFailed.emit(tr("No VPN plugin can read this file. Install the one for your VPN, for example "
                             "network-manager-openvpn for OpenVPN."));
        return;
    }
    importVpnAs(file, plugins, {});
}

void Handler::importVpnAs(const std::string& file, std::vector<std::string> types, const std::string& error)
{
    if (types.empty()) {
        vpnAddFailed.emit(error.empty() ? std::string(tr("This file isn't a VPN configuration this system can read."))
                                        : error);
        return;
    }

    const std::string type = types.front();
    types.erase(types.begin());

    // A plugin's reader runs in nmcli: NetworkManager offers no way to call it over D-Bus.
    runNmcli({"connection", "import", "type", type, "file", file},
             [this, file, types, error](bool started, bool succeeded, const std::string& output,
                                        const std::string& errorOutput) {
                 if (!started) {
                     vpnAddFailed.emit(tr("nmcli, which reads VPN files, is not installed."));
                     return;
                 }
                 const std::string message = text::trimmed(errorOutput);
                 if (succeeded) {
                     std::smatch uuid;
                     static const std::regex pattern("[0-9a-f]{8}(-[0-9a-f]{4}){3}-[0-9a-f]{12}");
                     std::regex_search(output, uuid, pattern);
                     vpnAdded.emit(uuid.empty() ? std::string() : uuid.str());
                     return;
                 }
                 importVpnAs(file, types, message.empty() ? error : message);
             });
}

std::vector<vpn::Kind> Handler::vpnKinds() const
{
    return vpn::manualKinds();
}

VpnForm Handler::vpnForm(const std::string& kind, const std::string& connection) const
{
    Settings settings;
    std::string shortName = kind;
    if (!connection.empty()) {
        settings = m_client->settings(connection);
        shortName = vpn::kind(*m_client, connection);
    }
    const bool wireguard = shortName == "wireguard";
    const VariantMap info = settings.count("connection") ? settings.at("connection") : VariantMap();
    const auto never = [&settings](const char* family) {
        const auto found = settings.find(family);
        return found != settings.end() && toBool(value(found->second, "never-default"));
    };

    VpnForm form;
    form.kind = shortName;
    form.label = vpn::kindLabel(shortName);
    form.installed = wireguard || vpn::pluginInstalled("org.freedesktop.NetworkManager." + shortName);
    form.package = vpn::package(shortName);
    form.name = toString(value(info, "id"));
    form.shared = !connection.empty() && toStringList(value(info, "permissions")).empty();
    form.ownNetworkOnly = never("ipv4") || never("ipv6");
    form.exportable = wireguard || vpn::schema::exportable(shortName);
    form.exportPatterns = wireguard ? std::vector<std::string>{"*.conf"} : vpn::schema::importPatterns(shortName);
    if (wireguard) {
        form.wireguard = vpn::wireGuardForm(settings);
    } else {
        const auto vpnGroup = settings.find("vpn");
        const std::map<std::string, std::string> data =
            vpnGroup == settings.end() ? std::map<std::string, std::string>() : toStringMap(value(vpnGroup->second, "data"));
        form.fields = vpn::schema::formFields(shortName, data);
    }
    return form;
}

void Handler::getSecrets(const std::string& connection, const std::string& setting,
                         std::function<void(const Settings&)> done)
{
    m_client->call(connection, connectionInterface, "GetSecrets", {Variant(setting)}, &m_context,
                   [done](const Reply& reply) {
                       // A profile without secrets of that kind, or one this user may
                       // not read them from, gives an error: the form then starts empty.
                       done(reply.value<Settings>().value_or(Settings()));
                   },
                   Client::authorizedTimeout);
}

void Handler::loadVpnSecrets(const std::string& connection)
{
    const std::string uuid = toString(m_client->setting(connection, "connection", "uuid"));
    if (vpn::kind(*m_client, connection) == "wireguard") {
        getSecrets(connection, "wireguard", [this, connection](const Settings& secrets) {
            VpnSecrets loaded;
            const auto wireguard = secrets.find("wireguard");
            if (wireguard != secrets.end()) {
                for (const VariantMap& peer : toMapList(value(wireguard->second, "peers"))) {
                    if (variants::contains(peer, "preshared-key"))
                        loaded.presharedKeys[toString(value(peer, "public-key"))] = toString(value(peer, "preshared-key"));
                }
                loaded.privateKey = toString(value(wireguard->second, "private-key"));
            }
            vpnSecretsLoaded.emit(connection, loaded);
        });
        return;
    }

    // System-owned secrets come from NetworkManager, this user's from the keyring.
    getSecrets(connection, "vpn", [this, connection, uuid](const Settings& secrets) {
        const auto vpnGroup = secrets.find("vpn");
        const std::map<std::string, std::string> system =
            vpnGroup == secrets.end() ? std::map<std::string, std::string>() : toStringMap(value(vpnGroup->second, "secrets"));
        if (!m_secrets)
            m_secrets = std::make_unique<SecretReader>();
        m_secrets->load(uuid, "vpn", [this, connection, system](const std::map<std::string, std::string>& stored) {
            VpnSecrets loaded;
            loaded.values = system;
            for (const auto& [key, secret] : stored)
                loaded.values[key] = secret;
            vpnSecretsLoaded.emit(connection, loaded);
        });
    });
}

void Handler::saveVpn(const std::string& connection, const VpnValues& form)
{
    const bool adding = connection.empty();
    const auto fail = [this, adding](const std::string& message) {
        if (adding)
            vpnAddFailed.emit(message);
        else
            vpnSaveFailed.emit(message);
    };

    Settings profile = adding ? Settings() : m_client->settings(connection);
    const std::string kind = adding ? form.kind : vpn::kind(*m_client, connection);
    const bool shared = form.shared;
    const std::string name = text::trimmed(form.name);
    std::string error;

    if (kind == "wireguard") {
        vpn::WireGuardForm wireguard = form.wireguard;
        wireguard.name = form.name;
        const auto settings = vpn::wireGuardSettings(wireguard, profile, &error);
        if (!settings)
            return fail(error);
        profile = *settings;
    } else {
        std::map<std::string, std::string> data = toStringMap(value(profile["vpn"], "data"));
        std::map<std::string, std::string> secrets;
        // A shared profile's passwords are NetworkManager's to keep, a private
        // one's this user's: a storage choice from the other arrangement moves over.
        std::map<std::string, uint32_t> storage = form.storage;
        for (auto& [key, flags] : storage) {
            if (shared && flags == SecretAgentOwned)
                flags = SecretSystemOwned;
            else if (!shared && flags == SecretSystemOwned)
                flags = SecretAgentOwned;
        }
        if (!vpn::schema::apply(kind, form.values, storage, form.secrets, shared, data, secrets, &error))
            return fail(error);

        VariantMap info = profile["connection"];
        if (adding) {
            info["uuid"] = Variant(text::newUuid());
            info["type"] = Variant(std::string("vpn"));
            // Named after the server, as the file imports are named after their file.
            std::string fallback;
            for (const vpn::schema::Field& field : vpn::schema::fields(kind)) {
                if (field.required && fallback.empty()) {
                    const auto found = data.find(field.key);
                    fallback = found == data.end() ? std::string() : found->second;
                }
            }
            info["id"] = Variant(fallback.empty() ? vpn::kindLabel(kind) : fallback);
        }
        profile["connection"] = info;

        VariantMap& vpn = profile["vpn"];
        vpn["service-type"] = Variant("org.freedesktop.NetworkManager." + kind);
        vpn["data"] = Variant(data);
        // Without secrets in the update, NetworkManager keeps the stored ones.
        if (secrets.empty())
            vpn.erase("secrets");
        else
            vpn["secrets"] = Variant(secrets);
        for (const char* family : {"ipv4", "ipv6"}) {
            if (!variants::contains(profile[family], "method"))
                profile[family]["method"] = Variant(std::string("auto"));
        }
    }

    if (!name.empty())
        profile["connection"]["id"] = Variant(name);
    profile["connection"]["permissions"] =
        Variant(shared ? std::vector<std::string>() : std::vector<std::string>{Client::userPermission()});
    for (const char* family : {"ipv4", "ipv6"}) {
        if (form.ownNetworkOnly)
            profile[family]["never-default"] = Variant(true);
        else
            profile[family].erase("never-default");
    }

    if (adding) {
        addPrivateVpn(profile);
        return;
    }
    m_client->call(connection, connectionInterface, "Update", {Variant(profile)}, &m_context,
                   [this, connection](const Reply& reply) {
                       if (reply.isError())
                           vpnSaveFailed.emit(reply.error().message);
                       else
                           vpnSaved.emit(connection);
                   },
                   Client::authorizedTimeout);
}

void Handler::exportVpn(const std::string& connection, const std::string& file)
{
    if (vpn::kind(*m_client, connection) == "wireguard") {
        getSecrets(connection, "wireguard", [this, connection, file](const Settings& secrets) {
            Settings settings = m_client->settings(connection);
            const auto wireguard = secrets.find("wireguard");
            if (wireguard == secrets.end() || !variants::contains(wireguard->second, "private-key")) {
                vpnExportFailed.emit(tr("The private key can't be read, so the file would be incomplete."));
                return;
            }
            settings["wireguard"]["private-key"] = value(wireguard->second, "private-key");
            // Peers come with their preshared keys only here.
            if (variants::contains(wireguard->second, "peers"))
                settings["wireguard"]["peers"] = value(wireguard->second, "peers");

            // The file holds the private key.
            if (!base::file::writeAtomically(file, vpn::wireGuardConfig(settings))) {
                vpnExportFailed.emit(tr("The file can't be written."));
                return;
            }
            chmod(file.c_str(), S_IRUSR | S_IWUSR);
            vpnExported.emit(file);
        });
        return;
    }

    // The plugin's writer runs in nmcli, like its reader.
    const std::string uuid = toString(m_client->setting(connection, "connection", "uuid"));
    runNmcli({"connection", "export", "uuid", uuid, file},
             [this, file](bool started, bool succeeded, const std::string&, const std::string& errorOutput) {
                 if (!started) {
                     vpnExportFailed.emit(tr("nmcli, which writes VPN files, is not installed."));
                     return;
                 }
                 const std::string message = text::trimmed(errorOutput);
                 if (succeeded)
                     vpnExported.emit(file);
                 else
                     vpnExportFailed.emit(message.empty() ? std::string(tr("The VPN couldn't be written to a file."))
                                                          : message);
             });
}

ip::Form Handler::ipForm(const std::string& connection) const
{
    return ip::form(m_client->settings(connection));
}

ip::Form Handler::currentAddresses(const std::string& connection) const
{
    return ip::currentAddresses(*m_client, connection);
}

void Handler::saveIp(const std::string& connection, const ip::Form& form)
{
    bool changed = false;
    std::string error;
    const auto profile = ip::settings(form, m_client->settings(connection), &changed, &error);
    if (!profile) {
        ipSaveFailed.emit(connection, error);
        return;
    }
    if (!changed) {
        ipSaved.emit(connection);
        return;
    }

    // Without secrets in it, an update keeps the ones NetworkManager and the keyring hold.
    m_client->call(connection, connectionInterface, "Update", {Variant(*profile)}, &m_context,
                   [this, connection](const Reply& reply) {
                       if (reply.isError()) {
                           ipSaveFailed.emit(connection, reply.error().message);
                           return;
                       }
                       ipSaved.emit(connection);
                       reapply(connection);
                   },
                   Client::authorizedTimeout);
}

// As GNOME's editor does after saving: the device takes the new addressing at once.
void Handler::reapply(const std::string& connection)
{
    const std::string active = vpn::activeConnection(*m_client, connection);
    const std::vector<std::string> devices = toStringList(m_client->property(active, activeInterface, "Devices"));
    if (devices.empty())
        return;

    const std::string device = devices.front();
    // An empty profile reapplies the saved one; version 0 skips the version check.
    m_client->call(device, deviceInterface, "Reapply", {Variant(Settings()), Variant(uint64_t(0)), Variant(uint32_t(0))},
                   &m_context,
                   [this, connection, device](const Reply& reply) {
                       if (!reply.isError() || reply.error().name == "org.freedesktop.NetworkManager.Device.NotActive")
                           return;
                       // What the device can't take while up, it takes on connecting again.
                       std::fprintf(stderr, "Cannot reapply the network settings: %s\n", reply.error().message.c_str());
                       activateConnection(connection, device, {});
                   },
                   Client::authorizedTimeout);
}

void Handler::addPrivateVpn(Settings profile)
{
    // As the big desktops do: the VPN is this user's unless the form says
    // otherwise, which needs no administrator password, and comes up only when
    // asked to. A WireGuard profile would otherwise connect the moment it is
    // added, its interface being its own.
    profile["connection"]["autoconnect"] = Variant(false);
    if (!variants::contains(profile["connection"], "permissions"))
        profile["connection"]["permissions"] = Variant(std::vector<std::string>{Client::userPermission()});

    const std::string uuid = toString(value(profile["connection"], "uuid"));
    m_client->call(settingsPath, settingsInterface, "AddConnection", {Variant(profile)}, &m_context,
                   [this, uuid](const Reply& reply) {
                       if (reply.isError())
                           vpnAddFailed.emit(reply.error().message);
                       else
                           vpnAdded.emit(uuid);
                   },
                   Client::authorizedTimeout);
}

std::string Handler::hotspotActiveConnection() const
{
    const std::string hotspot = m_client->hotspotConnection();
    if (hotspot.empty())
        return {};
    for (const std::string& active : m_client->activeConnections()) {
        const uint32_t state = toUInt(m_client->property(active, activeInterface, "State"));
        if (objectPath(m_client->property(active, activeInterface, "Connection")) == hotspot
            && (state == ActiveActivating || state == ActiveActivated))
            return active;
    }
    return {};
}

std::string Handler::hotspotDevice(uint32_t* capabilities) const
{
    // An idle adapter that can run an access point is best; one in use gets disconnected.
    std::string device;
    uint32_t deviceCapabilities = 0;
    for (const std::string& candidate : m_client->devices()) {
        if (m_client->deviceType(candidate) != DeviceWifi
            || !toBool(m_client->property(candidate, deviceInterface, "Managed")))
            continue;
        const uint32_t caps = toUInt(m_client->property(candidate, wirelessInterface, "WirelessCapabilities"));
        if (!(caps & (CapAp | CapAdhoc)))
            continue;
        const bool idle = m_client->activeConnection(candidate).empty();
        const bool better = device.empty() || ((caps & CapAp) && !(deviceCapabilities & CapAp))
                            || (bool(caps & CapAp) == bool(deviceCapabilities & CapAp) && idle
                                && !m_client->activeConnection(device).empty());
        if (better) {
            device = candidate;
            deviceCapabilities = caps;
        }
    }
    if (capabilities)
        *capabilities = deviceCapabilities;
    return device;
}

void Handler::update()
{
    joinScanned();

    const std::string device = hotspotDevice();
    const bool capable = !device.empty();

    // Polkit answers "auth" when an administrator password would do; an
    // unread answer doesn't block either.
    const std::string permission = m_client->permission("org.freedesktop.NetworkManager.wifi.share.protected");
    const bool allowed = toBool(m_client->manager("WirelessEnabled")) && permission != "no";

    // The network this adapter is on goes when the hotspot takes it over.
    std::string replaces;
    const std::string active = capable ? m_client->activeConnection(device) : std::string();
    if (!active.empty()
        && objectPath(m_client->property(active, activeInterface, "Connection")) != m_client->hotspotConnection())
        replaces = toString(m_client->property(active, activeInterface, "Id"));

    if (capable != m_hotspotCapable || allowed != m_hotspotAllowed || replaces != m_hotspotReplaces) {
        m_hotspotCapable = capable;
        m_hotspotAllowed = allowed;
        m_hotspotReplaces = replaces;
        hotspotSupportedChanged.emit();
    }

    const bool hotspot = !hotspotActiveConnection().empty();
    if (hotspot != m_hotspotActive) {
        m_hotspotActive = hotspot;
        hotspotActiveChanged.emit();
    }
}

} // namespace Lingmo::Networkmanager
