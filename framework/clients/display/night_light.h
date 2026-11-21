// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "base/dbus.h"
#include "base/signal.h"

namespace Lingmo::Display {

// The night light lingmo-services keeps. Its state is read once, without
// blocking, and then follows the service's change signals; a setter shows its
// value at once and the service's answer settles it.
class NightLight
{
public:
    enum Schedule {
        Never,
        SunsetToSunrise,
        Custom,
    };

    NightLight();
    ~NightLight();

    NightLight(const NightLight&) = delete;
    NightLight& operator=(const NightLight&) = delete;

    bool loaded() const { return m_loaded; }

    // Whether a display can be tinted at all.
    bool available() const { return m_available; }

    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled);

    // How warm, from 0 to 1.
    double temperature() const { return m_temperature; }
    void setTemperature(double temperature);

    int schedule() const { return m_schedule; }
    void setSchedule(int schedule);

    // Minutes after midnight.
    int customStart() const { return m_customStart; }
    void setCustomStart(int minutes);

    int customEnd() const { return m_customEnd; }
    void setCustomEnd(int minutes);

    // The next sunset and sunrise the schedule turns at, in minutes after midnight.
    int sunset() const { return m_sunset; }
    int sunrise() const { return m_sunrise; }

    // Shows the temperature while the night light is off; the service ends it
    // if this process leaves the bus.
    bool preview() const { return m_preview; }
    void setPreview(bool preview);

    // Any value above changed.
    base::Signal<> changed;

private:
    void load();
    template<typename T>
    void update(T& member, T value);

    base::dbus::Proxy m_service;
    base::dbus::NameWatch m_watch;
    base::dbus::PendingCall m_load;
    std::vector<base::dbus::Match> m_matches;
    bool m_loaded = false;
    bool m_available = false;
    bool m_enabled = false;
    double m_temperature = 0.5;
    int m_schedule = Never;
    int m_customStart = 18 * 60;
    int m_customEnd = 6 * 60;
    bool m_preview = false;
    int m_sunset = 18 * 60;
    int m_sunrise = 6 * 60;
};

} // namespace Lingmo::Display
