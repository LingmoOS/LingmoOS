// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/vpn_notifier.h"

#include "base/i18n.h"

using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace Lingmo::Shell {

namespace dbus = base::dbus;

static constexpr char kService[] = "org.freedesktop.Notifications";
static constexpr char kPath[] = "/org/freedesktop/Notifications";

VpnNotifier::VpnNotifier(dbus::Connection *bus)
    : m_bus(bus)
    , m_server(bus, kService, kPath, kService)
{
    if (!m_bus)
        return;
    dbus::MatchRule rule;
    rule.sender = kService;
    rule.path = kPath;
    rule.interface = kService;
    rule.member = "ActionInvoked";
    m_matches.push_back(m_bus->subscribe(rule, [this](const dbus::Message &message) {
        uint32_t id = 0;
        std::string key;
        if (!message.read(id, key) || !m_shown.count(id))
            return;
        if (openSettings)
            openSettings();
    }));
    rule.member = "NotificationClosed";
    m_matches.push_back(m_bus->subscribe(rule, [this](const dbus::Message &message) {
        uint32_t id = 0;
        uint32_t reason = 0;
        if (message.read(id, reason))
            m_shown.erase(id);
    }));
}

VpnNotifier::~VpnNotifier() = default;

void VpnNotifier::vpnFailed(const std::string &name, const std::string &message)
{
    if (!m_bus)
        return;
    dbus::VariantMap hints;
    hints["urgency"] = dbus::Variant(uint8_t(1));
    hints["desktop-entry"] = dbus::Variant(std::string("lingmo-settings"));
    // TRANSLATORS: {reason} is a sentence of its own, such as "The VPN service stopped."
    const std::string body = format(tr("“{name}” could not connect. {reason}"), {{"name", name}, {"reason", message}});
    m_calls.track([&](std::function<void()> finished) {
        return m_server.callAsync(
                "Notify",
                [this, finished](const dbus::Reply &reply) {
                    uint32_t id = 0;
                    if (reply.read(id) && id)
                        m_shown.insert(id);
                    finished();
                },
                std::string(), uint32_t(0), std::string("network-vpn"), std::string(tr("VPN connection failed")), body,
                std::vector<std::string> {"default", ""}, hints, int32_t(-1));
    });
}

} // namespace Lingmo::Shell
