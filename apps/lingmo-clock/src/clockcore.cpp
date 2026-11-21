// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "clockcore.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace Clock {
void Stopwatch::start() {
    if (!m_running) {
        m_started = std::chrono::steady_clock::now();
        m_running = true;
    }
}
void Stopwatch::pause() {
    if (m_running) {
        m_accumulated += std::chrono::steady_clock::now() - m_started;
        m_running = false;
    }
}
void Stopwatch::reset() {
    m_running = false;
    m_accumulated = {};
    m_laps.clear();
}
std::chrono::nanoseconds Stopwatch::elapsed() const {
    return m_accumulated +
           (m_running ? std::chrono::steady_clock::now() - m_started : std::chrono::steady_clock::duration{});
}
std::chrono::nanoseconds Stopwatch::lap() {
    auto value = elapsed();
    m_laps.push_back(value);
    return value;
}

std::string formatDuration(std::chrono::nanoseconds value, bool centiseconds) {
    const auto total = std::max<int64_t>(0, value.count());
    const int64_t cs = total / 10'000'000;
    std::ostringstream out;
    out << std::setfill('0') << std::setw(2) << (cs / 360000) << ':' << std::setw(2) << ((cs / 6000) % 60) << ':'
        << std::setw(2) << ((cs / 100) % 60);
    if (centiseconds)
        out << '.' << std::setw(2) << (cs % 100);
    return out.str();
}

std::string localTime(const std::string& zone, std::chrono::system_clock::time_point now) {
    const auto parts = localTimeParts(zone, now);
    std::ostringstream out;
    out << std::setfill('0') << std::setw(2) << parts[0] << ':' << std::setw(2) << parts[1];
    return out.str();
}

static long offsetSeconds(const std::string& zone, std::chrono::system_clock::time_point now) {
    const char* previous = std::getenv("TZ");
    const std::string saved = previous ? previous : "";
    if (!zone.empty()) {
        setenv("TZ", zone.c_str(), 1);
        tzset();
    }
    const auto raw = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&raw, &tm);
    const long offset = tm.tm_gmtoff;
    if (!zone.empty()) {
        if (previous)
            setenv("TZ", saved.c_str(), 1);
        else
            unsetenv("TZ");
        tzset();
    }
    return offset;
}

std::string utcOffset(const std::string& zone, std::chrono::system_clock::time_point now) {
    const long offset = offsetSeconds(zone, now);
    const char sign = offset < 0 ? '-' : '+';
    const long absolute = std::labs(offset);
    std::ostringstream out;
    out << "UTC" << sign << std::setfill('0') << std::setw(2) << absolute / 3600 << ':' << std::setw(2)
        << (absolute % 3600) / 60;
    return out.str();
}
std::string timeDifference(const std::string& zone, std::chrono::system_clock::time_point now) {
    const long minutes = (offsetSeconds(zone, now) - offsetSeconds("", now)) / 60;
    const char sign = minutes < 0 ? '-' : '+';
    const long absolute = std::labs(minutes);
    std::ostringstream out;
    out << sign << absolute / 60;
    if (absolute % 60)
        out << ':' << std::setfill('0') << std::setw(2) << absolute % 60;
    out << ' ' << "h";
    return out.str();
}

std::array<int, 3> localTimeParts(const std::string& zone, std::chrono::system_clock::time_point now) {
    const char* previous = std::getenv("TZ");
    const std::string saved = previous ? previous : "";
    if (!zone.empty()) {
        setenv("TZ", zone.c_str(), 1);
        tzset();
    }
    const std::time_t raw = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&raw, &tm);
    if (!zone.empty()) {
        if (previous)
            setenv("TZ", saved.c_str(), 1);
        else
            unsetenv("TZ");
        tzset();
    }
    return {tm.tm_hour, tm.tm_min, tm.tm_sec};
}

std::vector<ZoneMatch> searchTimeZones(const std::string& query) {
    std::vector<ZoneMatch> matches;
    if (query.empty())
        return matches;
    std::string needle = query;
    std::transform(needle.begin(), needle.end(), needle.begin(), [](unsigned char c) { return char(std::tolower(c)); });
    std::ifstream table("/usr/share/zoneinfo/zone1970.tab");
    if (!table)
        table.open("/usr/share/zoneinfo/zone.tab");
    std::string line;
    while (std::getline(table, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        const auto first = line.find('\t');
        if (first == std::string::npos)
            continue;
        const auto second = line.find('\t', first + 1);
        if (second == std::string::npos)
            continue;
        const auto third = line.find('\t', second + 1);
        if (third == std::string::npos)
            continue;
        const auto fourth = line.find('\t', third + 1);
        const std::string zone = line.substr(second + 1, third - second - 1);
        const std::string comment = fourth == std::string::npos ? std::string() : line.substr(fourth + 1);
        std::string city = zone.substr(zone.find_last_of('/') == std::string::npos ? 0 : zone.find_last_of('/') + 1);
        std::replace(city.begin(), city.end(), '_', ' ');
        std::string haystack = city + " " + zone + " " + comment;
        std::transform(haystack.begin(), haystack.end(), haystack.begin(),
                       [](unsigned char c) { return char(std::tolower(c)); });
        if (haystack.find(needle) != std::string::npos)
            matches.push_back({city, zone});
        if (matches.size() >= 12)
            break;
    }
    const std::string local = std::getenv("TZ") ? std::getenv("TZ") : "";
    if (local.find(query) != std::string::npos && std::filesystem::exists("/usr/share/zoneinfo/" + local))
        matches.insert(matches.begin(), {"Local time", local});
    return matches;
}

int64_t nextAlarmOccurrenceUtc(int hour, int minute, uint8_t weekdays, std::chrono::system_clock::time_point after,
                               const std::string& zone) {
    const char* old = std::getenv("TZ");
    const std::string saved = old ? old : "";
    if (!zone.empty()) {
        setenv("TZ", zone.c_str(), 1);
        tzset();
    }
    const std::time_t start = std::chrono::system_clock::to_time_t(after);
    std::tm day{};
    localtime_r(&start, &day);
    day.tm_hour = 0;
    day.tm_min = 0;
    day.tm_sec = 0;
    std::time_t candidate = 0;
    for (int offset = 0; offset < 9; ++offset) {
        std::tm local = day;
        local.tm_mday += offset;
        local.tm_isdst = -1;
        local.tm_hour = hour;
        local.tm_min = minute;
        local.tm_sec = 0;
        candidate = std::mktime(&local);
        const int weekdayBit = (local.tm_wday + 6) % 7;
        if ((weekdays & (1u << weekdayBit)) && candidate > start)
            break;
        candidate = 0;
    }
    if (!zone.empty()) {
        if (old)
            setenv("TZ", saved.c_str(), 1);
        else
            unsetenv("TZ");
        tzset();
    }
    return static_cast<int64_t>(candidate);
}
} // namespace Clock
