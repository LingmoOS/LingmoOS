// SPDX-License-Identifier: GPL-3.0-or-later
#include "connectivity.h"
#include "base/i18n.h"
#include "base/process.h"


namespace dbus = Lingmo::base::dbus;
using Lingmo::I18n::tr;

Connectivity::Connectivity(dbus::Connection *system, dbus::Connection *session, Open open)
    : m_session(session)
    , m_nm(system, "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
           "org.freedesktop.NetworkManager")
    , m_notification(session)
    , m_open(std::move(open))
{
    if (!m_open)
        m_open = [](const std::string &uri) {
            return Lingmo::base::Process::startDetached("xdg-open", {uri}) > 0;
        };
    m_action = m_notification.actionInvoked.connect([this](uint32_t, const std::string &action) {
        if (action == "login" || action == "default")
            openLogin();
    });
    m_failed = m_notification.failed.connect([this] { m_prompted = false; });
    m_poll.onTimeout = [this] {
        if (m_enabled && m_wireless && m_state != Full)
            checkConnectivity();
        else
            refresh();
    };
    m_properties = m_nm.onPropertiesChanged([this](const dbus::VariantMap &, const std::vector<std::string> &) {
        refresh();
    });
    if (system)
        m_owner = system->watchName("org.freedesktop.NetworkManager", [this](const std::string &owner) {
            ++m_generation;
            m_read = {};
            m_activeRead = {};
            m_check = {};
            m_poll.stop();
            apply({});
            if (!owner.empty()) {
                refresh();
                m_poll.start(std::chrono::seconds(15));
            }
        });
    if (session)
        m_notificationsOwner = session->watchName("org.freedesktop.Notifications", [this](const std::string &owner) {
            if (!owner.empty())
                announce();
        });
}

void Connectivity::refresh()
{
    const uint64_t generation = ++m_generation;
    m_activeRead = {};
    m_read = m_nm.getAllAsync([this, generation](const dbus::Reply &reply) {
        if (generation != m_generation)
            return;
        const auto properties = reply.value<dbus::VariantMap>();
        if (!properties) {
            apply({});
            return;
        }
        const auto it = properties->find("PrimaryConnection");
        const std::string path = it == properties->end() ? std::string()
                : it->second.value<dbus::ObjectPath>().path;
        if (path.empty() || path == "/") {
            apply(*properties);
            return;
        }
        dbus::Proxy active(m_nm.connection(), m_nm.service(), path,
                           "org.freedesktop.NetworkManager.Connection.Active");
        m_activeRead = active.getAllAsync([this, generation, properties = *properties](const dbus::Reply &reply) mutable {
            if (generation != m_generation)
                return;
            if (const auto active = reply.value<dbus::VariantMap>()) {
                const auto type = active->find("Type");
                const auto state = active->find("State");
                if (type != active->end() && state != active->end() && state->second.value<uint32_t>() == 2)
                    properties["PrimaryConnectionType"] = type->second;
            }
            apply(properties);
        });
    });
}

void Connectivity::checkConnectivity()
{
    const std::string primary = m_primary;
    m_nm.setTimeout(std::chrono::seconds(10));
    m_check = m_nm.callAsync("CheckConnectivity", [this, primary](const dbus::Reply &) {
        // The result belongs to a route that may have gone while checking.
        // Read an authoritative snapshot instead of applying the reply value.
        if (primary == m_primary)
            refresh();
    });
}

void Connectivity::apply(const dbus::VariantMap &properties)
{
    const auto get = [&](const char *name) {
        const auto it = properties.find(name);
        return it == properties.end() ? dbus::Variant() : it->second;
    };
    const std::string primary = get("PrimaryConnection").value<dbus::ObjectPath>().path;
    const bool enabled = get("ConnectivityCheckAvailable").value<bool>()
            && get("ConnectivityCheckEnabled").value<bool>();
    const uint32_t raw = get("Connectivity").value<uint32_t>();
    const uint32_t state = enabled && raw <= Full ? raw : Unknown;
    const bool wireless = primary != "/" && !primary.empty()
            && get("PrimaryConnectionType").value<std::string>() == "802-11-wireless";
    const bool routeChanged = primary != m_primary;
    if (routeChanged || !wireless)
        m_prompted = false;
    m_primary = primary;
    m_enabled = enabled;
    m_wireless = wireless;
    m_state = state;
    m_uri = get("ConnectivityCheckUri").value<std::string>();
    if (m_object)
        m_object->emitPropertiesChanged({"state", "checkingEnabled", "wireless"});
    if (state != Portal || !wireless)
        m_notification.close();
    announce();
    if (routeChanged && m_enabled && m_wireless)
        checkConnectivity();
}

void Connectivity::announce()
{
    if (m_state != Portal || !m_wireless || m_prompted || !m_session)
        return;
    // Do not launch a browser unsolicited: the persistent action is available
    // even when the user dismisses the notification, through Wi-Fi Settings.
    m_prompted = true;
    m_notification.setAppName(tr("Network"));
    m_notification.setIconName("network-wireless");
    m_notification.setSummary(tr("Network login required"));
    m_notification.setBody(tr("Connected to Wi-Fi. Sign in to access the Internet."));
    m_notification.setActions({"default", "", "login", tr("Open login page")});
    m_notification.show();
}

bool Connectivity::openLogin()
{
    if (m_state != Portal || !m_wireless || !m_enabled)
        return false;
    // Open the configured HTTP probe, so the network can redirect the browser.
    // Never execute a redirect target or accept file/javascript/custom schemes.
    if (!m_uri.starts_with("http://") || m_uri.size() <= 7 || m_uri.size() > 2048
        || m_uri.find_first_of("\r\n\t ") != std::string::npos)
        return false;
    return m_open(m_uri);
}

bool Connectivity::publish()
{
    if (!m_session)
        return false;
    m_object = std::make_unique<dbus::Object>(m_session, "/org/lingmo/Services/Connectivity",
                                             "org.lingmo.Services.Connectivity");
    m_object->addProperty("state", [this] { return m_state; });
    m_object->addProperty("checkingEnabled", [this] { return m_enabled; });
    m_object->addProperty("wireless", [this] { return m_wireless; });
    m_object->addMethod("openLogin", [this] { return openLogin(); });
    return m_object->publish();
}
