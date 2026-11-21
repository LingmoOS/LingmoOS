// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <systemd/sd-bus.h>

#include "base/dbus.h"
#include "base/event_loop.h"

namespace fake {

using namespace Lingmo::base;

inline const std::string adapter = "/org/bluez/hci0";
inline const std::string device = adapter + "/dev_AA_BB_CC_DD_EE_FF";
inline const std::string adapterInterface = "org.bluez.Adapter1";
inline const std::string deviceInterface = "org.bluez.Device1";

using Interfaces = std::map<std::string, dbus::VariantMap>;
using Objects = std::map<dbus::ObjectPath, Interfaces>;

// BlueZ on a connection of its own, as the bluez client's tests have it:
// every object below "/" answers, as the object manager, the properties
// interface and the adapter and device methods.
class FakeBluez
{
public:
    explicit FakeBluez(EventLoop& loop)
        : bus(dbus::Connection::open(dbus::Connection::Bus::Session, &loop))
    {
        objects[{adapter}][adapterInterface] = {{"Address", std::string("11:22:33:44:55:66")},
                                                {"Alias", std::string("Test adapter")},
                                                {"Powered", true},
                                                {"Pairable", true}};
        objects[{device}][deviceInterface] = {{"Address", std::string("AA:BB:CC:DD:EE:FF")},
                                              {"Alias", std::string("Headphones")},
                                              {"Adapter", dbus::ObjectPath{adapter}},
                                              {"Paired", false},
                                              {"Connected", false}};
        sd_bus_add_fallback(bus->bus(), &m_slot, "/", &FakeBluez::dispatch, this);
    }

    ~FakeBluez()
    {
        sd_bus_slot_unref(m_slot);
    }

    bool registerService() { return bus->requestName("org.bluez"); }
    void unregisterService() { bus->releaseName("org.bluez"); }

    void set(const std::string& path, const std::string& interface, const std::string& key, const dbus::Variant& value)
    {
        objects[{path}][interface].insert_or_assign(key, value);
        bus->emitSignal(path, "org.freedesktop.DBus.Properties", "PropertiesChanged", interface,
                        dbus::VariantMap{{key, value}}, std::vector<std::string>());
    }

    void add(const std::string& path, const Interfaces& interfaces)
    {
        for (const auto& [interface, properties] : interfaces)
            objects[{path}][interface] = properties;
        bus->emitSignal("/", "org.freedesktop.DBus.ObjectManager", "InterfacesAdded", dbus::ObjectPath{path},
                        interfaces);
    }

    void remove(const std::string& path, const std::vector<std::string>& interfaces)
    {
        for (const std::string& interface : interfaces)
            objects[{path}].erase(interface);
        if (objects[{path}].empty())
            objects.erase(dbus::ObjectPath{path});
        bus->emitSignal("/", "org.freedesktop.DBus.ObjectManager", "InterfacesRemoved", dbus::ObjectPath{path},
                        interfaces);
    }

    // Calls the registered agent; the reply arrives in `done`.
    template<typename... Args>
    dbus::PendingCall ask(const std::string& method, std::function<void(const dbus::Reply&)> done, const Args&... args)
    {
        dbus::Message message = bus->newMethodCall(agentOwner, agentPath, "org.bluez.Agent1", method);
        message.append(args...);
        return bus->callAsync(message, std::move(done));
    }

    std::unique_ptr<dbus::Connection> bus;
    Objects objects;
    std::vector<std::string> calls;
    std::string lastPath;
    std::string failMethod;
    std::string failName = "org.bluez.Error.Failed";
    std::string failMessage = "Injected failure";
    std::string agentOwner;
    std::string agentPath;
    bool holdPair = false;
    bool holdSnapshot = false;
    bool holdGetAll = false;
    std::optional<dbus::MethodCall> pendingPair;
    std::optional<dbus::MethodCall> pendingSnapshot;
    std::optional<dbus::MethodCall> pendingGetAll;
    Objects snapshot;
    dbus::VariantMap oldProperties;

private:
    static int dispatch(sd_bus_message* message, void* data, sd_bus_error*)
    {
        uint8_t type = 0;
        if (sd_bus_message_get_type(message, &type) < 0 || type != SD_BUS_MESSAGE_METHOD_CALL)
            return 0;
        return static_cast<FakeBluez*>(data)->handle(dbus::Message(message)) ? 1 : 0;
    }

    bool handle(const dbus::Message& message)
    {
        const std::string member = message.member();
        if (member == "Introspect" || member == "Ping" || member == "GetMachineId")
            return false;
        calls.push_back(member);
        lastPath = message.path();
        dbus::MethodCall call(message);
        if (member == failMethod) {
            call.replyError(failName, failMessage);
            return true;
        }
        const std::string path = message.path();
        if (member == "GetManagedObjects") {
            if (holdSnapshot) {
                pendingSnapshot = call;
                snapshot = objects;
            } else {
                call.reply(objects);
            }
        } else if (member == "Set") {
            std::string interface;
            std::string key;
            dbus::Variant value;
            message.read(interface, key, value);
            set(path, interface, key, value);
            call.reply();
        } else if (member == "GetAll") {
            const std::string interface = message.argument<std::string>(0).value_or(std::string());
            if (holdGetAll) {
                pendingGetAll = call;
                oldProperties = objects[{path}][interface];
            } else {
                call.reply(objects[{path}][interface]);
            }
        } else if (member == "RegisterAgent") {
            agentOwner = message.sender();
            agentPath = message.argument<dbus::ObjectPath>(0).value_or(dbus::ObjectPath()).path;
            call.reply();
        } else if (member == "Pair") {
            if (holdPair) {
                pendingPair = call;
            } else {
                set(path, deviceInterface, "Paired", true);
                call.reply();
            }
        } else if (member == "CancelPairing") {
            if (pendingPair) {
                pendingPair->replyError("org.bluez.Error.AuthenticationCanceled", "Canceled");
                pendingPair.reset();
            }
            call.reply();
        } else if (member == "Connect" || member == "Disconnect") {
            set(path, deviceInterface, "Connected", member == "Connect");
            call.reply();
        } else if (member == "ReadValue") {
            const dbus::Variant value = objects[{path}]["org.bluez.GattCharacteristic1"]["Value"];
            call.reply(value.value<std::vector<uint8_t>>());
        } else if (member == "RemoveDevice") {
            remove(message.argument<dbus::ObjectPath>(0).value_or(dbus::ObjectPath()).path, {deviceInterface});
            call.reply();
        } else if (member.ends_with("Discovery")) {
            set(path, adapterInterface, "Discovering", member == "StartDiscovery");
            call.reply();
        } else {
            call.reply();
        }
        return true;
    }

    sd_bus_slot* m_slot = nullptr;
};

} // namespace fake
