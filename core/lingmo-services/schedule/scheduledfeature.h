// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/settings.h"
#include "base/signal.h"
#include "base/timer.h"

#include "scheduleutils.h"

// A feature that turns on and off on a schedule: from sunset to sunrise, or
// between custom times. Modelled on Ash's ScheduledFeature: where the schedule
// says otherwise it wins, and a manual toggle holds until the schedule's next
// turn, across sessions too.
class ScheduledFeature
{
public:
    // Ash's ScheduleType.
    enum Schedule {
        Never,
        SunsetToSunrise,
        Custom,
    };

    using Clock = std::function<ScheduleUtils::DateTime()>;

    // Where the feature keeps its state in services.conf. A feature without
    // custom time keys offers no Custom schedule.
    struct Keys {
        std::string group;
        std::string enabled;
        std::string schedule;
        std::string holdUntil;
        std::string customStart;
        std::string customEnd;
    };

    ScheduledFeature(const Keys &keys, Lingmo::Base::Settings &settings, Clock clock);
    virtual ~ScheduledFeature();

    ScheduledFeature(const ScheduledFeature &) = delete;
    ScheduledFeature &operator=(const ScheduledFeature &) = delete;

    // Reads the saved state and takes the schedule's position. Called once,
    // after showEnabled() can be reached: at the end of a subclass constructor.
    void start();

    bool enabled() const { return m_enabled; }

    int schedule() const { return m_schedule; }

    // Minutes after midnight.
    int customStart() const { return m_customStart; }

    int customEnd() const { return m_customEnd; }

    // The next sunset and sunrise the schedule turns at, in minutes after
    // midnight; shown by Settings for the sunset to sunrise schedule.
    int sunset() const { return m_sunset; }

    int sunrise() const { return m_sunrise; }

    // The schedule's next turn; invalid without a schedule.
    ScheduleUtils::DateTime nextCheckpoint() const { return m_next; }

    // What the schedule timer and a resume run, for tests that drive the clock.
    void checkpointForTesting() { advance(Reason::Scheduled); }

    void resumeForTesting() { advance(Reason::Reset); }

    void setEnabled(bool enabled);
    void setSchedule(int schedule);
    void setCustomStart(int minutes);
    void setCustomEnd(int minutes);

    Lingmo::Base::Signal<bool> enabledChanged;
    Lingmo::Base::Signal<int> scheduleChanged;
    Lingmo::Base::Signal<int> customStartChanged;
    Lingmo::Base::Signal<int> customEndChanged;
    Lingmo::Base::Signal<int> sunsetChanged;
    Lingmo::Base::Signal<int> sunriseChanged;

protected:
    // Shows a change of enabled() over `duration`: a second when the user or a
    // change of settings made it, a minute on a scheduled turn.
    virtual void showEnabled(std::chrono::milliseconds duration);

    template<typename T>
    void write(const std::string &key, const T &value)
    {
        m_settings.setValue(m_keys.group + '/' + key, value);
        m_settings.sync();
    }

    Lingmo::Base::Settings &m_settings;

private:
    // ScheduledFeature::RefreshReason.
    enum class Reason {
        // The schedule reached a turn.
        Scheduled,
        // The schedule, its times or the time zone changed, or the service started.
        SettingsChanged,
        // The system woke up.
        Reset,
    };

    bool hasCustomSchedule() const { return !m_keys.customStart.empty(); }
    std::string key(const std::string &name) const { return m_keys.group + '/' + name; }

    void onPrepareForSleep(bool sleeping);
    void onTimeDateChanged(const Lingmo::Base::DBus::VariantMap &changed,
                           const std::vector<std::string> &invalidated);
    void refresh(Reason reason);
    // Applies the schedule once its next turn has come; until then a manual
    // toggle holds and the timer waits for the turn.
    void advance(Reason reason);
    void arm(const ScheduleUtils::DateTime &next);
    void setEnabledInternal(bool enabled, std::chrono::milliseconds duration);
    void updateSunTimes();
    ScheduleUtils::Times timesFor(ScheduleUtils::Date date);
    ScheduleUtils::Times sunTimesFor(ScheduleUtils::Date date);

    const Keys m_keys;
    Clock m_clock;
    Lingmo::Base::Timer m_timer;
    ScheduleUtils::DateTime m_next;
    bool m_enabled = false;
    int m_schedule = Never;
    int m_customStart = 18 * 60;
    int m_customEnd = 6 * 60;
    int m_sunset = 18 * 60;
    int m_sunrise = 6 * 60;
    std::string m_zone;
    std::optional<ScheduleUtils::Coordinates> m_coordinates;
    Lingmo::Base::DBus::Match m_prepareForSleep;
    Lingmo::Base::DBus::Match m_timeDateChanged;
};

