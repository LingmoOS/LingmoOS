// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"

namespace settings {

// The shell's notification settings: Do Not Disturb, and how each
// application that has sent a notification is shown.
class Notifications
{
public:
    // Matches NotificationSettings::ApplicationMode in the shell.
    enum ApplicationMode {
        Banners = 0,
        CenterOnly = 1,
        Off = 2,
    };

    struct Application
    {
        std::string key;
        std::string name;
        std::string icon;
        int mode = Banners;
    };

    Notifications();

    bool doNotDisturb() const { return m_doNotDisturb; }
    void setDoNotDisturb(bool enabled);

    const std::vector<Application>& applications() const { return m_applications; }
    // The list is not updated here: the shell's applicationsChanged reloads it.
    void setApplicationMode(const std::string& key, int mode);

    Lingmo::base::Signal<> doNotDisturbChanged;
    Lingmo::base::Signal<> applicationsChanged;

private:
    void loadApplications();

    Lingmo::base::dbus::Proxy m_shell;
    Lingmo::base::dbus::Match m_doNotDisturbMatch;
    Lingmo::base::dbus::Match m_applicationsMatch;
    Lingmo::base::dbus::PendingCall m_doNotDisturbCall;
    Lingmo::base::dbus::PendingCall m_applicationsCall;
    bool m_doNotDisturb = false;
    std::vector<Application> m_applications;
};

} // namespace settings
