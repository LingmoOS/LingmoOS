// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <systemd/sd-bus.h>

#include "base/dbus.h"
#include "lingmo/networkmanager/types.h"
#include "lingmo/networkmanager/variants.h"

// NetworkManager on a bus connection of its own: its objects, profiles and the
// calls made to it, with the signals NetworkManager sends when they change.
namespace fakenm {

using Lingmo::Base::dbus::Connection;
using Lingmo::Base::dbus::Message;
using Lingmo::Base::dbus::ObjectPath;
using Lingmo::Base::dbus::Variant;
using Lingmo::Base::dbus::VariantMap;
using Lingmo::networkmanager::Settings;
using Interfaces = std::map<std::string, VariantMap>;

inline const std::string managerPath = "/org/freedesktop/NetworkManager";
inline const std::string manager = "org.freedesktop.NetworkManager";
inline const std::string device = "org.freedesktop.NetworkManager.Device";
inline const std::string wireless = "org.freedesktop.NetworkManager.Device.Wireless";
inline const std::string accessPoint = "org.freedesktop.NetworkManager.AccessPoint";
inline const std::string active = "org.freedesktop.NetworkManager.Connection.Active";
inline const std::string connection = "org.freedesktop.NetworkManager.Settings.Connection";
inline const std::string settingsInterface = "org.freedesktop.NetworkManager.Settings";
inline const std::string properties = "org.freedesktop.DBus.Properties";
inline const std::string objectManager = "org.freedesktop.DBus.ObjectManager";

inline Variant path(const std::string& path)
{
    return Variant(ObjectPath{path.empty() ? std::string("/") : path});
}

inline Variant paths(const std::vector<std::string>& list)
{
    std::vector<ObjectPath> paths;
    for (const std::string& path : list)
        paths.push_back(ObjectPath{path});
    return Variant(paths);
}

inline Variant bytes(const std::string& text)
{
    return Variant(std::vector<uint8_t>(text.begin(), text.end()));
}

struct Call
{
    std::string member;
    std::string path;
    std::string signature;
    Variant::Items arguments;
};

// NetworkManager's completion of a profile for the access point it is added
// and activated with (nm_wifi_utils_complete_connection() for an
// infrastructure AP), then its 802-1x verify(): empty for the completed
// profile, or the error NetworkManager would refuse it with.
inline std::string completeForAccessPoint(Settings& profile, const VariantMap& ap)
{
    using namespace Lingmo::Networkmanager::variants;
    constexpr uint32_t privacy = 0x1;
    constexpr uint32_t keyPsk = 0x100, key8021x = 0x200, keySae = 0x400, keyOwe = 0x800 | 0x1000, keySuiteB = 0x2000;

    VariantMap& wifi = profile["802-11-wireless"];
    const std::string ssid = toBytes(value(ap, "Ssid"));
    if (!contains(wifi, "ssid"))
        wifi["ssid"] = bytes(ssid);
    else if (toBytes(value(wifi, "ssid")) != ssid)
        return "802-11-wireless.ssid: connection does not match access point";
    const std::string mode = contains(wifi, "mode") ? toString(value(wifi, "mode")) : "infrastructure";
    if (mode != "infrastructure" && mode != "ap")
        return "802-11-wireless.mode: connection does not match access point";
    wifi["mode"] = Variant(mode);

    const uint32_t flags = toUInt(value(ap, "Flags"));
    const uint32_t wpa = toUInt(value(ap, "WpaFlags"));
    const uint32_t rsn = toUInt(value(ap, "RsnFlags"));
    const bool has8021x = profile.count("802-1x");

    if (!(flags & privacy) && !wpa && !rsn) {
        if (profile.count("802-11-wireless-security") || has8021x)
            return "Access point is unencrypted but setting specifies security";
        return {};
    }

    VariantMap& wsec = profile["802-11-wireless-security"];
    const std::string keyMgmt = toString(value(wsec, "key-mgmt"));
    if (!wpa && !rsn) {
        if (keyMgmt.starts_with("wpa") || keyMgmt == "sae")
            return "WEP access point but setting requires WPA";
        wsec["key-mgmt"] = Variant(std::string(has8021x || keyMgmt == "ieee8021x" ? "ieee8021x" : "none"));
    } else {
        if (keyMgmt == "ieee8021x")
            return "WPA authentication is incompatible with non-EAP (original) LEAP or Dynamic WEP";
        if (contains(wsec, "wep-key0"))
            return "WPA access point but setting requires WEP";
        if (keyMgmt == "wpa-psk") {
            if (has8021x)
                return "WPA-PSK authentication is incompatible with 802.1x";
            if (!((wpa | rsn) & keyPsk))
                return "Access point does not support PSK but setting requires it";
        }
        const bool wpaEap = keyMgmt == "wpa-eap" || keyMgmt == "wpa-eap-suite-b-192";
        if (wpaEap && !has8021x)
            return "WPA-EAP authentication requires an 802.1x setting";
        if (!keyMgmt.empty() && !wpaEap && has8021x)
            return "802.1x setting requires 'wpa-eap' key management";
        if ((wpaEap || has8021x) && !((wpa | rsn) & key8021x) && !(rsn & keySuiteB))
            return "Access point does not support 802.1x but setting requires it";

        std::string completed;
        if (has8021x)
            completed = "wpa-eap";
        else if ((rsn & keySae) && ((wpa | rsn) & keyPsk))
            completed = keyMgmt == "sae" ? keyMgmt : "wpa-psk";
        else if (keyMgmt == "sae" || (rsn & keySae))
            completed = "sae";
        else if (keyMgmt == "owe" || (rsn & keyOwe))
            completed = "owe";
        else if (keyMgmt == "wpa-psk" || ((wpa | rsn) & keyPsk))
            completed = "wpa-psk";
        else if (keyMgmt == "wpa-eap-suite-b-192" || (rsn & keySuiteB))
            completed = "wpa-eap-suite-b-192";
        else
            return "Failed to determine AP security information";
        wsec["key-mgmt"] = Variant(completed);
    }

    if (!has8021x)
        return {};
    const VariantMap& eap = profile["802-1x"];
    const std::vector<std::string> methods = toStringList(value(eap, "eap"));
    if (methods.empty())
        return "802-1x.eap: property is missing";
    for (const std::string& method : methods) {
        if (method == "tls") {
            if (!contains(eap, "client-cert") || !contains(eap, "private-key"))
                return "802-1x.client-cert: property is missing";
            continue;
        }
        if (method == "sim" || method == "otp" || method == "external")
            continue;
        if (toString(value(eap, "identity")).empty())
            return "802-1x.identity: property is missing";
        if ((method == "peap" || method == "ttls") && contains(eap, "phase2-auth") == contains(eap, "phase2-autheap"))
            return "802-1x.phase2-auth, 802-1x.phase2-autheap: exactly one property must be set";
    }
    return {};
}

class FakeNetworkManager
{
public:
    explicit FakeNetworkManager(Connection* bus)
        : m_bus(bus)
    {
        objects[managerPath][manager] = {
            {"Version", Variant(std::string("1.52.1"))},
            {"State", Variant(uint32_t(70))},
            {"NetworkingEnabled", Variant(true)},
            {"WirelessEnabled", Variant(true)},
            {"WirelessHardwareEnabled", Variant(true)},
            {"WwanEnabled", Variant(false)},
            {"WwanHardwareEnabled", Variant(false)},
            {"Devices", paths({})},
            {"ActiveConnections", paths({})},
        };
        objects[managerPath + "/Settings"][settingsInterface] = {{"Connections", paths({})}};
        sd_bus_add_fallback(m_bus->bus(), &m_slot, "/org/freedesktop", &FakeNetworkManager::dispatch, this);
    }

    ~FakeNetworkManager() { sd_bus_slot_unref(m_slot); }

    bool serve() { return m_bus->requestName(manager); }
    void stop() { m_bus->releaseName(manager); }

    // A new object, announced the way NetworkManager announces one.
    void add(const std::string& objectPath, const Interfaces& interfaces, bool announce = true)
    {
        objects[objectPath] = interfaces;
        if (interfaces.count(connection))
            appendPath(managerPath + "/Settings", settingsInterface, "Connections", objectPath, announce);
        if (interfaces.count(active))
            appendPath(managerPath, manager, "ActiveConnections", objectPath, announce);
        if (interfaces.count(device))
            appendPath(managerPath, manager, "Devices", objectPath, announce);
        if (announce)
            emitSignal("/org/freedesktop", objectManager, "InterfacesAdded",
                       {Variant(ObjectPath{objectPath}), Variant(interfaces)});
    }

    void remove(const std::string& objectPath)
    {
        std::vector<std::string> interfaces;
        for (const auto& [interface, values] : objects[objectPath])
            interfaces.push_back(interface);
        objects.erase(objectPath);
        settings.erase(objectPath);
        removePath(managerPath + "/Settings", settingsInterface, "Connections", objectPath);
        removePath(managerPath, manager, "ActiveConnections", objectPath);
        removePath(managerPath, manager, "Devices", objectPath);
        emitSignal("/org/freedesktop", objectManager, "InterfacesRemoved",
                   {Variant(ObjectPath{objectPath}), Variant(interfaces)});
    }

    void set(const std::string& objectPath, const std::string& interface, const std::string& name, const Variant& value)
    {
        objects[objectPath][interface][name] = value;
        emitSignal(objectPath, properties, "PropertiesChanged",
                   {Variant(interface), Variant(VariantMap{{name, value}}), Variant(std::vector<std::string>())});
    }

    void emitSignal(const std::string& objectPath, const std::string& interface, const std::string& name,
                    const Variant::Items& arguments)
    {
        Message signal = m_bus->newSignal(objectPath, interface, name);
        for (const Variant& argument : arguments)
            signal.appendAs(argument);
        m_bus->send(signal);
    }

    int count(const std::string& member) const
    {
        int count = 0;
        for (const Call& call : calls)
            count += call.member == member;
        return count;
    }

    const Call& last(const std::string& member) const
    {
        static const Call none;
        for (auto it = calls.rbegin(); it != calls.rend(); ++it) {
            if (it->member == member)
                return *it;
        }
        return none;
    }

    std::string addConnection(const Settings& profile)
    {
        const std::string objectPath = managerPath + "/Settings/" + std::to_string(++m_next);
        settings[objectPath] = profile;
        add(objectPath, {{connection, {{"Unsaved", Variant(false)}, {"Flags", Variant(uint32_t(0))}}}});
        return objectPath;
    }

    std::string activate(const std::string& profile, const std::string& onDevice, const std::string& specificObject)
    {
        const std::string objectPath = managerPath + "/ActiveConnection/" + std::to_string(++m_next);
        add(objectPath, {{active,
                          {{"Connection", path(profile)},
                           {"SpecificObject", path(specificObject)},
                           {"Devices", paths({onDevice})},
                           {"State", Variant(uint32_t(1))},
                           {"Type", Variant(std::string("802-11-wireless"))},
                           {"Vpn", Variant(false)}}}});
        return objectPath;
    }

    std::map<std::string, Interfaces> objects;
    std::map<std::string, Settings> settings;
    std::map<std::string, Settings> secrets;
    std::set<std::string> privateConnections;
    // Activations fail and vanish before their reply is sent.
    bool dropActivations = false;
    std::map<std::string, std::string> permissions;
    std::vector<Call> calls;

private:
    static int dispatch(sd_bus_message* raw, void* data, sd_bus_error*)
    {
        if (!sd_bus_message_is_method_call(raw, nullptr, nullptr))
            return 0;
        return static_cast<FakeNetworkManager*>(data)->handle(Message(raw));
    }

    void reply(const Message& call, const Variant::Items& values)
    {
        sd_bus_message* raw = nullptr;
        if (sd_bus_message_new_method_return(call.get(), &raw) < 0)
            return;
        Message message(raw);
        sd_bus_message_unref(raw);
        for (const Variant& value : values)
            message.appendAs(value);
        m_bus->send(message);
    }

    void replyError(const Message& call, const std::string& name, const std::string& text)
    {
        sd_bus_reply_method_errorf(call.get(), name.c_str(), "%s", text.c_str());
    }

    int handle(const Message& message)
    {
        using namespace Lingmo::Networkmanager::variants;
        const std::string member = message.member();
        const std::string objectPath = message.path();
        const Variant::Items& args = message.arguments();
        const auto argument = [&args](size_t index) { return index < args.size() ? args[index] : Variant(); };
        if (message.interface() != properties)
            calls.push_back({member, objectPath, message.signature(), args});

        if (member == "GetManagedObjects") {
            std::map<ObjectPath, Interfaces> managed;
            for (const auto& [path, interfaces] : objects)
                managed[ObjectPath{path}] = interfaces;
            reply(message, {Variant(managed)});
        } else if (member == "Set") {
            set(objectPath, toString(argument(0)), toString(argument(1)), argument(2).get<Variant>().value_or(Variant()));
            reply(message, {});
        } else if (member == "GetPermissions") {
            reply(message, {Variant(permissions)});
        } else if (member == "GetSettings") {
            if (privateConnections.count(objectPath))
                replyError(message, "org.freedesktop.NetworkManager.Settings.PermissionDenied", "Not yours");
            else
                reply(message, {Variant(settings[objectPath])});
        } else if (member == "GetSecrets") {
            reply(message, {Variant(secrets[objectPath])});
        } else if (member == "Update") {
            settings[objectPath] = argument(0).get<Settings>().value_or(Settings());
            reply(message, {});
            emitSignal(objectPath, connection, "Updated", {});
        } else if (member == "Delete") {
            reply(message, {});
            remove(objectPath);
        } else if (member == "AddConnection") {
            reply(message, {path(addConnection(argument(0).get<Settings>().value_or(Settings())))});
        } else if (member == "AddAndActivateConnection") {
            Settings partial = argument(0).get<Settings>().value_or(Settings());
            const std::string specificObject = toString(argument(2));
            const auto ap = objects.find(specificObject);
            if (ap != objects.end() && ap->second.count(accessPoint)) {
                const std::string error = completeForAccessPoint(partial, ap->second.at(accessPoint));
                if (!error.empty()) {
                    replyError(message, "org.freedesktop.NetworkManager.Settings.Connection.InvalidProperty", error);
                    return 1;
                }
            }
            const std::string profile = addConnection(partial);
            const std::string activeConnection = activate(profile, toString(argument(1)), specificObject);
            if (dropActivations)
                remove(activeConnection);
            reply(message, {path(profile), path(activeConnection)});
        } else if (member == "ActivateConnection") {
            reply(message, {path(activate(toString(argument(0)), toString(argument(1)), toString(argument(2))))});
        } else if (member == "DeactivateConnection" || member == "Disconnect" || member == "RequestScan"
                   || member == "Enable" || member == "Reapply") {
            // Recorded above; the tests drive the outcome.
            reply(message, {});
        } else {
            return 0;
        }
        return 1;
    }

    void appendPath(const std::string& owner, const std::string& interface, const std::string& name,
                    const std::string& objectPath, bool announce)
    {
        std::vector<std::string> list = Lingmo::networkmanager::variants::toStringList(objects[owner][interface][name]);
        list.push_back(objectPath);
        if (announce)
            set(owner, interface, name, paths(list));
        else
            objects[owner][interface][name] = paths(list);
    }

    void removePath(const std::string& owner, const std::string& interface, const std::string& name,
                    const std::string& objectPath)
    {
        std::vector<std::string> list = Lingmo::networkmanager::variants::toStringList(objects[owner][interface][name]);
        if (std::erase(list, objectPath))
            set(owner, interface, name, paths(list));
    }

    Connection* m_bus;
    sd_bus_slot* m_slot = nullptr;
    int m_next = 100;
};

} // namespace fakenm
