// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/powerprofiles/power_profiles.h"

#include <algorithm>

using namespace Lingmo::Base::dbus;

namespace Lingmo::powerprofiles {

static constexpr const char* kService = "org.freedesktop.UPower.PowerProfiles";
static constexpr const char* kPath = "/org/freedesktop/UPower/PowerProfiles";

PowerProfiles::PowerProfiles(Connection* bus)
    : m_daemon(bus, kService, kPath, kService)
{
    m_propertiesMatch = m_daemon.onPropertiesChanged(
            [this](const VariantMap& changed, const std::vector<std::string>&) { update(changed); });
    load();
}

PowerProfiles::~PowerProfiles() = default;

void PowerProfiles::setActiveProfile(const std::string& profile)
{
    if (profile == m_activeProfile || std::find(m_profiles.begin(), m_profiles.end(), profile) == m_profiles.end())
        return;

    m_setCall = m_daemon.setPropertyAsync("ActiveProfile", profile, [this](const Reply& reply) {
        if (reply.isError())
            load();
    });
    m_activeProfile = profile;
    activeProfileChanged.emit();
}

void PowerProfiles::load()
{
    m_loadCall = m_daemon.getAllAsync([this](const Reply& reply) {
        if (const auto properties = reply.value<VariantMap>())
            update(*properties);
        if (!m_ready) {
            m_ready = true;
            readyChanged.emit();
        }
    });
}

void PowerProfiles::update(const VariantMap& properties)
{
    if (const auto it = properties.find("Profiles"); it != properties.end()) {
        std::vector<std::string> profiles;
        for (const VariantMap& profile : it->second.value<std::vector<VariantMap>>()) {
            if (const auto name = profile.find("Profile"); name != profile.end())
                profiles.push_back(name->second.value<std::string>());
        }
        if (profiles != m_profiles) {
            m_profiles = std::move(profiles);
            profilesChanged.emit();
        }
    }

    if (const auto it = properties.find("ActiveProfile"); it != properties.end()) {
        const std::string profile = it->second.value<std::string>();
        if (profile != m_activeProfile) {
            m_activeProfile = profile;
            activeProfileChanged.emit();
        }
    }

    if (const auto it = properties.find("PerformanceDegraded"); it != properties.end()) {
        const std::string degraded = it->second.value<std::string>();
        if (degraded != m_performanceDegraded) {
            m_performanceDegraded = degraded;
            performanceDegradedChanged.emit();
        }
    }
}

} // namespace Lingmo::powerprofiles
