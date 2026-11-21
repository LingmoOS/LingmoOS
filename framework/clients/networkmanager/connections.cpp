// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/networkmanager/connections.h"
#include "lingmo/networkmanager/client.h"
#include "lingmo/networkmanager/variants.h"

#include <cstdio>

namespace Lingmo::Networkmanager {

using base::dbus::Variant;
using namespace variants;

EnabledConnections::EnabledConnections()
    : m_client(Client::instance())
{
    m_subscription = m_client->changed.connect([this] { update(); });
    update();
}

EnabledConnections::~EnabledConnections() = default;

void EnabledConnections::update()
{
    bool changed = false;
    const auto set = [&changed](bool& field, bool value) {
        if (field == value)
            return;
        field = value;
        changed = true;
    };

    const bool ready = m_client->ready();
    set(m_networkingEnabled, ready && toBool(m_client->manager("NetworkingEnabled")));
    set(m_wirelessEnabled, ready && toBool(m_client->manager("WirelessEnabled")));
    set(m_wirelessHwEnabled, ready && toBool(m_client->manager("WirelessHardwareEnabled")));

    bool available = false;
    bool enabled = false;
    for (const std::string& device : m_client->devices()) {
        if (m_client->deviceType(device) != DeviceEthernet
            || !toBool(m_client->property(device, deviceInterface, "Managed")))
            continue;

        available = true;

        // Disconnecting a device clears its autoconnect flag, so this tracks the user's switch.
        const uint32_t state = m_client->deviceState(device);
        if (toBool(m_client->property(device, deviceInterface, "Autoconnect"))
            || (state >= DevicePrepare && state <= DeviceActivated))
            enabled = true;
    }
    set(m_wiredAvailable, available);
    set(m_wiredEnabled, enabled);
    set(m_ready, ready);
    if (changed)
        this->changed.emit();
}

ConnectionSettings::ConnectionSettings(std::string path)
    : m_client(Client::instance())
{
    m_subscription = m_client->changed.connect([this] { refresh(); });
    setPath(path);
}

ConnectionSettings::~ConnectionSettings() = default;

void ConnectionSettings::setPath(const std::string& path)
{
    if (m_path == path)
        return;
    m_path = path;
    m_pending.clear();
    refresh();
}

void ConnectionSettings::setName(const std::string& name)
{
    if (name == m_name || !m_client->readable(m_path))
        return;
    m_name = name;
    update("id", Variant(name));
    changed.emit();
}

void ConnectionSettings::setAutoConnect(bool value)
{
    if (value == m_autoConnect || !m_client->readable(m_path))
        return;
    m_autoConnect = value;
    update("autoconnect", Variant(value));
    changed.emit();
}

void ConnectionSettings::setAvailableToOtherUsers(bool value)
{
    if (value == m_availableToOtherUsers || !m_client->readable(m_path))
        return;
    m_availableToOtherUsers = value;
    update("permissions",
           Variant(value ? std::vector<std::string>() : std::vector<std::string>{Client::userPermission()}));
    changed.emit();
}

void ConnectionSettings::update(const std::string& key, const Variant& value)
{
    // Without secrets in it, an update keeps the ones NetworkManager and the keyring hold.
    Settings settings = m_client->settings(m_path);
    settings["connection"][key] = value;
    m_pending[key] = value;
    const std::string path = m_path;
    m_client->call(m_path, connectionInterface, "Update", {Variant(settings)}, &m_context,
                   [this, path, key, value](const base::dbus::Reply& reply) {
                       if (!reply.isError())
                           return;
                       std::fprintf(stderr, "Cannot update the network: %s\n", reply.error().message.c_str());
                       // The profile keeps its old value, so show that again.
                       const auto pending = m_pending.find(key);
                       if (path == m_path && pending != m_pending.end() && pending->second == value) {
                           m_pending.erase(pending);
                           refresh();
                       }
                   },
                   Client::authorizedTimeout);
}

// The cache is re-read after NetworkManager reports the profile updated, which
// comes after any unrelated change. Until then a written value is the truth.
Variant ConnectionSettings::cached(const std::string& key, const Variant& defaultValue)
{
    const Variant value = m_client->setting(m_path, "connection", key);
    const auto pending = m_pending.find(key);
    if (pending == m_pending.end())
        return value;
    // NetworkManager leaves out a value that is the default.
    if ((value.isValid() ? value : defaultValue) != pending->second)
        return pending->second;
    m_pending.erase(pending);
    return value;
}

void ConnectionSettings::refresh()
{
    if (!m_client->readable(m_path))
        return;

    const std::string name = toString(cached("id"));
    const Variant autoConnect = cached("autoconnect", Variant(true));
    const bool available = toStringList(cached("permissions", Variant(std::vector<std::string>()))).empty();

    bool changed = false;
    if (name != m_name) {
        m_name = name;
        changed = true;
    }
    // NetworkManager leaves out a value that is the default.
    const bool automatic = !autoConnect.isValid() || toBool(autoConnect);
    if (automatic != m_autoConnect) {
        m_autoConnect = automatic;
        changed = true;
    }
    if (available != m_availableToOtherUsers) {
        m_availableToOtherUsers = available;
        changed = true;
    }
    if (changed)
        this->changed.emit();
}

} // namespace Lingmo::Networkmanager
