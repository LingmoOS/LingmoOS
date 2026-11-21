// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <compare>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

// Where a day stands on a scheduled feature's schedule, and the sunset and
// sunrise it follows. Modelled on Ash's schedule_utils::GetCurrentPosition() and
// GeolocationController, with the location taken from the time zone.
namespace ScheduleUtils {

using Milliseconds = std::chrono::milliseconds;
using Date = std::chrono::year_month_day;
using Zone = const std::chrono::time_zone *;

// A moment, and the time zone its date and time of day are read in.
struct DateTime {
    std::chrono::sys_time<Milliseconds> time;
    Zone zone = nullptr;

    bool isValid() const { return zone != nullptr; }
    int64_t toMSecsSinceEpoch() const { return time.time_since_epoch().count(); }
    Date date() const;
    // Since midnight.
    Milliseconds timeOfDay() const;
    int64_t msecsTo(const DateTime &other) const { return (other.time - time).count(); }

    bool operator==(const DateTime &other) const { return time == other.time; }
    auto operator<=>(const DateTime &other) const { return time <=> other.time; }
};

// The system's time zone, UTC when it cannot be read.
Zone systemZone();
// A zone by its IANA id; null when unknown.
Zone zone(const std::string &id);
DateTime currentDateTime();
DateTime fromMSecsSinceEpoch(int64_t msecs, Zone zone);

// The moment a wall clock shows the time of day on the date. A time the clocks
// skip counts with the offset before the change; a repeated one is its first
// occurrence, or its second with `later`.
DateTime at(Date date, Milliseconds timeOfDay, Zone zone, bool later = false);

Milliseconds timeOfDay(int hours, int minutes, int seconds = 0);

// The times of day a feature turns on and off.
struct Times {
    Milliseconds start;
    Milliseconds end;
};

using TimesForDate = std::function<Times(Date)>;

struct Position {
    // What the schedule says now.
    bool enabled = false;
    // When it next says the opposite; invalid when it never turns on.
    DateTime next;
};

// On from `start` to `end` every day, across midnight when `end` comes first.
// Equal times never turn it on.
Position position(const DateTime &now, const TimesForDate &times);

struct SunTimes {
    DateTime sunrise;
    DateTime sunset;
};

// NOAA's sunrise equation, good to a few minutes, for `date` at a latitude and
// longitude in degrees, north and east positive, as times in `zone`. Empty on a
// day the sun does not rise or does not set.
std::optional<SunTimes> sunTimes(Date date, double latitude, double longitude, Zone zone);

struct Coordinates {
    double latitude = 0;
    double longitude = 0;
};

// tzdata's zone1970.tab and zone.tab.
std::vector<std::string> zoneTables();

// Where `tables` place `zoneId`'s principal city. Empty for a zone they do not
// list, such as UTC.
std::optional<Coordinates> zoneCoordinates(const std::string &zoneId,
                                           const std::vector<std::string> &tables = zoneTables());

} // namespace ScheduleUtils

