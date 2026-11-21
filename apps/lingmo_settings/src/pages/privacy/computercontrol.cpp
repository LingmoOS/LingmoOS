// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/privacy/computercontrol.h"

#include <cstdio>

using namespace Lingmo::base;

namespace settings {

static constexpr const char* kService = "org.lingmo.ComputerControl1";
static constexpr const char* kPath = "/org/lingmo/ComputerControl1";

static std::vector<ComputerControl::App> appsOf(const dbus::Reply& reply)
{
    std::vector<ComputerControl::App> apps;
    for (const dbus::VariantMap& entry : reply.value<std::vector<dbus::VariantMap>>().value_or(
                 std::vector<dbus::VariantMap>())) {
        ComputerControl::App app;
        if (const auto it = entry.find("path"); it != entry.end())
            app.path = it->second.value<std::string>();
        if (const auto it = entry.find("name"); it != entry.end())
            app.name = it->second.value<std::string>();
        if (const auto it = entry.find("app_id"); it != entry.end())
            app.appId = it->second.value<std::string>();
        if (!app.path.empty())
            apps.push_back(std::move(app));
    }
    return apps;
}

ComputerControl::ComputerControl()
    : m_control(dbus::Connection::session(), kService, kPath, kService)
{
    dbus::Connection* bus = m_control.connection();
    if (!bus)
        return;
    // Read at once, so the page has the list for its first frame.
    if (const dbus::Reply reply = m_control.call("allowedApps"); !reply.isError()) {
        m_apps = appsOf(reply);
        m_available = true;
    }
    m_changedMatch = m_control.onSignal("changed", [this](const dbus::Message&) { reload(); });
    m_watch = bus->watchName(kService, [this](const std::string& owner) {
        if (!owner.empty()) {
            reload();
        } else if (m_available) {
            m_available = false;
            m_apps.clear();
            changed.emit();
        }
    });
}

void ComputerControl::reload()
{
    m_reload = m_control.callAsync("allowedApps", [this](const dbus::Reply& reply) {
        if (reply.isError())
            return;
        m_apps = appsOf(reply);
        m_available = true;
        changed.emit();
    });
}

void ComputerControl::forget(const std::string& path)
{
    if (!m_available)
        return;
    m_forget = m_control.callAsync("forget", [](const dbus::Reply& reply) {
        if (reply.isError())
            fprintf(stderr, "Cannot forget the app: %s\n", reply.error().message.c_str());
    }, path);
}

} // namespace settings
