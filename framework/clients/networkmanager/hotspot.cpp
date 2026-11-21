// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/networkmanager/hotspot.h"
#include "lingmo/networkmanager/client.h"
#include "lingmo/networkmanager/variants.h"
#include "lingmo/networkmanager/wifi_utils.h"

#include <memory>
#include <unistd.h>

#include "base/event_loop.h"

namespace Lingmo::Networkmanager {

using namespace variants;

static std::string hostName()
{
    char name[256] = {};
    if (gethostname(name, sizeof(name) - 1) != 0)
        return {};
    return name;
}

HotspotConfiguration* HotspotConfiguration::instance()
{
    static thread_local std::unique_ptr<HotspotConfiguration> configuration;
    static thread_local bool hooked = false;
    // Made again when the shared client was.
    if (!configuration || configuration->m_client != Client::instance()) {
        configuration = std::make_unique<HotspotConfiguration>();
        base::EventLoop* loop = base::EventLoop::current();
        if (loop && !hooked) {
            hooked = true;
            loop->addDestroyHook([] { configuration.reset(); });
        }
    }
    return configuration.get();
}

HotspotConfiguration::HotspotConfiguration()
    : m_client(Client::instance())
    , m_generatedPassword(wifi::generatedPassword())
{
    m_subscriptions.push_back(m_client->readyChanged.connect([this] { load(); }));
    // A profile made or edited later, here or elsewhere, is read again.
    m_subscriptions.push_back(m_client->changed.connect([this] {
        if (connection() != m_loadedProfile || m_client->settings(connection()) != m_loadedSettings)
            load();
    }));
    load();
}

HotspotConfiguration::~HotspotConfiguration() = default;

std::string HotspotConfiguration::connection() const
{
    return m_client->hotspotConnection();
}

std::string HotspotConfiguration::name() const
{
    if (!m_name.empty())
        return m_name;
    const std::string profile = connection();
    if (!profile.empty())
        return wifi::ssidLabel(toBytes(m_client->setting(profile, "802-11-wireless", "ssid")));
    const std::string host = hostName();
    return host.empty() || host == "localhost" ? std::string("Hotspot") : host;
}

std::string HotspotConfiguration::password() const
{
    if (!m_password.empty())
        return m_password;
    return m_profilePassword.empty() ? m_generatedPassword : m_profilePassword;
}

void HotspotConfiguration::setName(const std::string& name)
{
    if (name == this->name())
        return;
    m_name = name;
    changed.emit();
}

void HotspotConfiguration::setPassword(const std::string& password)
{
    if (password == this->password())
        return;
    m_password = password;
    changed.emit();
}

bool HotspotConfiguration::nameValid(const std::string& name)
{
    return !name.empty() && name.size() <= 32;
}

bool HotspotConfiguration::passwordValid(const std::string& password)
{
    return wifi::pskValid(password);
}

std::string HotspotConfiguration::generatePassword()
{
    return wifi::generatedPassword();
}

// The profile keeps its password with NetworkManager, which reads it out only on request.
void HotspotConfiguration::load()
{
    const std::string profile = connection();
    m_loadedProfile = profile;
    m_loadedSettings = m_client->settings(profile);
    if (profile.empty())
        return;

    changed.emit();
    m_client->call(profile, connectionInterface, "GetSecrets", {base::dbus::Variant(std::string("802-11-wireless-security"))},
                   &m_context, [this](const base::dbus::Reply& reply) {
                       const auto secrets = reply.value<Settings>();
                       if (!secrets)
                           return;
                       const auto security = secrets->find("802-11-wireless-security");
                       if (security == secrets->end())
                           return;
                       std::string password = toString(value(security->second, "psk"));
                       if (!contains(security->second, "psk"))
                           password = toString(value(security->second, "wep-key0"));
                       if (password.empty() || password == m_profilePassword)
                           return;
                       m_profilePassword = password;
                       changed.emit();
                   });
}

} // namespace Lingmo::Networkmanager
