// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/bluez/manager.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <linux/rfkill.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <utility>

#include "base/event_loop.h"
#include "framework_i18n.h"

using namespace Lingmo::Base;
using namespace std::chrono_literals;
using Lingmo::framework::tr;
using Lingmo::i18n::format;

namespace Lingmo::bluez {

static constexpr const char* kService = "org.bluez";
static constexpr const char* kPropertiesInterface = "org.freedesktop.DBus.Properties";
static constexpr const char* kObjectManagerInterface = "org.freedesktop.DBus.ObjectManager";
static constexpr const char* kGattCharacteristicInterface = "org.bluez.GattCharacteristic1";
static constexpr const char* kAgentManagerPath = "/org/bluez";
static constexpr const char* kAgentManagerInterface = "org.bluez.AgentManager1";

// The Bluetooth rfkill switches, from /dev/rfkill.
class Rfkill
{
public:
    explicit Rfkill(Manager* manager)
        : m_manager(manager)
    {
        m_fd = ::open("/dev/rfkill", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (m_fd < 0)
            return;
        m_loop = EventLoop::current();
        if (m_loop)
            m_watch = m_loop->watch(m_fd, EPOLLIN, [this](uint32_t) { readEvents(); });
        readEvents();
    }

    ~Rfkill()
    {
        if (m_loop && m_watch)
            m_loop->unwatch(m_watch);
        if (m_fd >= 0)
            ::close(m_fd);
    }

    bool hardwareBlocked() const
    {
        return std::any_of(m_states.begin(), m_states.end(), [](const auto& state) { return state.second.second; });
    }

    bool softwareBlocked() const
    {
        return std::any_of(m_states.begin(), m_states.end(), [](const auto& state) { return state.second.first; });
    }

    bool unblock()
    {
        if (!softwareBlocked())
            return true;
        const int fd = ::open("/dev/rfkill", O_WRONLY | O_CLOEXEC);
        rfkill_event event{};
        event.type = RFKILL_TYPE_BLUETOOTH;
        event.op = RFKILL_OP_CHANGE_ALL;
        const bool ok = fd >= 0 && ::write(fd, &event, sizeof(event)) == sizeof(event);
        const int error = errno;
        if (fd >= 0)
            ::close(fd);
        if (!ok)
            m_manager->fail(format(tr("Cannot unblock Bluetooth: {error}"), {{"error", std::strerror(error)}}));
        return ok;
    }

private:
    void readEvents()
    {
        rfkill_event event{};
        while (::read(m_fd, &event, sizeof(event)) == sizeof(event)) {
            if (event.op == RFKILL_OP_DEL)
                m_states.erase(event.idx);
            else if (event.type == RFKILL_TYPE_BLUETOOTH)
                m_states[event.idx] = {event.soft != 0, event.hard != 0};
        }
        m_manager->emitChanged();
    }

    Manager* m_manager;
    EventLoop* m_loop = nullptr;
    int m_fd = -1;
    int m_watch = 0;
    // Soft and hard blocks by switch.
    std::map<uint32_t, std::pair<bool, bool>> m_states;
};

static std::weak_ptr<Manager> s_shared;

std::shared_ptr<Manager> Manager::shared()
{
    std::shared_ptr<Manager> manager = s_shared.lock();
    if (!manager) {
        manager = std::make_shared<Manager>(dbus::Connection::system(), true);
        manager->m_notifications = manager->deviceFailed.connect(
            [manager = manager.get()](const std::string& summary, const std::string& body) {
                manager->notify(summary, body);
            });
        s_shared = manager;
    }
    return manager;
}

void Manager::share(const std::shared_ptr<Manager>& manager)
{
    s_shared = manager;
}

Manager::Manager(dbus::Connection* bus, bool monitorRfkill)
    : m_bus(bus)
{
    m_promptTimer.setSingleShot(true);
    m_promptTimer.setInterval(120s);
    m_promptTimer.onTimeout = [this] {
        const std::string device = m_pairingDevice;
        cancelPairing();
        deviceFailure("Pair", device, tr("The device didn't finish pairing in time. Try again."));
    };
    m_retryTimer.setSingleShot(true);
    m_retryTimer.setInterval(2s);
    m_retryTimer.onTimeout = [this] { refresh(); };
    m_syncTimer.setSingleShot(true);
    m_syncTimer.onTimeout = [this] { syncDiscovery(); };
    m_stopRetryTimer.setSingleShot(true);
    m_stopRetryTimer.setInterval(1s);
    m_stopRetryTimer.onTimeout = [this] { syncDiscovery(); };
    if (monitorRfkill)
        m_rfkill = std::make_unique<Rfkill>(this);
    if (!m_bus) {
        fail(tr("The system D-Bus connection is unavailable."));
        setReady();
        return;
    }
    // Reports the owner at once, which loads BlueZ when it runs.
    m_watch = m_bus->watchName(kService, [this](const std::string& owner) { ownerChanged(owner); });
}

Manager::~Manager()
{
    finishPrompt();
    if (!m_bus || m_owner.empty())
        return;
    for (const std::string& path : m_pairingOperations)
        m_bus->send(m_bus->newMethodCall(m_owner, path, kDeviceInterface, "CancelPairing"));
    if (m_agentReady) {
        dbus::Message message =
            m_bus->newMethodCall(m_owner, kAgentManagerPath, kAgentManagerInterface, "UnregisterAgent");
        message.append(dbus::ObjectPath{kAgentPath});
        m_bus->send(message);
    }
    for (const std::string& path : m_discoveryOwned)
        m_bus->send(m_bus->newMethodCall(m_owner, path, kAdapterInterface, "StopDiscovery"));
    for (auto& [path, object] : m_objects)
        object->detach();
}

void Manager::emitChanged()
{
    changed.emit();
    const bool present = available();
    if (m_lastAvailable != present) {
        m_lastAvailable = present;
        availableChanged.emit();
    }
    const bool powered = enabled();
    if (m_lastEnabled != powered) {
        m_lastEnabled = powered;
        enabledChanged.emit();
    }
}

void Manager::refresh()
{
    if (m_loading || !m_bus)
        return;
    const uint64_t generation = m_generation;
    const uint64_t id = ++m_nextCall;
    dbus::PendingCall pending = dbus::Proxy(m_bus, "org.freedesktop.DBus", "/org/freedesktop/DBus",
                                            "org.freedesktop.DBus")
                                    .callAsync(
                                        "GetNameOwner",
                                        [this, generation, id](const dbus::Reply& reply) {
                                            m_calls.erase(id);
                                            if (generation != m_generation)
                                                return;
                                            const auto owner = reply.value<std::string>();
                                            if (!owner) {
                                                if (m_operational)
                                                    reset();
                                                setReady();
                                                return;
                                            }
                                            if (!m_operational)
                                                load(*owner);
                                        },
                                        std::string(kService));
    if (!pending.isNull())
        m_calls.emplace(id, std::move(pending));
}

void Manager::setReady()
{
    if (m_ready)
        return;
    m_ready = true;
    readyChanged.emit();
}

void Manager::ownerChanged(const std::string& owner)
{
    // The first report of an owner already being read.
    if (owner == m_owner && (m_loading || m_operational))
        return;
    reset();
    if (!owner.empty())
        load(owner);
    else
        setReady();
}

void Manager::reset()
{
    ++m_generation;
    m_retryTimer.stop();
    m_matches.clear();
    m_calls.clear();
    finishPrompt();
    m_owner.clear();
    m_operational = false;
    m_loading = false;
    m_agentReady = false;
    m_registeringAgent = false;
    m_events.clear();
    m_discoveryOwned.clear();
    m_discoveryPending.clear();
    m_discoveryFailed.clear();
    m_stopRetries.clear();
    m_pairingOperations.clear();
    const auto objects = std::exchange(m_objects, {});
    m_objectSubscriptions.clear();
    for (const auto& [path, object] : objects)
        object->detach();
    adaptersChanged.emit();
    devicesChanged.emit();
    emitChanged();
}

void Manager::load(const std::string& owner)
{
    if (m_loading)
        return;
    m_owner = owner;
    m_loading = true;
    m_matches.push_back(m_bus->subscribe(
        {.sender = owner, .path = "/", .interface = kObjectManagerInterface, .member = "InterfacesAdded"},
        [this](const dbus::Message& message) {
            dbus::ObjectPath path;
            BluetoothObject::Interfaces interfaces;
            if (message.read(path, interfaces))
                interfacesAdded(path.path, interfaces);
        }));
    m_matches.push_back(m_bus->subscribe(
        {.sender = owner, .path = "/", .interface = kObjectManagerInterface, .member = "InterfacesRemoved"},
        [this](const dbus::Message& message) {
            dbus::ObjectPath path;
            std::vector<std::string> interfaces;
            if (message.read(path, interfaces))
                interfacesRemoved(path.path, interfaces);
        }));
    m_matches.push_back(m_bus->subscribe(
        {.sender = owner, .interface = kPropertiesInterface, .member = "PropertiesChanged"},
        [this](const dbus::Message& message) {
            std::string interface;
            dbus::VariantMap properties;
            std::vector<std::string> invalidated;
            if (message.read(interface, properties, invalidated))
                propertiesChanged(message.path(), interface, properties, invalidated);
        }));
    call("/", kObjectManagerInterface, "GetManagedObjects", {}, [this](const dbus::Reply& reply) {
        using Objects = std::map<dbus::ObjectPath, BluetoothObject::Interfaces>;
        const auto objects = reply.value<Objects>();
        if (!objects) {
            reset();
            m_retryTimer.start();
            setReady();
            return;
        }
        m_loading = false;
        for (const auto& [path, interfaces] : *objects)
            interfacesAdded(path.path, interfaces);
        const auto events = std::exchange(m_events, {});
        for (const auto& event : events)
            event();
        m_operational = true;
        emitChanged();
        setReady();
        registerAgent();
        syncDiscovery();
    });
}

void Manager::interfacesAdded(const std::string& path, const BluetoothObject::Interfaces& interfaces)
{
    if (m_loading) {
        m_events.push_back([this, path, interfaces] { interfacesAdded(path, interfaces); });
        return;
    }
    std::shared_ptr<BluetoothObject>& entry = m_objects[path];
    if (!entry) {
        entry = std::make_shared<BluetoothObject>(path, this);
        m_objectSubscriptions[path] = entry->changed.connect([this] {
            emitChanged();
            devicesChanged.emit();
            m_syncTimer.start(0ms);
        });
    }
    const bool wasAdapter = entry->isAdapter();
    const std::shared_ptr<BluetoothObject> object = entry;
    for (const auto& [interface, properties] : interfaces)
        object->update(interface, properties);
    if (!wasAdapter && object->isAdapter())
        adaptersChanged.emit();
    devicesChanged.emit();
    emitChanged();
}

void Manager::interfacesRemoved(const std::string& path, const std::vector<std::string>& interfaces)
{
    if (m_loading) {
        m_events.push_back([this, path, interfaces] { interfacesRemoved(path, interfaces); });
        return;
    }
    const std::shared_ptr<BluetoothObject> entry = object(path);
    if (!entry)
        return;
    const auto contains = [&interfaces](const char* interface) {
        return std::find(interfaces.begin(), interfaces.end(), interface) != interfaces.end();
    };
    const auto drop = [this, &path, &entry] {
        m_objects.erase(path);
        m_objectSubscriptions.erase(path);
        entry->detach();
    };
    if (contains(kDeviceInterface) || contains(kAdapterInterface)) {
        if (m_pairingPath == path)
            finishPrompt();
        const bool adapter = entry->isAdapter();
        drop();
        m_discoveryOwned.erase(path);
        m_discoveryPending.erase(path);
        m_discoveryFailed.erase(path);
        m_stopRetries.erase(path);
        m_pairingOperations.erase(path);
        // bluetoothd removes an adapter's devices itself, one InterfacesRemoved
        // each, before the adapter.
        if (adapter)
            adaptersChanged.emit();
    } else {
        for (const std::string& interface : interfaces)
            entry->removeInterface(interface);
        // bluetoothd names media transports from a global counter, so their paths never
        // return; the standard freedesktop interfaces alone do not keep an object.
        if (!entry->hasBluezInterface())
            drop();
    }
    devicesChanged.emit();
    emitChanged();
}

void Manager::propertiesChanged(const std::string& path, const std::string& interface,
                                const dbus::VariantMap& properties, const std::vector<std::string>& invalidated)
{
    if (m_loading) {
        m_events.push_back([this, path, interface, properties, invalidated] {
            propertiesChanged(path, interface, properties, invalidated);
        });
        return;
    }
    const std::shared_ptr<BluetoothObject> entry = object(path);
    if (!entry)
        return;
    entry->update(interface, properties, invalidated);
    // A code shown for a pairing another process started has nothing left to
    // wait for once the device reports it paired.
    if (interface == kDeviceInterface && entry->path() == m_pairingPath && !m_request) {
        const auto paired = properties.find("Paired");
        if (paired != properties.end() && paired->second.value<bool>())
            finishPrompt();
    }
    if (interface == kAdapterInterface && !entry->powered()) {
        m_discoveryOwned.erase(entry->path());
        m_discoveryFailed.erase(entry->path());
        m_stopRetries.erase(entry->path());
    }
    // Invalidated values must be fetched again; a missing optional value stays absent.
    if (!invalidated.empty()) {
        std::weak_ptr<BluetoothObject> guard = entry;
        std::map<std::string, uint64_t> revisions;
        for (const std::string& key : invalidated)
            revisions[key] = entry->revision(interface, key);
        call(
            entry->path(), kPropertiesInterface, "GetAll", {interface},
            [guard, interface, revisions](const dbus::Reply& reply) {
                const std::shared_ptr<BluetoothObject> object = guard.lock();
                const auto values = reply.value<dbus::VariantMap>();
                if (!object || !object->hasInterface(interface) || !values)
                    return;
                dbus::VariantMap refreshed;
                for (const auto& [key, revision] : revisions) {
                    const auto value = values->find(key);
                    if (object->revision(interface, key) == revision && value != values->end())
                        refreshed.insert_or_assign(key, value->second);
                }
                if (!refreshed.empty())
                    object->update(interface, refreshed);
            },
            Errors::Ignore);
    }
}

std::vector<std::shared_ptr<BluetoothObject>> Manager::adapters() const
{
    std::vector<std::shared_ptr<BluetoothObject>> result;
    for (const auto& [path, object] : m_objects) {
        if (object->isAdapter())
            result.push_back(object);
    }
    return result;
}

std::vector<std::shared_ptr<BluetoothObject>> Manager::devices() const
{
    std::vector<std::shared_ptr<BluetoothObject>> result;
    for (const auto& [path, entry] : m_objects) {
        if (entry->isDevice() && object(entry->adapterPath()))
            result.push_back(entry);
    }
    return result;
}

std::shared_ptr<BluetoothObject> Manager::object(const std::string& path) const
{
    const auto found = m_objects.find(path);
    return found == m_objects.end() ? nullptr : found->second;
}

bool Manager::isCurrent(const BluetoothObject* entry) const
{
    return entry && object(entry->path()).get() == entry;
}

bool Manager::enabled() const
{
    for (const auto& adapter : adapters()) {
        if (adapter->powered())
            return true;
    }
    return false;
}

bool Manager::hardwareBlocked() const
{
    return m_rfkill && m_rfkill->hardwareBlocked();
}

bool Manager::scanning() const
{
    for (const auto& adapter : adapters()) {
        if (adapter->discovering())
            return true;
    }
    return false;
}

void Manager::setEnabled(bool enabled)
{
    clearError();
    if (enabled && m_rfkill && !m_rfkill->unblock())
        return;
    if (enabled && hardwareBlocked())
        fail(tr("Bluetooth is blocked by a hardware switch."));
    if (!enabled)
        setDiscoveryRequested(false);
    if (!m_operational) {
        fail(tr("The Bluetooth service is unavailable."));
        return;
    }
    for (const auto& adapter : adapters())
        adapter->setPowered(enabled);
}

void Manager::call(const std::string& path, const std::string& interface, const std::string& method,
                   const dbus::Variant::Items& arguments, Completion completion, Errors errors,
                   std::chrono::milliseconds timeout)
{
    if (m_owner.empty()) {
        const dbus::Reply error({}, {"org.bluez.Error.NotReady", tr("The Bluetooth service is unavailable.")});
        if (errors == Errors::Report)
            fail(error.error().message);
        if (completion)
            completion(error);
        return;
    }
    dbus::Message message = m_bus->newMethodCall(m_owner, path, interface, method);
    for (const dbus::Variant& argument : arguments)
        message.appendAs(argument);
    const uint64_t generation = m_generation;
    const uint64_t id = ++m_nextCall;
    dbus::PendingCall pending = m_bus->callAsync(
        message,
        [this, generation, id, method, completion = std::move(completion), errors](const dbus::Reply& reply) {
            m_calls.erase(id);
            if (generation != m_generation)
                return;
            if (reply.isError() && errors == Errors::Report) {
                std::fprintf(stderr, "lingmo.bluez: %s failed: %s %s\n", method.c_str(), reply.error().name.c_str(),
                             reply.error().message.c_str());
                const std::string reason = describe(reply.error());
                fail(reason.empty() ? tr("Bluetooth couldn't complete the request.") : reason);
            }
            if (completion)
                completion(reply);
        },
        timeout);
    if (!pending.isNull())
        m_calls.emplace(id, std::move(pending));
}

void Manager::setObjectProperty(BluetoothObject* entry, const std::string& property, const dbus::Variant& value)
{
    if (!isCurrent(entry))
        return;
    const std::string interface = entry->isAdapter() ? kAdapterInterface : kDeviceInterface;
    call(entry->path(), kPropertiesInterface, "Set", {interface, property, dbus::Type<dbus::Variant>::toVariant(value)});
}

void Manager::operate(BluetoothObject* entry, const std::string& method, const std::string& argument)
{
    if (!isCurrent(entry) || !entry->isDevice()) {
        deviceFailure(method, entry ? entry->name() : std::string(), tr("This device is no longer available."));
        return;
    }
    const bool cancel = method == "CancelPairing";
    const bool pair = method == "Pair";
    if (entry->busy() && !cancel) {
        deviceFailure(method, entry->name(), tr("The device is busy. Try again in a moment."));
        return;
    }
    // Without an agent of its own the caller pairs through BlueZ's default one,
    // which the shell keeps registered.
    if (pair && m_agentEnabled && !m_agentReady) {
        deviceFailure(method, entry->name(), tr("Bluetooth isn't ready yet. Try again in a moment."));
        return;
    }
    clearError();
    if (!cancel)
        entry->setBusy(true);
    const bool remove = method == "RemoveDevice";
    const std::weak_ptr<BluetoothObject> guard = object(entry->path());
    const std::string path = entry->path();
    const std::string name = entry->name();
    if (pair)
        m_pairingOperations.insert(path);
    dbus::Variant::Items arguments;
    if (remove)
        arguments.push_back(dbus::ObjectPath{path});
    else if (!argument.empty())
        arguments.push_back(argument);
    call(
        remove ? entry->adapterPath() : path, remove ? kAdapterInterface : kDeviceInterface, method, arguments,
        [this, guard, path, name, method, cancel, pair](const dbus::Reply& reply) {
            const std::shared_ptr<BluetoothObject> device = guard.lock();
            if (pair)
                m_pairingOperations.erase(path);
            if (device && !cancel)
                device->setBusy(false);
            if ((pair || cancel) && m_pairingPath == path)
                finishPrompt();
            if (reply.isError() && !cancel) {
                std::fprintf(stderr, "lingmo.bluez: %s failed: %s %s\n", method.c_str(),
                             reply.error().name.c_str(), reply.error().message.c_str());
                // Dismissing the pairing prompt ends Pair with Canceled or Rejected,
                // and a device that is already in the requested state needs no report.
                static const char* const quiet[] = {"org.bluez.Error.AuthenticationCanceled",
                                                    "org.bluez.Error.AuthenticationRejected",
                                                    "org.bluez.Error.AlreadyConnected",
                                                    "org.bluez.Error.AlreadyExists", "org.bluez.Error.NotConnected"};
                const bool isQuiet = std::find(std::begin(quiet), std::end(quiet), reply.error().name) != std::end(quiet);
                const bool alreadyConnected = reply.error().message.ends_with("-already-connected");
                if (!isQuiet && !alreadyConnected)
                    deviceFailure(method, device ? device->name() : name, describe(reply.error()));
            }
            if (!device || reply.isError() || !pair)
                return;
            // Trust is granted only after a successful user-initiated pairing.
            device->setBusy(true);
            call(path, kPropertiesInterface, "Set",
                 {std::string(kDeviceInterface), std::string("Trusted"), dbus::Type<dbus::Variant>::toVariant(true)},
                 [this, guard](const dbus::Reply& trustedReply) {
                     const std::shared_ptr<BluetoothObject> trusted = guard.lock();
                     if (!trusted)
                         return;
                     trusted->setBusy(false);
                     if (!trustedReply.isError())
                         operate(trusted.get(), "Connect");
                 });
        },
        Errors::Ignore, pair ? 180s : 60s);
}

void Manager::setDiscoveryRequested(bool requested)
{
    m_discoveryFailed.clear();
    m_stopRetries.clear();
    if (m_discoveryRequested != requested) {
        m_discoveryRequested = requested;
        emitChanged();
    }
    syncDiscovery();
}

void Manager::syncDiscovery()
{
    if (!m_operational)
        return;
    for (const auto& adapter : adapters())
        discovery(adapter.get(), m_discoveryRequested && adapter->powered());
}

void Manager::discovery(BluetoothObject* adapter, bool start)
{
    if (!adapter || !adapter->isAdapter() || !m_operational)
        return;
    const std::string path = adapter->path();
    const auto retries = m_stopRetries.find(path);
    if (m_discoveryPending.contains(path) || m_discoveryOwned.contains(path) == start
        || (start && m_discoveryFailed.contains(path))
        || (!start && retries != m_stopRetries.end() && retries->second >= 3))
        return;
    m_discoveryPending.insert(path);
    const std::weak_ptr<BluetoothObject> guard = object(path);
    call(path, kAdapterInterface, start ? "StartDiscovery" : "StopDiscovery", {},
         [this, path, guard, start](const dbus::Reply& reply) {
             m_discoveryPending.erase(path);
             const std::shared_ptr<BluetoothObject> current = guard.lock();
             if (!current || object(path) != current)
                 return;
             if (reply.isError()) {
                 if (start) {
                     m_discoveryFailed.insert(path);
                     if (m_discoveryOwned.empty() && m_discoveryPending.empty())
                         m_discoveryRequested = false;
                     emitChanged();
                 } else {
                     ++m_stopRetries[path];
                     m_stopRetryTimer.start();
                 }
                 return;
             }
             if (start)
                 m_discoveryOwned.insert(path);
             else
                 m_discoveryOwned.erase(path);
             m_stopRetries.erase(path);
             if (start != (m_discoveryRequested && current->powered()))
                 syncDiscovery();
         });
}

void Manager::mediaCommand(BluetoothObject* device, const std::string& command)
{
    static const char* const commands[] = {"Play", "Pause", "Stop", "Next", "Previous", "FastForward", "Rewind"};
    if (!device || std::find(std::begin(commands), std::end(commands), command) == std::end(commands))
        return;
    for (const auto& [path, entry] : m_objects) {
        if (path.starts_with(device->path() + "/") && entry->hasInterface("org.bluez.MediaPlayer1")) {
            call(path, "org.bluez.MediaPlayer1", command);
            return;
        }
    }
    fail(tr("This device has no media player."));
}

static std::string lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return char(std::tolower(c)); });
    return text;
}

static std::string trimmed(const std::string& text)
{
    const auto first = text.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos)
        return {};
    const auto last = text.find_last_not_of(" \t\n\r\f\v");
    return text.substr(first, last - first + 1);
}

void Manager::readDeviceInformation(BluetoothObject* device)
{
    if (!isCurrent(device) || !device->connected())
        return;
    static const std::string modelNumberUuid = "00002a24-0000-1000-8000-00805f9b34fb";
    static const std::string firmwareRevisionUuid = "00002a26-0000-1000-8000-00805f9b34fb";
    const std::weak_ptr<BluetoothObject> guard = object(device->path());
    for (const auto& [path, entry] : m_objects) {
        if (!path.starts_with(device->path() + "/") || !entry->hasInterface(kGattCharacteristicInterface))
            continue;
        const std::string uuid = lower(entry->value(kGattCharacteristicInterface, "UUID").value<std::string>());
        const bool model = uuid == modelNumberUuid;
        if (!model && uuid != firmwareRevisionUuid)
            continue;
        call(
            path, kGattCharacteristicInterface, "ReadValue", {dbus::VariantMap()},
            [guard, model](const dbus::Reply& reply) {
                const std::shared_ptr<BluetoothObject> target = guard.lock();
                const auto bytes = reply.value<std::vector<uint8_t>>();
                if (!target || !bytes)
                    return;
                // Some devices pad the string with NUL bytes.
                std::string text(bytes->begin(), bytes->end());
                const size_t end = text.find('\0');
                if (end != std::string::npos)
                    text.resize(end);
                text = trimmed(text);
                if (model)
                    target->setModelNumber(text);
                else
                    target->setFirmwareRevision(text);
            },
            Errors::Ignore);
    }
}

void Manager::setAgentEnabled(bool enabled)
{
    if (m_agentEnabled == enabled)
        return;
    if (enabled) {
        if (!m_bus) {
            fail(tr("Cannot export the Bluetooth pairing agent: no system bus."));
            return;
        }
        m_agent = std::make_unique<dbus::Object>(m_bus, kAgentPath, "org.bluez.Agent1");
        const std::pair<const char*, const char*> methods[] = {
            {"Release", ""}, {"Cancel", ""}, {"RequestPinCode", "o"}, {"RequestPasskey", "o"},
            {"DisplayPinCode", "os"}, {"DisplayPasskey", "ouq"}, {"RequestConfirmation", "ou"},
            {"RequestAuthorization", "o"}, {"AuthorizeService", "os"}};
        for (const auto& [name, signature] : methods) {
            const std::string member = name;
            const std::string out = member == "RequestPinCode" ? "s" : member == "RequestPasskey" ? "u" : "";
            m_agent->addMethod(member, signature, out, [this](dbus::MethodCall& call) { handleAgentCall(call); });
        }
        if (!m_agent->publish()) {
            m_agent.reset();
            fail(tr("Cannot export the Bluetooth pairing agent."));
            return;
        }
    }
    m_agentEnabled = enabled;
    if (enabled) {
        registerAgent();
    } else {
        const std::string promptPath = m_pairingPath;
        cancelPairing();
        const auto pending = m_pairingOperations;
        for (const std::string& path : pending) {
            if (path == promptPath)
                continue;
            if (const auto device = object(path))
                operate(device.get(), "CancelPairing");
        }
        if (m_agentReady)
            call(kAgentManagerPath, kAgentManagerInterface, "UnregisterAgent", {dbus::ObjectPath{kAgentPath}});
        m_agentReady = false;
        m_agent.reset();
    }
    emitChanged();
}

void Manager::registerAgent()
{
    if (!m_agentEnabled || !m_operational || m_agentReady || m_registeringAgent)
        return;
    m_registeringAgent = true;
    call(kAgentManagerPath, kAgentManagerInterface, "RegisterAgent",
         {dbus::ObjectPath{kAgentPath}, std::string("KeyboardDisplay")}, [this](const dbus::Reply& reply) {
             m_registeringAgent = false;
             if (reply.isError())
                 return;
             if (!m_agentEnabled) {
                 call(kAgentManagerPath, kAgentManagerInterface, "UnregisterAgent", {dbus::ObjectPath{kAgentPath}});
                 return;
             }
             m_agentReady = true;
             emitChanged();
             call(kAgentManagerPath, kAgentManagerInterface, "RequestDefaultAgent", {dbus::ObjectPath{kAgentPath}});
         });
}

void Manager::handleAgentCall(dbus::MethodCall& call)
{
    if (call.message().sender() != m_owner || !m_agentEnabled) {
        call.replyError("org.bluez.Error.Rejected", "Agent unavailable");
        return;
    }
    const std::string member = call.message().member();
    if (member == "Cancel" || member == "Release") {
        finishPrompt();
        if (member == "Release") {
            m_agentReady = false;
            emitChanged();
        }
        call.reply();
    } else {
        prompt(call);
    }
}

void Manager::prompt(dbus::MethodCall& call)
{
    const dbus::Message& message = call.message();
    const std::string path = message.argument<dbus::ObjectPath>(0).value_or(dbus::ObjectPath()).path;
    const std::string kind = message.member();
    const bool display = kind.starts_with("Display");
    if (m_request || (!m_pairingPath.empty() && m_pairingPath != path)) {
        call.replyError("org.bluez.Error.Rejected", "Another pairing request is active.");
        return;
    }
    m_pairingPath = path;
    const std::shared_ptr<BluetoothObject> device = object(path);
    m_pairingDevice = device ? device->name() : path;
    m_pairingKind = kind;
    m_pairingCode.clear();
    m_pairingEntered = 0;
    if (kind == "RequestConfirmation" || kind == "DisplayPasskey") {
        char code[16];
        std::snprintf(code, sizeof(code), "%06u", message.argument<uint32_t>(1).value_or(0));
        m_pairingCode = code;
    } else if (kind == "DisplayPinCode" || kind == "AuthorizeService") {
        m_pairingCode = message.argument<std::string>(1).value_or(std::string());
    }
    if (kind == "DisplayPasskey")
        m_pairingEntered = message.argument<uint16_t>(2).value_or(0);
    if (display)
        call.reply();
    else
        m_request = call;
    m_promptTimer.start();
    pairingChanged.emit();
    changed.emit();
}

void Manager::respondPairing(bool accept, const std::string& code)
{
    if (!m_request)
        return;
    if (!accept) {
        finishPrompt("org.bluez.Error.Rejected");
        return;
    }
    if (m_pairingKind == "RequestPinCode") {
        if (code.empty() || code.size() > 16) {
            fail(tr("Enter a PIN containing 1–16 bytes."));
            return;
        }
        m_request->reply(code);
    } else if (m_pairingKind == "RequestPasskey") {
        const bool digits = !code.empty() && code.size() <= 6
                            && std::all_of(code.begin(), code.end(), [](char c) { return c >= '0' && c <= '9'; });
        if (!digits) {
            fail(tr("Enter a passkey containing 1–6 digits."));
            return;
        }
        m_request->reply(uint32_t(std::stoul(code)));
    } else {
        m_request->reply();
    }
    m_request.reset();
    finishPrompt();
}

void Manager::finishPrompt(const std::string& error)
{
    if (m_request)
        m_request->replyError(error, "Pairing request dismissed.");
    m_request.reset();
    m_promptTimer.stop();
    m_pairingKind.clear();
    m_pairingPath.clear();
    m_pairingDevice.clear();
    m_pairingCode.clear();
    m_pairingEntered = 0;
    pairingChanged.emit();
    changed.emit();
}

void Manager::cancelPairing()
{
    const std::string path = m_pairingPath;
    finishPrompt();
    if (const auto device = object(path))
        operate(device.get(), "CancelPairing");
}

void Manager::fail(const std::string& message)
{
    m_error = message;
    errorChanged.emit();
    changed.emit();
}

void Manager::deviceFailure(const std::string& method, const std::string& device, const std::string& body)
{
    const bool named = !device.empty();
    std::string summary;
    if (method == "Connect" || method == "ConnectProfile")
        summary = named ? format(tr("Couldn't connect to {device}"), {{"device", device}})
                        : tr("Couldn't connect to the device");
    else if (method == "Disconnect" || method == "DisconnectProfile")
        summary = named ? format(tr("Couldn't disconnect {device}"), {{"device", device}})
                        : tr("Couldn't disconnect the device");
    else if (method == "Pair")
        summary = named ? format(tr("Couldn't pair with {device}"), {{"device", device}})
                        : tr("Couldn't pair with the device");
    else if (method == "RemoveDevice")
        summary = named ? format(tr("Couldn't forget {device}"), {{"device", device}})
                        : tr("Couldn't forget the device");
    else
        return;
    deviceFailed.emit(summary,
                      body.empty() ? tr("Try again. If it keeps failing, turn the device off and on again.") : body);
}

// BlueZ reports most connection failures as org.bluez.Error.Failed with a
// "br-connection-*" or "le-connection-*" reason, or an errno string on older versions.
std::string Manager::describe(const dbus::Error& error) const
{
    const size_t dot = error.name.rfind('.');
    const std::string name = dot == std::string::npos ? error.name : error.name.substr(dot + 1);
    std::string reason = error.message;
    for (const char* prefix : {"br-connection-", "le-connection-"}) {
        if (reason.starts_with(prefix))
            reason = reason.substr(std::strlen(prefix));
    }

    if (name == "NoReply" || name == "AuthenticationTimeout" || reason == "timeout"
        || reason == "Connection timed out")
        return tr("The device didn't respond in time. Make sure it is turned on and nearby.");
    if (name == "ConnectionAttemptFailed" || reason == "page-timeout" || reason == "Host is down")
        return tr("The device didn't respond. Make sure it is turned on, nearby, and not connected to another device.");
    if (reason == "key-missing")
        return tr("The device no longer recognizes this computer. Forget the device and pair it again.");
    if (name == "AuthenticationFailed")
        return tr("Pairing failed. Make sure the code matches on both devices and try again.");
    if (name == "AuthenticationRejected")
        return tr("The device declined the request.");
    if (reason == "refused" || reason == "Connection refused")
        return tr("The device refused the connection.");
    if (name == "NotSupported" || reason == "profile-unavailable" || reason == "not-supported"
        || reason == "sdp-search" || reason == "Protocol not available")
        return tr("The device has no features this computer can use.");
    if (reason == "adapter-not-powered")
        return tr("Bluetooth is turned off.");
    if (name == "Blocked" && hardwareBlocked())
        return tr("Bluetooth is blocked by a hardware switch.");
    if (name == "NotReady")
        return tr("Bluetooth isn't ready yet. Try again in a moment.");
    if (name == "InProgress" || reason == "busy" || reason == "Operation already in progress")
        return tr("The device is busy. Try again in a moment.");
    if (reason == "concurrent-connection-limit")
        return tr("Too many devices are connected. Disconnect one and try again.");
    if (reason == "aborted-by-remote")
        return tr("The device ended the connection.");
    if (reason == "aborted-by-local" || reason == "canceled")
        return tr("The connection was interrupted. Try again.");
    if (name == "DoesNotExist" || name == "UnknownObject")
        return tr("This device is no longer available.");
    return {};
}

void Manager::notify(const std::string& summary, const std::string& body)
{
    // Sent from whichever process shows Bluetooth, so no desktop entry of its own.
    dbus::Connection* session = dbus::Connection::session();
    if (!session)
        return;
    const dbus::VariantMap hints{{"urgency", uint8_t(1)}};
    dbus::Proxy(session, "org.freedesktop.Notifications", "/org/freedesktop/Notifications",
                "org.freedesktop.Notifications")
        .send("Notify", std::string("Bluetooth"), uint32_t(0), std::string("bluetooth"), summary, body,
              std::vector<std::string>(), hints, int32_t(-1));
}

void Manager::clearError()
{
    if (m_error.empty())
        return;
    m_error.clear();
    errorChanged.emit();
    changed.emit();
}

} // namespace Lingmo::bluez
