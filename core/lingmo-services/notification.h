// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"

// One message in the notification centre (org.freedesktop.Notifications).
// show() again replaces what it showed, until the user or close() removed it.
class Notification
{
public:
    enum Urgency : uint8_t {
        Low,
        Normal,
        Critical,
    };

    explicit Notification(Lingmo::Base::DBus::Connection *bus);
    ~Notification();

    Notification(const Notification &) = delete;
    Notification &operator=(const Notification &) = delete;

    // The name shown in the notification, when it is not the sender's
    // desktop entry that names the application.
    void setAppName(std::string name) { m_appName = std::move(name); }
    // The application it is from.
    void setDesktopEntry(std::string id) { m_desktopEntry = std::move(id); }
    void setIconName(std::string name) { m_iconName = std::move(name); }
    void setSummary(std::string summary) { m_summary = std::move(summary); }
    void setBody(std::string body) { m_body = std::move(body); }
    void setUrgency(Urgency urgency) { m_urgency = urgency; }
    // The spec's "value" hint: progress in percent, or -1 for no bar.
    void setValue(int value) { m_value = value; }
    // The spec's flat actions list: key, label, key, label, ...
    void setActions(std::vector<std::string> actions) { m_actions = std::move(actions); }
    // Notify's expire_timeout; -1 leaves it to the server, 0 never expires.
    void setTimeout(int timeout) { m_timeout = timeout; }

    void show();
    void close();

    // The user or the server removed it.
    Lingmo::Base::Signal<> closed;
    // The user pressed one of its actions.
    Lingmo::Base::Signal<uint32_t, const std::string&> actionInvoked;
    // No server took it, as early in a session.
    Lingmo::Base::Signal<> failed;

private:
    void send();
    void sendClose();

    Lingmo::Base::DBus::Connection *m_bus;
    Lingmo::Base::DBus::Proxy m_proxy;
    Lingmo::Base::DBus::Match m_closedMatch;
    Lingmo::Base::DBus::Match m_actionMatch;
    Lingmo::Base::DBus::PendingCall m_call;
    std::string m_appName;
    std::string m_desktopEntry;
    std::string m_iconName;
    std::string m_summary;
    std::string m_body;
    std::vector<std::string> m_actions;
    Urgency m_urgency = Normal;
    int m_value = -1;
    int m_timeout = -1;
    uint32_t m_id = 0;
    bool m_pending = false;
    bool m_showAgain = false;
    bool m_closeWhenShown = false;
};

