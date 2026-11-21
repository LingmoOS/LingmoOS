// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "base/timer.h"
#include "lingmo/bluez/bluetooth_object.h"

namespace Lingmo::bluez {

inline constexpr const char* kAdapterInterface = "org.bluez.Adapter1";
inline constexpr const char* kDeviceInterface = "org.bluez.Device1";
inline constexpr const char* kAgentPath = "/org/lingmo/BluetoothAgent";

class Rfkill;

// BlueZ as one process sees it: its adapters and devices from the object
// manager, kept up to date from its signals, discovery while someone asks for
// it, and optionally a pairing agent whose prompts the caller shows.
class Manager
{
public:
    // The system bus's BlueZ, shared by everyone in the process who asks while
    // one is alive. Device failures become desktop notifications.
    static std::shared_ptr<Manager> shared();
    // Makes shared() return this manager while it lives: for tests.
    static void share(const std::shared_ptr<Manager>& manager);

    // On the given bus, without rfkill or notifications unless asked: for tests.
    explicit Manager(base::dbus::Connection* bus, bool monitorRfkill = false);
    ~Manager();

    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;

    bool operational() const { return m_operational; }
    bool available() const { return m_operational && !adapters().empty(); }
    // BlueZ has been read, or found absent; until then available() is false either way.
    bool ready() const { return m_ready; }
    // An adapter is powered.
    bool enabled() const;
    bool hardwareBlocked() const;
    std::vector<std::shared_ptr<BluetoothObject>> adapters() const;
    // The devices of known adapters.
    std::vector<std::shared_ptr<BluetoothObject>> devices() const;
    bool scanning() const;
    bool discoveryRequested() const { return m_discoveryRequested; }
    bool agentEnabled() const { return m_agentEnabled; }
    bool agentReady() const { return m_agentReady; }
    const std::string& errorText() const { return m_error; }
    const std::string& pairingKind() const { return m_pairingKind; }
    const std::string& pairingDevice() const { return m_pairingDevice; }
    const std::string& pairingCode() const { return m_pairingCode; }
    int pairingEntered() const { return m_pairingEntered; }

    // Powers every adapter, unblocking rfkill first.
    void setEnabled(bool enabled);
    void setDiscoveryRequested(bool requested);
    void setAgentEnabled(bool enabled);
    void clearError();
    void respondPairing(bool accept, const std::string& code = {});
    void cancelPairing();
    void refresh();
    std::shared_ptr<BluetoothObject> object(const std::string& path) const;

    // For BluetoothObject.
    void setObjectProperty(BluetoothObject* object, const std::string& property, const base::dbus::Variant& value);
    void operate(BluetoothObject* object, const std::string& method, const std::string& argument = {});
    void discovery(BluetoothObject* adapter, bool start);
    void mediaCommand(BluetoothObject* device, const std::string& command);
    void readDeviceInformation(BluetoothObject* device);
    void fail(const std::string& message);

    using Completion = std::function<void(const base::dbus::Reply&)>;
    // Background refreshes race object removal; their failures are not the user's concern.
    enum class Errors {
        Report,
        Ignore,
    };
    void call(const std::string& path, const std::string& interface, const std::string& method,
              const base::dbus::Variant::Items& arguments = {}, Completion completion = {},
              Errors errors = Errors::Report, std::chrono::milliseconds timeout = std::chrono::seconds(30));

    base::Signal<> changed;
    base::Signal<> enabledChanged;
    base::Signal<> availableChanged;
    base::Signal<> readyChanged;
    base::Signal<> adaptersChanged;
    base::Signal<> devicesChanged;
    base::Signal<> errorChanged;
    base::Signal<> pairingChanged;
    // Device actions report here rather than through errorText; shared()
    // turns each one into a desktop notification.
    base::Signal<std::string, std::string> deviceFailed;

private:
    friend class Rfkill;

    void emitChanged();
    void ownerChanged(const std::string& owner);
    void reset();
    void load(const std::string& owner);
    void setReady();
    void syncDiscovery();
    void registerAgent();
    void handleAgentCall(base::dbus::MethodCall& call);
    void prompt(base::dbus::MethodCall& call);
    void finishPrompt(const std::string& error = "org.bluez.Error.Canceled");
    void interfacesAdded(const std::string& path, const BluetoothObject::Interfaces& interfaces);
    void interfacesRemoved(const std::string& path, const std::vector<std::string>& interfaces);
    void propertiesChanged(const std::string& path, const std::string& interface,
                           const base::dbus::VariantMap& properties, const std::vector<std::string>& invalidated);
    void deviceFailure(const std::string& method, const std::string& device, const std::string& body);
    std::string describe(const base::dbus::Error& error) const;
    void notify(const std::string& summary, const std::string& body);
    bool isCurrent(const BluetoothObject* object) const;

    base::dbus::Connection* m_bus;
    std::string m_owner;
    base::dbus::NameWatch m_watch;
    std::vector<base::dbus::Match> m_matches;
    std::map<std::string, std::shared_ptr<BluetoothObject>> m_objects;
    std::map<std::string, base::Subscription> m_objectSubscriptions;
    std::unique_ptr<Rfkill> m_rfkill;
    std::unique_ptr<base::dbus::Object> m_agent;
    base::Timer m_promptTimer;
    base::Timer m_retryTimer;
    base::Timer m_syncTimer;
    base::Timer m_stopRetryTimer;
    uint64_t m_generation = 0;
    uint64_t m_nextCall = 0;
    std::map<uint64_t, base::dbus::PendingCall> m_calls;
    bool m_operational = false;
    bool m_ready = false;
    bool m_lastEnabled = false;
    bool m_lastAvailable = false;
    bool m_loading = false;
    bool m_discoveryRequested = false;
    bool m_agentEnabled = false;
    bool m_agentReady = false;
    bool m_registeringAgent = false;
    std::set<std::string> m_discoveryOwned;
    std::set<std::string> m_discoveryPending;
    std::set<std::string> m_discoveryFailed;
    std::map<std::string, int> m_stopRetries;
    std::set<std::string> m_pairingOperations;
    std::string m_error;
    std::string m_pairingKind;
    std::string m_pairingPath;
    std::string m_pairingDevice;
    std::string m_pairingCode;
    int m_pairingEntered = 0;
    std::optional<base::dbus::MethodCall> m_request;
    std::vector<std::function<void()>> m_events;
    base::Subscription m_notifications;
};

} // namespace Lingmo::bluez
