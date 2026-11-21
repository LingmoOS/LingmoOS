// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <string>

#include "base/dbus.h"
#include "base/signal.h"

namespace Lingmo::timedate {

// The system clock's settings as org.freedesktop.timedate1 keeps them: the
// time zone and network time. Read when made, so the first frame can show
// them, and kept up to date from the service's PropertiesChanged.
class TimeDate
{
public:
    explicit TimeDate(base::dbus::Connection* bus = base::dbus::Connection::system());
    ~TimeDate();

    TimeDate(const TimeDate&) = delete;
    TimeDate& operator=(const TimeDate&) = delete;

    bool isAvailable() const { return m_available; }

    // An IANA id such as "Asia/Singapore".
    std::string timezone() const;
    bool ntp() const;
    // Whether a network time service is installed.
    bool canNtp() const;
    bool ntpSynchronized() const;
    bool localRtc() const;

    // The service may ask polkit first. Calls back with the error, empty on
    // success; the values change once the service says so.
    void setNtp(bool enabled, std::function<void(const std::string& error)> done = {});
    void setTimezone(const std::string& timezone, std::function<void(const std::string& error)> done = {});

    // After any value changed.
    base::Signal<> changed;

private:
    template<typename T>
    T value(const char* name, const T& fallback) const;

    base::dbus::Proxy m_service;
    base::dbus::Match m_propertiesMatch;
    base::dbus::PendingCall m_ntpCall;
    base::dbus::PendingCall m_timezoneCall;
    base::dbus::VariantMap m_values;
    bool m_available = false;
};

} // namespace Lingmo::timedate
