// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scheduledfeature.h"

#include <algorithm>

using namespace std::chrono_literals;
using namespace Lingmo::Base;
using ScheduleUtils::DateTime;

// kManualAnimationDuration and kAutomaticAnimationDuration.
static constexpr std::chrono::milliseconds kManualDuration = 1s;
static constexpr std::chrono::milliseconds kScheduledDuration = 60s;

// Ash's defaults: a custom schedule from 6 PM to 6 AM, and the same times for
// sunset and sunrise where the location is unknown or the sun does not set.
static constexpr int kDefaultStart = 18 * 60;
static constexpr int kDefaultEnd = 6 * 60;

static constexpr int kMinutesPerDay = 24 * 60;

static ScheduleUtils::Milliseconds timeOfDay(int minutes)
{
    return std::chrono::minutes(minutes);
}

static int minutesOfDay(int minutes)
{
    return (minutes % kMinutesPerDay + kMinutesPerDay) % kMinutesPerDay;
}

static int minutesOfDay(ScheduleUtils::Milliseconds time)
{
    return int(std::chrono::floor<std::chrono::minutes>(time).count());
}

ScheduledFeature::ScheduledFeature(const Keys &keys, Settings &settings, Clock clock)
    : m_settings(settings)
    , m_keys(keys)
    , m_clock(std::move(clock))
{
    m_enabled = m_settings.boolValue(key(m_keys.enabled), false);
    m_schedule = std::clamp(int(m_settings.intValue(key(m_keys.schedule), Never)), int(Never),
                            int(hasCustomSchedule() ? Custom : SunsetToSunrise));
    if (hasCustomSchedule()) {
        m_customStart = minutesOfDay(int(m_settings.intValue(key(m_keys.customStart), kDefaultStart)));
        m_customEnd = minutesOfDay(int(m_settings.intValue(key(m_keys.customEnd), kDefaultEnd)));
    }

    m_timer.setSingleShot(true);
    m_timer.onTimeout = [this] {
        advance(Reason::Scheduled);
    };

    // Timers do not run while the system sleeps, and the time zone moves every turn.
    if (dbus::Connection *bus = dbus::Connection::system()) {
        const dbus::Proxy logind(bus, "org.freedesktop.login1", "/org/freedesktop/login1",
                                 "org.freedesktop.login1.Manager");
        m_prepareForSleep = logind.onSignal("PrepareForSleep", [this](const dbus::Message &message) {
            if (const auto sleeping = message.argument<bool>())
                onPrepareForSleep(*sleeping);
        });
        const dbus::Proxy timedate(bus, "org.freedesktop.timedate1", "/org/freedesktop/timedate1",
                                   "org.freedesktop.timedate1");
        m_timeDateChanged = timedate.onPropertiesChanged(
                [this](const dbus::VariantMap &changed, const std::vector<std::string> &invalidated) {
                    onTimeDateChanged(changed, invalidated);
                });
    }
}

ScheduledFeature::~ScheduledFeature() = default;

void ScheduledFeature::start()
{
    // The schedule decides how a session starts, unless a manual toggle from
    // an earlier session still waits for its turn.
    const std::string hold = key(m_keys.holdUntil);
    if (m_schedule != Never && m_settings.contains(hold)
        && m_clock().toMSecsSinceEpoch() < m_settings.intValue(hold)) {
        updateSunTimes();
        arm(ScheduleUtils::fromMSecsSinceEpoch(m_settings.intValue(hold), m_clock().zone));
    } else {
        refresh(Reason::SettingsChanged);
    }
}

void ScheduledFeature::setEnabled(bool enabled)
{
    // The schedule keeps its turn: a manual toggle holds until then, across
    // sessions too.
    if (m_next.isValid() && enabled != m_enabled)
        write(m_keys.holdUntil, m_next.toMSecsSinceEpoch());
    setEnabledInternal(enabled, kManualDuration);
}

void ScheduledFeature::setSchedule(int schedule)
{
    if (schedule < Never || schedule > (hasCustomSchedule() ? Custom : SunsetToSunrise)
        || schedule == m_schedule)
        return;
    m_schedule = schedule;
    write(m_keys.schedule, schedule);
    scheduleChanged.emit(schedule);
    refresh(Reason::SettingsChanged);
}

void ScheduledFeature::setCustomStart(int minutes)
{
    minutes = minutesOfDay(minutes);
    if (!hasCustomSchedule() || minutes == m_customStart)
        return;
    m_customStart = minutes;
    write(m_keys.customStart, minutes);
    customStartChanged.emit(minutes);
    if (m_schedule == Custom)
        refresh(Reason::SettingsChanged);
}

void ScheduledFeature::setCustomEnd(int minutes)
{
    minutes = minutesOfDay(minutes);
    if (!hasCustomSchedule() || minutes == m_customEnd)
        return;
    m_customEnd = minutes;
    write(m_keys.customEnd, minutes);
    customEndChanged.emit(minutes);
    if (m_schedule == Custom)
        refresh(Reason::SettingsChanged);
}

void ScheduledFeature::showEnabled(std::chrono::milliseconds)
{
}

void ScheduledFeature::onPrepareForSleep(bool sleeping)
{
    if (!sleeping)
        advance(Reason::Reset);
}

void ScheduledFeature::onTimeDateChanged(const dbus::VariantMap &changed,
                                         const std::vector<std::string> &invalidated)
{
    const std::string timezone = "Timezone";
    if (!changed.contains(timezone)
        && std::find(invalidated.begin(), invalidated.end(), timezone) == invalidated.end())
        return;
    m_zone.clear();
    refresh(Reason::SettingsChanged);
}

// ScheduledFeature::RefreshScheduleTimer: where the schedule says otherwise,
// it wins; a change of settings takes a second to show, a scheduled turn a minute.
void ScheduledFeature::refresh(Reason reason)
{
    const std::string hold = key(m_keys.holdUntil);
    if (m_settings.contains(hold)) {
        m_settings.remove(hold);
        m_settings.sync();
    }
    updateSunTimes();
    if (m_schedule == Never) {
        arm({});
        return;
    }
    const auto position = ScheduleUtils::position(m_clock(), [this](ScheduleUtils::Date date) {
        return timesFor(date);
    });
    setEnabledInternal(position.enabled,
                       reason == Reason::Scheduled ? kScheduledDuration : kManualDuration);
    arm(position.next);
}

// ScheduledFeature::SuspendDone keeps a manual toggle through the sleep unless
// the turn it waited for passed meanwhile. A timer that fires before its turn,
// the clock having been set back, waits for it the same way.
void ScheduledFeature::advance(Reason reason)
{
    if (m_next.isValid() && m_clock() < m_next) {
        arm(m_next);
        return;
    }
    refresh(reason);
}

void ScheduledFeature::arm(const DateTime &next)
{
    m_next = next;
    if (!next.isValid()) {
        m_timer.stop();
        return;
    }
    m_timer.start(std::chrono::milliseconds(std::max<int64_t>(m_clock().msecsTo(next), 0)));
}

void ScheduledFeature::setEnabledInternal(bool enabled, std::chrono::milliseconds duration)
{
    if (enabled == m_enabled)
        return;
    m_enabled = enabled;
    write(m_keys.enabled, enabled);
    showEnabled(duration);
    enabledChanged.emit(enabled);
}

void ScheduledFeature::updateSunTimes()
{
    const DateTime now = m_clock();
    const ScheduleUtils::Date date = now.date();
    const ScheduleUtils::Times today = sunTimesFor(date);
    const ScheduleUtils::Times tomorrow =
            sunTimesFor(ScheduleUtils::Date(std::chrono::sys_days(date) + std::chrono::days(1)));
    const ScheduleUtils::Milliseconds time = now.timeOfDay();
    const int sunset = minutesOfDay(today.start > time ? today.start : tomorrow.start);
    const int sunrise = minutesOfDay(today.end > time ? today.end : tomorrow.end);
    if (sunset != m_sunset) {
        m_sunset = sunset;
        sunsetChanged.emit(sunset);
    }
    if (sunrise != m_sunrise) {
        m_sunrise = sunrise;
        sunriseChanged.emit(sunrise);
    }
}

ScheduleUtils::Times ScheduledFeature::timesFor(ScheduleUtils::Date date)
{
    if (m_schedule == Custom)
        return {timeOfDay(m_customStart), timeOfDay(m_customEnd)};
    return sunTimesFor(date);
}

ScheduleUtils::Times ScheduledFeature::sunTimesFor(ScheduleUtils::Date date)
{
    const ScheduleUtils::Zone zone = ScheduleUtils::systemZone();
    const std::string zoneId(zone->name());
    if (zoneId != m_zone) {
        m_zone = zoneId;
        m_coordinates = ScheduleUtils::zoneCoordinates(zoneId);
    }
    if (m_coordinates) {
        const auto sun = ScheduleUtils::sunTimes(date, m_coordinates->latitude,
                                                 m_coordinates->longitude, zone);
        if (sun)
            return {sun->sunset.timeOfDay(), sun->sunrise.timeOfDay()};
    }
    return {timeOfDay(kDefaultStart), timeOfDay(kDefaultEnd)};
}
