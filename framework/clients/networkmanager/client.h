// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "base/timer.h"
#include "lingmo/networkmanager/types.h"

namespace Lingmo::Networkmanager {

// NetworkManager as one GetManagedObjects and the signals since describe it,
// shared by everything in the process that shows or changes networks. Nothing
// here blocks: every call is asynchronous, and the cache is only ever read.
class Client
{
public:
    using Variant = base::dbus::Variant;
    using VariantMap = base::dbus::VariantMap;
    using Completion = std::function<void(const base::dbus::Reply& reply)>;

    explicit Client(base::dbus::Connection* bus);
    ~Client();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    // The system bus's client of the calling thread's loop, made on first use
    // and gone with the loop.
    static Client* instance();
    // Makes instance() a client of this bus from now on, for tests that serve
    // NetworkManager on a bus of their own; the client made so far goes.
    static void setInstanceBus(base::dbus::Connection* bus);

    base::dbus::Connection* bus() const { return m_bus; }
    // NetworkManager's unique bus name; empty while it is not running.
    const std::string& owner() const { return m_owner; }
    // The objects and the settings of every profile have arrived.
    bool ready() const { return m_ready; }

    bool has(const std::string& path, const std::string& interface) const;
    VariantMap properties(const std::string& path, const std::string& interface) const;
    Variant property(const std::string& path, const std::string& interface, const std::string& name) const;
    Variant manager(const std::string& name) const;

    std::vector<std::string> devices() const;
    uint32_t deviceType(const std::string& device) const;
    uint32_t deviceState(const std::string& device) const;
    std::string activeConnection(const std::string& device) const;
    std::vector<std::string> activeConnections() const;
    std::vector<std::string> accessPoints(const std::string& device) const;
    // Access points the device no longer lists only because nothing scanned
    // for a while; their properties stay readable through property().
    std::vector<std::string> rememberedAccessPoints(const std::string& device) const;
    bool remembers(const std::string& accessPoint) const;
    // The Wi-Fi device that lists the access point, or listed a remembered one.
    std::string accessPointDevice(const std::string& accessPoint) const;
    // `accessPoint` if NetworkManager lists it; for a remembered one, the same
    // network's access point once a scan lists it again, empty until then.
    std::string scannedAccessPoint(const std::string& accessPoint) const;
    // What `device` lists for the access point with these properties: the
    // same BSSID, else the strongest with the same name and security.
    std::string listedAccessPoint(const std::string& device, const VariantMap& accessPoint) const;

    // The profiles this user may read.
    std::vector<std::string> connections() const;
    bool readable(const std::string& connection) const;
    // As NetworkManager sent them, so an Update hands back the D-Bus types it was given.
    Settings settings(const std::string& connection) const;
    Variant setting(const std::string& connection, const std::string& setting, const std::string& key) const;
    // The profile that shares this machine's connection over Wi-Fi.
    std::string hotspotConnection() const;

    std::string permission(const std::string& name) const;
    // New profiles are for every user when this user may change system
    // profiles without an administrator password; otherwise a
    // non-administrator could not join at all.
    bool sharesNewConnections() const;
    // This user's entry in a profile's connection.permissions.
    static std::string userPermission();

    // For calls that may wait on an administrator password: they may prompt
    // for it and wait that long for the answer.
    static constexpr std::chrono::milliseconds authorizedTimeout{2 * 60 * 1000};

    // The reply reaches `completion` unless `context` is gone by then.
    void call(const std::string& path, const std::string& interface, const std::string& method,
              Variant::Items arguments = {}, const CallContext* context = nullptr, Completion completion = {},
              std::chrono::milliseconds timeout = {});
    void setProperty(const std::string& path, const std::string& interface, const std::string& name,
                     const Variant& value, const CallContext* context = nullptr, Completion completion = {});

    base::Signal<> readyChanged;
    // Once per batch of applied changes.
    base::Signal<> changed;
    base::Signal<const std::string&, uint32_t, uint32_t> activeStateChanged;
    base::Signal<const std::string&> objectRemoved;

private:
    struct RememberedAccessPoint
    {
        std::string device;
        VariantMap properties;
        int missedScans = 0;
    };

    void refresh();
    void reset();
    void load(const std::string& owner);
    void subscribe(const std::string& owner);
    void addInterfaces(const std::string& path, const std::map<std::string, VariantMap>& interfaces);
    void interfacesAdded(const base::dbus::Message& message);
    void interfacesRemoved(const base::dbus::Message& message);
    void propertiesChanged(const base::dbus::Message& message);
    void connectionUpdated(const base::dbus::Message& message);
    void activeStateSignal(const base::dbus::Message& message);
    void fetchSettings(const std::string& path);
    void fetchPermissions();
    void scheduleChanged();
    void updateReady();
    bool keepsAccessPoints(const std::string& device) const;
    void noteAccessPoints(const std::string& device);
    void rememberAccessPoint(const std::string& path, const VariantMap& properties);
    void scanFinished(const std::string& device);
    void pruneRemembered();

    base::dbus::Connection* m_bus;
    CallContext m_context;
    std::string m_owner;
    uint64_t m_generation = 0;
    bool m_loading = false;
    bool m_loaded = false;
    bool m_ready = false;
    std::map<std::string, std::map<std::string, VariantMap>> m_objects;
    std::map<std::string, Settings> m_settings;
    // The newest GetSettings per profile; an older reply is stale.
    std::map<std::string, uint64_t> m_settingsRequests;
    uint64_t m_permissionsRequest = 0;
    uint64_t m_nextRequest = 0;
    std::map<std::string, std::string> m_permissions;
    std::map<std::string, std::string> m_accessPointDevices;
    // wpa_supplicant drops an access point 250 s after a scan last saw it,
    // and while connected NetworkManager hardly scans, so a list would open
    // with only the network in use. These stay until two later scans miss
    // them, as NetworkManager drops its own.
    std::map<std::string, RememberedAccessPoint> m_remembered;
    std::unordered_map<uint64_t, base::dbus::PendingCall> m_pending;
    uint64_t m_nextPending = 0;
    std::vector<base::dbus::Match> m_matches;
    base::dbus::NameWatch m_ownerWatch;
    base::Timer m_changedTimer;
    base::Timer m_retryTimer;
};

} // namespace Lingmo::Networkmanager
