// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/timedate/time_date.h"

#include <chrono>
#include <cstdio>

using namespace Lingmo::Base;

namespace Lingmo::timedate {

static constexpr const char* kService = "org.freedesktop.timedate1";
static constexpr const char* kPath = "/org/freedesktop/timedate1";
static constexpr const char* kInterface = "org.freedesktop.timedate1";

TimeDate::TimeDate(dbus::Connection* bus)
    : m_service(bus, kService, kPath, kInterface)
{
    if (!bus)
        return;
    // timedated only asks the polkit agent when the message itself allows it,
    // and the reply then waits for the password.
    m_service.setInteractiveAuthorizationAllowed(true);
    m_service.setTimeout(std::chrono::seconds(120));
    m_propertiesMatch = m_service.onPropertiesChanged(
            [this](const dbus::VariantMap& changedValues, const std::vector<std::string>&) {
                for (const auto& [name, value] : changedValues)
                    m_values[name] = value;
                changed.emit();
            });
    if (const dbus::Reply reply = m_service.getAll(); !reply.isError()) {
        m_values = reply.value<dbus::VariantMap>().value_or(dbus::VariantMap());
        m_available = true;
    }
}

TimeDate::~TimeDate() = default;

template<typename T>
T TimeDate::value(const char* name, const T& fallback) const
{
    const auto it = m_values.find(name);
    return it == m_values.end() ? fallback : it->second.value<T>(fallback);
}

std::string TimeDate::timezone() const
{
    return value<std::string>("Timezone", {});
}

bool TimeDate::ntp() const
{
    return value<bool>("NTP", false);
}

bool TimeDate::canNtp() const
{
    return value<bool>("CanNTP", false);
}

bool TimeDate::ntpSynchronized() const
{
    return value<bool>("NTPSynchronized", false);
}

bool TimeDate::localRtc() const
{
    return value<bool>("LocalRTC", false);
}

void TimeDate::setNtp(bool enabled, std::function<void(const std::string& error)> done)
{
    m_ntpCall = m_service.callAsync("SetNTP", [this, done](const dbus::Reply& reply) {
        if (reply.isError())
            fprintf(stderr, "Failed to change automatic time sync: %s\n", reply.error().message.c_str());
        if (done)
            done(reply.isError() ? reply.error().message : std::string());
    }, enabled, true);
}

void TimeDate::setTimezone(const std::string& timezone, std::function<void(const std::string& error)> done)
{
    if (timezone.empty())
        return;
    m_timezoneCall = m_service.callAsync("SetTimezone", [this, timezone, done](const dbus::Reply& reply) {
        if (reply.isError()) {
            fprintf(stderr, "Failed to set timezone: %s\n", reply.error().message.c_str());
        } else if (this->timezone() != timezone) {
            // PropertiesChanged says so too, but may come later than the reply.
            m_values["Timezone"] = timezone;
            changed.emit();
        }
        if (done)
            done(reply.isError() ? reply.error().message : std::string());
    }, timezone, true);
}

} // namespace Lingmo::timedate
