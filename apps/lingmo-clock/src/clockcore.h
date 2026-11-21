// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace Clock {

struct WorldClock {
    std::string id;
    std::string city;
    std::string zone;
};

struct ZoneMatch {
    std::string city;
    std::string zone;
};

struct Alarm {
    int64_t id = 0;
    std::string name;
    int hour = 7;
    int minute = 0;
    uint8_t weekdays = 0x7f;
    bool recurring = true;
    bool enabled = true;
    std::string sound;
    int64_t nextOccurrenceUtc = 0;
};

// Elapsed tasks use steady_clock; wall clock is only used for civil-time events.
class Stopwatch {
  public:
    void start();
    void pause();
    void reset();
    bool running() const {
        return m_running;
    }
    std::chrono::nanoseconds elapsed() const;
    std::chrono::nanoseconds lap();
    const std::vector<std::chrono::nanoseconds>& laps() const {
        return m_laps;
    }

  private:
    std::chrono::steady_clock::time_point m_started{};
    std::chrono::nanoseconds m_accumulated{};
    std::vector<std::chrono::nanoseconds> m_laps;
    bool m_running = false;
};

std::string formatDuration(std::chrono::nanoseconds value, bool centiseconds = true);
std::array<int, 3> localTimeParts(const std::string& zone, std::chrono::system_clock::time_point now);
std::string localTime(const std::string& zone, std::chrono::system_clock::time_point now);
std::string utcOffset(const std::string& zone, std::chrono::system_clock::time_point now);
std::string timeDifference(const std::string& zone, std::chrono::system_clock::time_point now);
std::vector<ZoneMatch> searchTimeZones(const std::string& query);
int64_t nextAlarmOccurrenceUtc(int hour, int minute, uint8_t weekdays, std::chrono::system_clock::time_point after,
                               const std::string& zone = "");

} // namespace Clock
