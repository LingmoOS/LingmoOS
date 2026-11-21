// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/datetime/timezonemap.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include <sstream>
#include <unistd.h>

#include <unicode/unistr.h>

#include "base/file.h"
#include "base/i18n.h"
#include "base/locale.h"
#include "lingmo/timedate/time_date.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trn;

namespace settings {

// Miller's cylindrical projection: the height of a latitude on the map,
// in units of the earth's radius.
static double millerY(double latitude)
{
    return 1.25 * std::log(std::tan(M_PI / 4 + 0.4 * latitude * M_PI / 180));
}

// The world map's top edge lies 81/180 of the projection's full height above
// the equator, its bottom edge at 59 degrees south, and its left edge at 168
// degrees west; positions are fractions of its size.
static double mapY(double latitude)
{
    const double top = 2 * millerY(90) * 81 / 180;
    return (top - millerY(latitude)) / (top - millerY(-59));
}

static double mapX(double longitude)
{
    return (longitude + 168) / 360;
}

// One coordinate of zone.tab's ISO 6709 location, sign then degrees, minutes
// and optional seconds: "+3130" or "-0583506", with `degreeDigits` of 2 for a
// latitude and 3 for a longitude.
static double iso6709Degrees(const std::string& text, size_t degreeDigits)
{
    if (text.empty() || (text[0] != '+' && text[0] != '-'))
        return 0.0;
    const size_t digits = text.size() - 1;
    if (digits != degreeDigits + 2 && digits != degreeDigits + 4)
        return 0.0;

    const auto field = [&text](size_t at, size_t length) { return std::atoi(text.substr(at, length).c_str()); };
    const double minutes = field(1 + degreeDigits, 2);
    const double seconds = digits == degreeDigits + 4 ? field(3 + degreeDigits, 2) : 0;
    const double degrees = field(1, degreeDigits) + minutes / 60 + seconds / 3600;
    return text[0] == '-' ? -degrees : degrees;
}

static const std::chrono::time_zone* locate(const std::string& id)
{
    try {
        return std::chrono::locate_zone(id);
    } catch (const std::runtime_error&) {
        return nullptr;
    }
}

static int offsetAt(const std::string& id, std::chrono::sys_seconds time)
{
    const std::chrono::time_zone* zone = locate(id);
    return zone ? int(zone->get_info(time).offset.count()) : 0;
}

static std::chrono::sys_seconds now()
{
    return std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
}

static icu::UnicodeString folded(const std::string& text)
{
    icu::UnicodeString string = icu::UnicodeString::fromUTF8(text);
    string.foldCase();
    return string;
}

static bool startsWithFolded(const std::string& text, const icu::UnicodeString& needle)
{
    return folded(text).startsWith(needle);
}

static bool containsFolded(const std::string& text, const icu::UnicodeString& needle)
{
    return folded(text).indexOf(needle) >= 0;
}

static std::string trimmed(const std::string& text)
{
    const size_t start = text.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos)
        return {};
    return text.substr(start, text.find_last_not_of(" \t\n\r\f\v") - start + 1);
}

// The zone /etc/localtime names.
static std::string systemTimeZoneId()
{
    char target[512];
    const ssize_t size = readlink("/etc/localtime", target, sizeof(target) - 1);
    if (size <= 0)
        return {};
    const std::string path(target, size_t(size));
    const size_t at = path.find("zoneinfo/");
    return at == std::string::npos ? std::string() : path.substr(at + 9);
}

TimeZoneMap::TimeZoneMap(timedate::TimeDate& timeDate)
    : m_timeDate(timeDate)
{
    if (const auto contents = base::file::read("/usr/share/zoneinfo/zone.tab")) {
        std::istringstream lines(*contents);
        std::string line;
        while (std::getline(lines, line)) {
            if (line.starts_with('#'))
                continue;
            std::vector<std::string> parts;
            std::istringstream fields(line);
            std::string field;
            while (std::getline(fields, field, '\t'))
                parts.push_back(field);
            if (parts.size() < 3)
                continue;

            const std::string& coordinate = parts[1];
            size_t index = coordinate.find('+', 3);
            if (index == std::string::npos)
                index = coordinate.find('-', 3);
            m_list.push_back({parts[0], parts[2], iso6709Degrees(coordinate.substr(0, index), 2),
                              index == std::string::npos ? 0.0 : iso6709Degrees(coordinate.substr(index), 3)});
        }
    }

    m_currentTimeZone = systemTimeZoneId();
    if (m_currentTimeZone.empty() || !locate(m_currentTimeZone))
        m_currentTimeZone = m_timeDate.timezone();
    m_timeDateChanged = m_timeDate.changed.connect([this] { setCurrentTimeZone(m_timeDate.timezone()); });
}

TimeZoneMap::~TimeZoneMap() = default;

const TimeZoneMap::Item* TimeZoneMap::find(const std::string& id) const
{
    for (const Item& item : m_list) {
        if (item.timeZone == id)
            return &item;
    }
    return nullptr;
}

int TimeZoneMap::nearestIndex(int x, int y, int width, int height) const
{
    double minimumDistance = std::numeric_limits<double>::max();
    int nearest = -1;
    for (size_t i = 0; i < m_list.size(); ++i) {
        const Item& item = m_list[i];
        const double dx = mapX(item.longitude) * width - x;
        const double dy = mapY(item.latitude) * height - y;
        const double distance = dx * dx + dy * dy;
        if (distance < minimumDistance) {
            minimumDistance = distance;
            nearest = int(i);
        }
    }
    return nearest;
}

void TimeZoneMap::clicked(int x, int y, int width, int height)
{
    std::vector<const Item*> items;
    for (const Item& item : m_list) {
        const double dx = mapX(item.longitude) * width - x;
        const double dy = mapY(item.latitude) * height - y;
        if (dx * dx + dy * dy <= 20)
            items.push_back(&item);
    }
    if (items.empty()) {
        const int index = nearestIndex(x, y, width, height);
        if (index != -1)
            items.push_back(&m_list[size_t(index)]);
    }

    m_currentList.clear();
    for (const Item* item : items)
        m_currentList.push_back(item->timeZone);
    std::sort(m_currentList.begin(), m_currentList.end());
    availableListChanged.emit();
}

void TimeZoneMap::setTimeZone(const std::string& id)
{
    if (id.empty())
        return;
    m_timeDate.setTimezone(id, [this, id](const std::string& error) {
        if (error.empty())
            setCurrentTimeZone(id);
    });
}

void TimeZoneMap::setCurrentTimeZone(const std::string& id)
{
    if (id.empty() || id == m_currentTimeZone || !locate(id))
        return;
    m_currentTimeZone = id;
    currentTimeZoneChanged.emit();
}

void TimeZoneMap::select(const std::string& id)
{
    if (id.empty())
        return;
    m_currentList = {id};
    availableListChanged.emit();
}

PointF TimeZoneMap::position(const std::string& id, int width, int height) const
{
    if (const Item* item = find(id))
        return {mapX(item->longitude) * width, mapY(item->latitude) * height};
    return {-1, -1};
}

std::string TimeZoneMap::nearest(int x, int y, int width, int height) const
{
    const int index = nearestIndex(x, y, width, height);
    return index < 0 ? std::string() : m_list[size_t(index)].timeZone;
}

std::vector<TimeZoneMap::Point> TimeZoneMap::points() const
{
    const auto time = now();
    std::vector<Point> result;
    result.reserve(m_list.size());
    for (const Item& item : m_list) {
        result.push_back({item.timeZone, mapX(item.longitude), mapY(item.latitude),
                          offsetAt(item.timeZone, time)});
    }
    return result;
}

std::string TimeZoneMap::territoryName(const std::string& code)
{
    return base::Locale::territoryName(code);
}

std::string TimeZoneMap::shortTime(const std::string& id, int64_t utcSeconds)
{
    const std::chrono::sys_seconds time{std::chrono::seconds(utcSeconds)};
    const int64_t local = utcSeconds + offsetAt(id, time);
    const int64_t minutes = ((local / 60) % (24 * 60) + 24 * 60) % (24 * 60);
    return base::Locale::system().toString(base::ClockTime{int(minutes / 60), int(minutes % 60)},
                                           base::Locale::FormatType::Short);
}

TimeZoneMap::Info TimeZoneMap::info(const std::string& id) const
{
    const Item* found = find(id);
    if (!found)
        return {};

    // Offsets are not something most people think in: the gap to the zone in
    // use, and the day when it differs, say the same thing plainly.
    const auto time = now();
    const int offset = offsetAt(found->timeZone, time);
    const int here = offsetAt(m_currentTimeZone, time);
    const int gap = offset - here;

    std::string difference;
    const int hours = std::abs(gap) / 3600;
    const int minutes = std::abs(gap) % 3600 / 60;
    if (gap == 0)
        difference = tr("Same time as now");
    else if (minutes == 0)
        difference = gap > 0 ? trn("{n} hour ahead", "{n} hours ahead", hours)
                             : trn("{n} hour behind", "{n} hours behind", hours);
    else if (hours == 0)
        difference = gap > 0 ? trn("{n} min ahead", "{n} min ahead", minutes)
                             : trn("{n} min behind", "{n} min behind", minutes);
    else
        difference = format(gap > 0 ? tr("{hours} h {minutes} min ahead") : tr("{hours} h {minutes} min behind"),
                            {{"hours", std::to_string(hours)}, {"minutes", std::to_string(minutes)}});

    const auto day = [](int64_t seconds) { return int64_t(std::floor(double(seconds) / 86400.0)); };
    const int64_t utc = time.time_since_epoch().count();
    std::string dayText;
    const int64_t days = day(utc + offset) - day(utc + here);
    if (days > 0)
        dayText = tr("Tomorrow");
    else if (days < 0)
        dayText = tr("Yesterday");

    return {found->timeZone, localeTimeZoneName(found->timeZone), territoryName(found->country), offset,
            shortTime(found->timeZone, utc), dayText, difference};
}

std::vector<TimeZoneMap::Info> TimeZoneMap::search(const std::string& text, int limit) const
{
    const std::string trimmedText = trimmed(text);
    if (trimmedText.empty())
        return {};
    const icu::UnicodeString needle = folded(trimmedText);

    struct Match
    {
        int rank;
        const Item* item;
    };
    std::vector<Match> matches;
    for (const Item& item : m_list) {
        // "New_York" is typed as "new york"; the country code as "US".
        std::string id = item.timeZone;
        std::replace(id.begin(), id.end(), '_', ' ');
        const std::string city = localeTimeZoneName(id);
        const std::string country = territoryName(item.country);
        int rank = -1;
        if (startsWithFolded(city, needle))
            rank = 0;
        else if (startsWithFolded(id, needle))
            rank = 1;
        else if (containsFolded(city, needle) || containsFolded(id, needle))
            rank = 2;
        else if (startsWithFolded(country, needle) || folded(item.country) == needle)
            rank = 3;
        if (rank >= 0)
            matches.push_back({rank, &item});
    }
    std::stable_sort(matches.begin(), matches.end(), [](const Match& a, const Match& b) {
        return a.rank != b.rank ? a.rank < b.rank : a.item->timeZone < b.item->timeZone;
    });

    std::vector<Info> result;
    for (const Match& match : matches) {
        if (int(result.size()) >= limit)
            break;
        result.push_back(info(match.item->timeZone));
    }
    return result;
}

std::string TimeZoneMap::currentTimeZone() const
{
    return localeTimeZoneName(m_currentTimeZone);
}

std::string TimeZoneMap::localeTimeZoneName(const std::string& id)
{
    const size_t index = id.rfind('/');
    std::string city = index == std::string::npos ? id : id.substr(index + 1);
    // "Los_Angeles" is written that way in the database only.
    std::replace(city.begin(), city.end(), '_', ' ');
    return city;
}

} // namespace settings
