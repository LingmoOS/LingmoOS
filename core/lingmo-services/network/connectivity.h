// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "base/dbus.h"
#include "base/timer.h"
#include "notification.h"
#include <functional>
#include <memory>
#include <string>

// Connectivity is a property of the primary route, not proof that every
// activated Wi-Fi interface can reach the Internet.
class Connectivity
{
public:
    enum State : uint32_t { Unknown, None, Portal, Limited, Full };
    using Open = std::function<bool(const std::string &)>;
    Connectivity(Lingmo::base::dbus::Connection *system, Lingmo::base::dbus::Connection *session, Open open = {});
    bool publish();
    bool openLogin();
    void refresh();
    uint32_t state() const { return m_state; }
    bool checkingEnabled() const { return m_enabled; }
    bool wireless() const { return m_wireless; }
private:
    void apply(const Lingmo::base::dbus::VariantMap &properties);
    void announce();
    void checkConnectivity();
    Lingmo::base::dbus::Connection *m_session;
    Lingmo::base::dbus::Proxy m_nm;
    Notification m_notification;
    Open m_open;
    uint32_t m_state = Unknown;
    bool m_enabled = false;
    bool m_wireless = false;
    bool m_prompted = false;
    std::string m_primary;
    std::string m_uri;
    uint64_t m_generation = 0;
    Lingmo::base::Timer m_poll;
    Lingmo::base::Subscription m_action;
    Lingmo::base::dbus::Match m_properties;
    Lingmo::base::dbus::NameWatch m_owner;
    Lingmo::base::dbus::NameWatch m_notificationsOwner;
    Lingmo::base::dbus::PendingCall m_read;
    Lingmo::base::dbus::PendingCall m_activeRead;
    Lingmo::base::dbus::PendingCall m_check;
    Lingmo::base::Subscription m_failed;
    std::unique_ptr<Lingmo::base::dbus::Object> m_object;
};
