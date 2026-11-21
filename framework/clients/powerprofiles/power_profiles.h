// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"

namespace Lingmo::powerprofiles {

// power-profiles-daemon's profiles, read asynchronously and kept up to date
// from its PropertiesChanged, so the getters never block.
class PowerProfiles
{
public:
    explicit PowerProfiles(base::dbus::Connection* bus = base::dbus::Connection::system());
    ~PowerProfiles();

    PowerProfiles(const PowerProfiles&) = delete;
    PowerProfiles& operator=(const PowerProfiles&) = delete;

    // Whether the daemon has answered once.
    bool isReady() const { return m_ready; }

    // The profile names in the daemon's order, e.g. "power-saver", "balanced".
    const std::vector<std::string>& profiles() const { return m_profiles; }
    const std::string& activeProfile() const { return m_activeProfile; }
    // Why the performance profile holds back, e.g. "lap-detected"; empty when
    // it does not.
    const std::string& performanceDegraded() const { return m_performanceDegraded; }

    // Shown as active at once; a refused switch, which announces nothing,
    // reads the daemon's profile back.
    void setActiveProfile(const std::string& profile);

    base::Signal<> readyChanged;
    base::Signal<> profilesChanged;
    base::Signal<> activeProfileChanged;
    base::Signal<> performanceDegradedChanged;

private:
    void load();
    void update(const base::dbus::VariantMap& properties);

    base::dbus::Proxy m_daemon;
    base::dbus::Match m_propertiesMatch;
    base::dbus::PendingCall m_loadCall;
    base::dbus::PendingCall m_setCall;
    bool m_ready = false;
    std::vector<std::string> m_profiles;
    std::string m_activeProfile;
    std::string m_performanceDegraded;
};

} // namespace Lingmo::powerprofiles
