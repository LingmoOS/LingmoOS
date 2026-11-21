// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/notifications/notifications.h"

#include <cstdint>

using namespace Lingmo::base;

namespace settings {

Notifications::Notifications()
    : m_shell(dbus::Connection::session(), "org.lingmo.Notification", "/Notification",
              "org.lingmo.Notification")
{
    m_doNotDisturb = m_shell.property<bool>("doNotDisturb").value_or(false);
    m_doNotDisturbMatch = m_shell.onSignal("doNotDisturbChanged", [this](const dbus::Message&) {
        m_doNotDisturbCall = m_shell.getPropertyAsync("doNotDisturb", [this](const dbus::Reply& reply) {
            const auto value = reply.value<dbus::Variant>();
            m_doNotDisturb = value ? value->value<bool>(false) : false;
            doNotDisturbChanged.emit();
        });
    });
    m_applicationsMatch = m_shell.onSignal("applicationsChanged", [this](const dbus::Message&) {
        loadApplications();
    });
    loadApplications();
}

void Notifications::setDoNotDisturb(bool enabled)
{
    m_doNotDisturb = enabled;
    m_shell.send("setDoNotDisturb", enabled);
    doNotDisturbChanged.emit();
}

void Notifications::setApplicationMode(const std::string& key, int mode)
{
    m_shell.send("setApplicationMode", key, int32_t(mode));
}

void Notifications::loadApplications()
{
    m_applicationsCall = m_shell.callAsync("applications", [this](const dbus::Reply& reply) {
        const auto list = reply.value<std::vector<dbus::VariantMap>>();
        if (!list)
            return;
        m_applications.clear();
        for (const dbus::VariantMap& entry : *list) {
            Application application;
            const auto text = [&entry](const char* key) {
                const auto it = entry.find(key);
                return it == entry.end() ? std::string() : it->second.value<std::string>();
            };
            application.key = text("key");
            application.name = text("name");
            application.icon = text("icon");
            if (const auto it = entry.find("mode"); it != entry.end())
                application.mode = it->second.value<int32_t>(Banners);
            m_applications.push_back(std::move(application));
        }
        applicationsChanged.emit();
    });
}

} // namespace settings
