// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scheduleutils.h"

#include "base/file.h"
#include "base/strings.h"

#include <cmath>

using namespace std::chrono;

namespace ScheduleUtils {

static constexpr double kPi = 3.14159265358979323846;

static double radians(double degrees)
{
    return degrees * kPi / 180.0;
}

// One ISO 6709 angle of zone.tab: a sign, the degrees in `degreeDigits`
// digits, then minutes and optionally seconds.
static std::optional<double> parseAngle(std::string_view text, size_t degreeDigits)
{
    if (text.empty() || (text[0] != '+' && text[0] != '-'))
        return std::nullopt;
    const std::string_view digits = text.substr(1);
    if (digits.size() != degreeDigits + 2 && digits.size() != degreeDigits + 4)
        return std::nullopt;
    const auto degrees = Lingmo::Base::strings::toNumber<int>(digits.substr(0, degreeDigits));
    const auto minutes = Lingmo::Base::strings::toNumber<int>(digits.substr(degreeDigits, 2));
    const auto seconds = digits.size() > degreeDigits + 2
            ? Lingmo::Base::strings::toNumber<int>(digits.substr(degreeDigits + 2))
            : std::optional<int>(0);
    if (!degrees || !minutes || !seconds)
        return std::nullopt;
    const double value = *degrees + *minutes / 60.0 + *seconds / 3600.0;
    return text[0] == '-' ? -value : value;
}

Date DateTime::date() const
{
    return Date(floor<days>(zone->to_local(time)));
}

Milliseconds DateTime::timeOfDay() const
{
    const local_time<Milliseconds> local = zone->to_local(time);
    return local - floor<days>(local);
}

Zone systemZone()
{
    try {
        return current_zone();
    } catch (const std::exception &) {
        return locate_zone("UTC");
    }
}

Zone zone(const std::string &id)
{
    try {
        return locate_zone(id);
    } catch (const std::exception &) {
        return nullptr;
    }
}

DateTime currentDateTime()
{
    return {floor<Milliseconds>(system_clock::now()), systemZone()};
}

DateTime fromMSecsSinceEpoch(int64_t msecs, Zone zone)
{
    return {sys_time<Milliseconds>(Milliseconds(msecs)), zone};
}

DateTime at(Date date, Milliseconds timeOfDay, Zone zone, bool later)
{
    const local_time<Milliseconds> local = local_days(date) + timeOfDay;
    const local_info info = zone->get_info(floor<seconds>(local));
    seconds offset = info.first.offset;
    if (info.result == local_info::ambiguous && later)
        offset = info.second.offset;
    return {sys_time<Milliseconds>(local.time_since_epoch() - offset), zone};
}

Milliseconds timeOfDay(int hours, int minutes, int seconds)
{
    return std::chrono::hours(hours) + std::chrono::minutes(minutes) + std::chrono::seconds(seconds);
}

Position position(const DateTime &now, const TimesForDate &times)
{
    const Date today = now.date();
    const Times day = times(today);
    if (day.start == day.end)
        return {false, {}};

    const auto turnAt = [&now](Date date, Milliseconds time) {
        const DateTime turn = at(date, time, now.zone);
        // When the clocks go back, the first occurrence of a repeated time may have passed.
        return turn > now ? turn : at(date, time, now.zone, true);
    };
    const Milliseconds time = now.timeOfDay();
    const Date tomorrow = Date(sys_days(today) + days(1));
    if (day.start < day.end) {
        if (time < day.start)
            return {false, turnAt(today, day.start)};
        if (time < day.end)
            return {true, turnAt(today, day.end)};
        return {false, turnAt(tomorrow, times(tomorrow).start)};
    }
    if (time < day.end)
        return {true, turnAt(today, day.end)};
    if (time < day.start)
        return {false, turnAt(today, day.start)};
    return {true, turnAt(tomorrow, times(tomorrow).end)};
}

std::optional<SunTimes> sunTimes(Date date, double latitude, double longitude, Zone zone)
{
    // The fractional year, the equation of time in minutes and the solar
    // declination, from NOAA's General Solar Position Calculations.
    const int daysInYear = date.year().is_leap() ? 366 : 365;
    const int dayOfYear = int((sys_days(date) - sys_days(date.year() / January / 1)).count()) + 1;
    const double gamma = 2 * kPi / daysInYear * (dayOfYear - 1);
    const double equationOfTime = 229.18
            * (0.000075 + 0.001868 * std::cos(gamma) - 0.032077 * std::sin(gamma)
               - 0.014615 * std::cos(2 * gamma) - 0.040849 * std::sin(2 * gamma));
    const double declination = 0.006918 - 0.399912 * std::cos(gamma) + 0.070257 * std::sin(gamma)
            - 0.006758 * std::cos(2 * gamma) + 0.000907 * std::sin(2 * gamma)
            - 0.002697 * std::cos(3 * gamma) + 0.00148 * std::sin(3 * gamma);

    // The sun's centre 0.833° below the horizon allows for refraction and its radius.
    const double phi = radians(latitude);
    const double cosHourAngle = std::cos(radians(90.833)) / (std::cos(phi) * std::cos(declination))
            - std::tan(phi) * std::tan(declination);
    if (!(cosHourAngle >= -1 && cosHourAngle <= 1))
        return std::nullopt;
    const double hourAngle = std::acos(cosHourAngle) * 180.0 / kPi;

    // Minutes from UTC midnight, which may fall on the day before or after.
    const sys_time<Milliseconds> midnight = sys_days(date);
    const auto turnAt = [&](double minutes) {
        return DateTime {midnight + seconds(std::lround(minutes * 60)), zone};
    };
    return SunTimes {turnAt(720 - 4 * (longitude + hourAngle) - equationOfTime),
                     turnAt(720 - 4 * (longitude - hourAngle) - equationOfTime)};
}

std::vector<std::string> zoneTables()
{
    return {"/usr/share/zoneinfo/zone1970.tab", "/usr/share/zoneinfo/zone.tab"};
}

std::optional<Coordinates> zoneCoordinates(const std::string &zoneId,
                                           const std::vector<std::string> &tables)
{
    if (zoneId.empty())
        return std::nullopt;
    for (const std::string &path : tables) {
        const std::optional<std::string> contents = Lingmo::Base::file::read(path);
        if (!contents)
            continue;
        for (const std::string_view rawLine : Lingmo::Base::strings::splitViews(*contents, '\n')) {
            const std::string_view line = Lingmo::Base::strings::trimmed(rawLine);
            if (line.empty() || line.front() == '#')
                continue;
            // Country codes, coordinates, zone, comment.
            const std::vector<std::string_view> fields = Lingmo::Base::strings::splitViews(line, '\t');
            if (fields.size() < 3 || fields[2] != zoneId)
                continue;
            const std::string_view iso = fields[1];
            size_t split = 1;
            while (split < iso.size() && iso[split] != '+' && iso[split] != '-')
                ++split;
            const auto latitude = parseAngle(iso.substr(0, split), 2);
            const auto longitude = parseAngle(iso.substr(split), 3);
            if (latitude && longitude)
                return Coordinates {*latitude, *longitude};
        }
    }
    return std::nullopt;
}

} // namespace ScheduleUtils
