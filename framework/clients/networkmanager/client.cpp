// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/networkmanager/client.h"
#include "lingmo/networkmanager/variants.h"

#include <algorithm>
#include <strings.h>

#include <pwd.h>
#include <time.h>
#include <unistd.h>

#include "base/event_loop.h"

namespace Lingmo::Networkmanager {

using namespace std::chrono_literals;
using base::dbus::Error;
using base::dbus::Message;
using base::dbus::ObjectPath;
using base::dbus::Reply;
using namespace variants;

// A scan sees its access points over a few seconds before LastScan, and
// NetworkManager starts scans at least 8 s apart while connected.
static constexpr int64_t kScanSpreadMs = 5000;
// A scan drops the access points it missed as it ends; age drops them 250 s after one.
static constexpr int64_t kAgedOutAfterMs = 60 * 1000;
static constexpr int kMissedScansForgotten = 2;

// LastScan and LastSeen count CLOCK_BOOTTIME, which runs on through suspend.
static int64_t bootTimeMs()
{
    timespec now;
    clock_gettime(CLOCK_BOOTTIME, &now);
    return int64_t(now.tv_sec) * 1000 + now.tv_nsec / 1000000;
}

static bool equalsIgnoringCase(const std::string& a, const std::string& b)
{
    return a.size() == b.size() && strcasecmp(a.c_str(), b.c_str()) == 0;
}

Client::Client(base::dbus::Connection* bus)
    : m_bus(bus)
{
    m_changedTimer.setSingleShot(true);
    m_changedTimer.setInterval(0ms);
    m_changedTimer.onTimeout = [this] {
        pruneRemembered();
        changed.emit();
    };

    m_retryTimer.setSingleShot(true);
    m_retryTimer.setInterval(2s);
    m_retryTimer.onTimeout = [this] { refresh(); };

    if (!m_bus)
        return;
    // Answers at once with the current owner, then on every change.
    m_ownerWatch = m_bus->watchName(service, [this](const std::string& owner) {
        reset();
        if (!owner.empty())
            load(owner);
    });
}

Client::~Client() = default;

static thread_local std::unique_ptr<Client> sharedClient;
static thread_local base::dbus::Connection* instanceBus = nullptr;

Client* Client::instance()
{
    if (!sharedClient) {
        base::EventLoop* loop = base::EventLoop::current();
        sharedClient = std::make_unique<Client>(instanceBus ? instanceBus : base::dbus::Connection::system(loop));
        // Before the loop closes the bus the client's calls and matches live on.
        if (loop)
            loop->addDestroyHook([] { sharedClient.reset(); });
    }
    return sharedClient.get();
}

void Client::setInstanceBus(base::dbus::Connection* bus)
{
    instanceBus = bus;
    sharedClient.reset();
}

bool Client::has(const std::string& path, const std::string& interface) const
{
    const auto object = m_objects.find(path);
    return object != m_objects.end() && object->second.count(interface);
}

Client::VariantMap Client::properties(const std::string& path, const std::string& interface) const
{
    const auto object = m_objects.find(path);
    if (object != m_objects.end()) {
        const auto found = object->second.find(interface);
        return found == object->second.end() ? VariantMap() : found->second;
    }
    if (interface == accessPointInterface) {
        const auto remembered = m_remembered.find(path);
        if (remembered != m_remembered.end())
            return remembered->second.properties;
    }
    return {};
}

Client::Variant Client::property(const std::string& path, const std::string& interface, const std::string& name) const
{
    const auto object = m_objects.find(path);
    if (object != m_objects.end()) {
        const auto found = object->second.find(interface);
        if (found == object->second.end())
            return {};
        return value(found->second, name);
    }
    if (interface == accessPointInterface) {
        const auto remembered = m_remembered.find(path);
        if (remembered != m_remembered.end())
            return value(remembered->second.properties, name);
    }
    return {};
}

Client::Variant Client::manager(const std::string& name) const
{
    return property(managerPath, managerInterface, name);
}

std::vector<std::string> Client::devices() const
{
    return toStringList(manager("Devices"));
}

uint32_t Client::deviceType(const std::string& device) const
{
    return toUInt(property(device, deviceInterface, "DeviceType"));
}

uint32_t Client::deviceState(const std::string& device) const
{
    // StateReason carries the state and its reason together.
    const Variant stateReason = property(device, deviceInterface, "StateReason");
    if (stateReason.signature() == "(uu)" && !stateReason.items().empty())
        return toUInt(stateReason.items().front());
    return toUInt(property(device, deviceInterface, "State"));
}

std::string Client::activeConnection(const std::string& device) const
{
    return objectPath(property(device, deviceInterface, "ActiveConnection"));
}

std::vector<std::string> Client::activeConnections() const
{
    return toStringList(manager("ActiveConnections"));
}

std::vector<std::string> Client::accessPoints(const std::string& device) const
{
    return toStringList(property(device, wirelessInterface, "AccessPoints"));
}

std::vector<std::string> Client::rememberedAccessPoints(const std::string& device) const
{
    // A map keeps them sorted.
    std::vector<std::string> remembered;
    for (const auto& [path, accessPoint] : m_remembered) {
        if (accessPoint.device == device && listedAccessPoint(device, accessPoint.properties).empty())
            remembered.push_back(path);
    }
    return remembered;
}

bool Client::remembers(const std::string& accessPoint) const
{
    return m_remembered.count(accessPoint);
}

std::string Client::accessPointDevice(const std::string& accessPoint) const
{
    const auto remembered = m_remembered.find(accessPoint);
    if (remembered != m_remembered.end())
        return remembered->second.device;
    if (!has(accessPoint, accessPointInterface))
        return {};
    const auto device = m_accessPointDevices.find(accessPoint);
    return device == m_accessPointDevices.end() ? std::string() : device->second;
}

std::string Client::scannedAccessPoint(const std::string& accessPoint) const
{
    if (has(accessPoint, accessPointInterface))
        return accessPoint;
    const auto remembered = m_remembered.find(accessPoint);
    if (remembered == m_remembered.end())
        return {};
    return listedAccessPoint(remembered->second.device, remembered->second.properties);
}

std::string Client::listedAccessPoint(const std::string& device, const VariantMap& accessPoint) const
{
    const std::string bssid = toString(value(accessPoint, "HwAddress"));
    const std::string ssid = toBytes(value(accessPoint, "Ssid"));

    std::string sameNetwork;
    int strongest = -1;
    for (const std::string& path : accessPoints(device)) {
        const auto object = m_objects.find(path);
        if (object == m_objects.end())
            continue;
        const auto found = object->second.find(accessPointInterface);
        if (found == object->second.end())
            continue;
        const VariantMap& listed = found->second;
        if (!bssid.empty() && equalsIgnoringCase(toString(value(listed, "HwAddress")), bssid))
            return path;

        const int strength = toInt(value(listed, "Strength"));
        if (ssid.empty() || toBytes(value(listed, "Ssid")) != ssid || strength <= strongest)
            continue;
        bool sameSecurity = true;
        for (const char* name : {"Mode", "Flags", "WpaFlags", "RsnFlags"})
            sameSecurity = sameSecurity && toUInt(value(listed, name)) == toUInt(value(accessPoint, name));
        if (!sameSecurity)
            continue;
        sameNetwork = path;
        strongest = strength;
    }
    return sameNetwork;
}

std::vector<std::string> Client::connections() const
{
    std::vector<std::string> paths;
    paths.reserve(m_settings.size());
    for (const auto& [path, settings] : m_settings)
        paths.push_back(path);
    return paths;
}

bool Client::readable(const std::string& connection) const
{
    return m_settings.count(connection);
}

Settings Client::settings(const std::string& connection) const
{
    const auto found = m_settings.find(connection);
    return found == m_settings.end() ? Settings() : found->second;
}

Client::Variant Client::setting(const std::string& connection, const std::string& setting, const std::string& key) const
{
    const auto profile = m_settings.find(connection);
    if (profile == m_settings.end())
        return {};
    const auto group = profile->second.find(setting);
    if (group == profile->second.end())
        return {};
    return value(group->second, key);
}

std::string Client::hotspotConnection() const
{
    for (const auto& [connection, settings] : m_settings) {
        const std::string mode = toString(setting(connection, "802-11-wireless", "mode"));
        if ((mode == "ap" || mode == "adhoc") && toString(setting(connection, "ipv4", "method")) == "shared")
            return connection;
    }
    return {};
}

std::string Client::permission(const std::string& name) const
{
    const auto found = m_permissions.find(name);
    return found == m_permissions.end() ? std::string() : found->second;
}

bool Client::sharesNewConnections() const
{
    return permission("org.freedesktop.NetworkManager.settings.modify.system") == "yes";
}

std::string Client::userPermission()
{
    const passwd* pw = ::getpwuid(::getuid());
    return "user:" + std::string(pw ? pw->pw_name : "") + ":";
}

void Client::call(const std::string& path, const std::string& interface, const std::string& method,
                  Variant::Items arguments, const CallContext* context, Completion completion,
                  std::chrono::milliseconds timeout)
{
    if (m_owner.empty()) {
        if (completion)
            completion(Reply({}, Error{"org.freedesktop.DBus.Error.ServiceUnknown", "NetworkManager is not running"}));
        return;
    }

    Message message = m_bus->newMethodCall(m_owner, path, interface, method);
    for (const Variant& argument : arguments)
        message.appendAs(argument);
    if (timeout >= authorizedTimeout)
        message.setAllowInteractiveAuthorization(true);

    const uint64_t id = ++m_nextPending;
    auto finished = std::make_shared<bool>(false);
    std::weak_ptr<char> token = (context ? context : &m_context)->token();
    base::dbus::PendingCall pending = m_bus->callAsync(
        message,
        [this, id, finished, token, completion = std::move(completion)](const Reply& reply) {
            *finished = true;
            // The bus holds the slot until this returns.
            m_pending.erase(id);
            if (completion && !token.expired())
                completion(reply);
        },
        timeout);
    if (!*finished)
        m_pending.emplace(id, std::move(pending));
}

void Client::setProperty(const std::string& path, const std::string& interface, const std::string& name,
                         const Variant& value, const CallContext* context, Completion completion)
{
    call(path, propertiesInterface, "Set",
         {Variant(interface), Variant(name), base::dbus::Type<Variant>::toVariant(value)}, context,
         std::move(completion));
}

void Client::refresh()
{
    if (m_loading || m_loaded || !m_bus)
        return;

    const uint64_t generation = m_generation;
    base::dbus::Proxy daemon(m_bus, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus");
    const uint64_t id = ++m_nextPending;
    auto finished = std::make_shared<bool>(false);
    base::dbus::PendingCall pending = daemon.callAsync(
        "GetNameOwner",
        [this, id, finished, generation](const Reply& reply) {
            *finished = true;
            m_pending.erase(id);
            if (generation != m_generation || m_loading || m_loaded)
                return;
            if (const auto owner = reply.value<std::string>())
                load(*owner);
        },
        service);
    if (!*finished)
        m_pending.emplace(id, std::move(pending));
}

void Client::reset()
{
    ++m_generation;
    m_retryTimer.stop();
    m_matches.clear();
    m_owner.clear();
    m_loading = false;
    m_loaded = false;

    std::vector<std::string> paths;
    for (const auto& [path, object] : m_objects)
        paths.push_back(path);
    m_objects.clear();
    m_settings.clear();
    m_settingsRequests.clear();
    m_permissions.clear();
    m_accessPointDevices.clear();
    m_remembered.clear();

    for (const std::string& path : paths)
        objectRemoved.emit(path);
    if (m_ready) {
        m_ready = false;
        readyChanged.emit();
    }
    scheduleChanged();
}

void Client::load(const std::string& owner)
{
    m_owner = owner;
    m_loading = true;

    // Subscribed before the fetch: a signal that arrives before its reply
    // describes a state the reply already holds, so the handlers ignore
    // everything while loading.
    subscribe(owner);

    const uint64_t generation = m_generation;
    call(rootPath, objectManagerInterface, "GetManagedObjects", {}, nullptr, [this, generation](const Reply& reply) {
        if (generation != m_generation)
            return;
        const auto objects = reply.value<std::map<ObjectPath, std::map<std::string, VariantMap>>>();
        if (!objects) {
            // NetworkManager most likely quit while answering.
            reset();
            m_retryTimer.start();
            return;
        }

        for (const auto& [path, interfaces] : *objects)
            addInterfaces(path.path, interfaces);

        m_loading = false;
        m_loaded = true;
        fetchPermissions();
        updateReady();
        scheduleChanged();
    });
}

void Client::subscribe(const std::string& owner)
{
    const auto bind = [this, &owner](const std::string& path, const std::string& interface, const std::string& member,
                                     std::function<void(const Message&)> handler) {
        base::dbus::MatchRule rule;
        rule.sender = owner;
        rule.path = path;
        rule.interface = interface;
        rule.member = member;
        m_matches.push_back(m_bus->subscribe(rule, std::move(handler)));
    };

    bind(rootPath, objectManagerInterface, "InterfacesAdded", [this](const Message& m) { interfacesAdded(m); });
    bind(rootPath, objectManagerInterface, "InterfacesRemoved", [this](const Message& m) { interfacesRemoved(m); });
    bind({}, propertiesInterface, "PropertiesChanged", [this](const Message& m) { propertiesChanged(m); });
    bind({}, connectionInterface, "Updated", [this](const Message& m) { connectionUpdated(m); });
    // The reason of an active connection's state exists only on this signal.
    bind({}, activeInterface, "StateChanged", [this](const Message& m) { activeStateSignal(m); });
    bind(managerPath, managerInterface, "CheckPermissions", [this](const Message&) { fetchPermissions(); });
}

void Client::addInterfaces(const std::string& path, const std::map<std::string, VariantMap>& interfaces)
{
    auto& object = m_objects[path];
    for (const auto& [interface, added] : interfaces) {
        VariantMap& properties = object[interface];
        for (const auto& [name, property] : added)
            properties[name] = property;
    }

    if (interfaces.count(connectionInterface))
        fetchSettings(path);
    if (interfaces.count(wirelessInterface))
        noteAccessPoints(path);
}

void Client::interfacesAdded(const Message& message)
{
    if (m_loading)
        return;

    ObjectPath path;
    std::map<std::string, VariantMap> interfaces;
    if (!message.read(path, interfaces))
        return;
    addInterfaces(path.path, interfaces);
    scheduleChanged();
}

void Client::interfacesRemoved(const Message& message)
{
    if (m_loading)
        return;

    ObjectPath path;
    std::vector<std::string> interfaces;
    if (!message.read(path, interfaces))
        return;
    const auto object = m_objects.find(path.path);
    if (object == m_objects.end())
        return;

    const auto removes = [&interfaces](const std::string& interface) {
        return std::find(interfaces.begin(), interfaces.end(), interface) != interfaces.end();
    };
    if (removes(accessPointInterface)) {
        const auto found = object->second.find(accessPointInterface);
        rememberAccessPoint(path.path, found == object->second.end() ? VariantMap() : found->second);
    }
    for (const std::string& interface : interfaces)
        object->second.erase(interface);

    if (removes(connectionInterface)) {
        m_settings.erase(path.path);
        m_settingsRequests.erase(path.path);
    }

    if (object->second.empty()) {
        m_objects.erase(object);
        objectRemoved.emit(path.path);
    }
    updateReady();
    scheduleChanged();
}

void Client::propertiesChanged(const Message& message)
{
    std::string interface;
    VariantMap properties;
    // Counters tick for whoever set a refresh rate; nothing here shows them.
    if (m_loading || !message.read(interface, properties) || interface == statisticsInterface)
        return;

    const std::string path = message.path();
    const auto object = m_objects.find(path);
    if (object == m_objects.end())
        return;
    const auto cached = object->second.find(interface);
    if (cached == object->second.end())
        return;

    for (const auto& [name, property] : properties)
        cached->second[name] = property;

    if (interface == wirelessInterface) {
        if (properties.count("AccessPoints"))
            noteAccessPoints(path);
        if (properties.count("LastScan"))
            scanFinished(path);
    }
    scheduleChanged();
}

void Client::connectionUpdated(const Message& message)
{
    if (m_loading || !has(message.path(), connectionInterface))
        return;
    fetchSettings(message.path());
}

void Client::activeStateSignal(const Message& message)
{
    uint32_t state = 0;
    uint32_t reason = 0;
    if (m_loading || !message.read(state, reason))
        return;

    const std::string path = message.path();
    if (has(path, activeInterface))
        m_objects[path][activeInterface]["State"] = Variant(state);
    activeStateChanged.emit(path, state, reason);
    scheduleChanged();
}

void Client::fetchSettings(const std::string& path)
{
    // A newer request makes an older reply stale, whichever arrives first.
    const uint64_t request = ++m_nextRequest;
    m_settingsRequests[path] = request;

    const uint64_t generation = m_generation;
    call(path, connectionInterface, "GetSettings", {}, nullptr, [this, generation, path, request](const Reply& reply) {
        const auto pending = m_settingsRequests.find(path);
        if (generation != m_generation || pending == m_settingsRequests.end() || pending->second != request)
            return;
        m_settingsRequests.erase(pending);

        // Another user's private profile: it exists, but its settings are not ours to read.
        if (const auto settings = reply.value<Settings>())
            m_settings[path] = *settings;
        else
            m_settings.erase(path);
        updateReady();
        scheduleChanged();
    });
}

void Client::fetchPermissions()
{
    if (m_loading)
        return;

    const uint64_t request = ++m_nextRequest;
    m_permissionsRequest = request;
    const uint64_t generation = m_generation;
    call(managerPath, managerInterface, "GetPermissions", {}, nullptr, [this, generation, request](const Reply& reply) {
        if (generation != m_generation || request != m_permissionsRequest)
            return;
        m_permissions = reply.value<std::map<std::string, std::string>>().value_or(std::map<std::string, std::string>());
        scheduleChanged();
    });
}

void Client::scheduleChanged()
{
    if (!m_changedTimer.isActive())
        m_changedTimer.start();
}

void Client::updateReady()
{
    // Once ready, later profile fetches don't take the network lists away again.
    if (m_ready || !m_loaded || !m_settingsRequests.empty())
        return;
    m_ready = true;
    readyChanged.emit();
}

// NetworkManager drops every access point when Wi-Fi goes off, the device
// goes down or asleep, or it starts a hotspot.
bool Client::keepsAccessPoints(const std::string& device) const
{
    return has(device, wirelessInterface) && toBool(manager("WirelessEnabled"))
           && deviceState(device) >= DeviceDisconnected
           && toUInt(property(device, wirelessInterface, "Mode")) != ModeAp;
}

void Client::noteAccessPoints(const std::string& device)
{
    for (const std::string& accessPoint : accessPoints(device))
        m_accessPointDevices[accessPoint] = device;
}

void Client::rememberAccessPoint(const std::string& path, const VariantMap& properties)
{
    const auto found = m_accessPointDevices.find(path);
    if (found == m_accessPointDevices.end())
        return;
    const std::string device = found->second;
    m_accessPointDevices.erase(found);
    if (device.empty() || !keepsAccessPoints(device))
        return;

    // Seen by the last scan and gone long after it: it aged out rather than went away.
    const int64_t lastScan = toInt64(property(device, wirelessInterface, "LastScan"), -1);
    const int64_t lastSeen = contains(properties, "LastSeen") ? toInt64(value(properties, "LastSeen"), -1) : -1;
    if (lastScan < 0 || lastSeen < 0 || lastSeen * 1000 < lastScan - kScanSpreadMs
        || bootTimeMs() - lastScan < kAgedOutAfterMs)
        return;
    m_remembered[path] = RememberedAccessPoint{device, properties, 0};
}

void Client::scanFinished(const std::string& device)
{
    for (auto it = m_remembered.begin(); it != m_remembered.end();) {
        if (it->second.device == device
            && (!listedAccessPoint(device, it->second.properties).empty()
                || ++it->second.missedScans >= kMissedScansForgotten))
            it = m_remembered.erase(it);
        else
            ++it;
    }
}

void Client::pruneRemembered()
{
    for (auto it = m_remembered.begin(); it != m_remembered.end();) {
        if (keepsAccessPoints(it->second.device))
            ++it;
        else
            it = m_remembered.erase(it);
    }
}

} // namespace Lingmo::Networkmanager
