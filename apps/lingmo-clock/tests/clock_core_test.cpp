// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "clockcore.h"
#include "clockstore.h"
#include <ctime>
#include <filesystem>
#include <gtest/gtest.h>
#include <sqlite3.h>
#include <unistd.h>

TEST(ClockCore, StopwatchUsesMonotonicAndKeepsLaps) {
    Clock::Stopwatch watch;
    EXPECT_EQ(watch.elapsed().count(), 0);
    watch.start();
    auto lap = watch.lap();
    EXPECT_GE(lap.count(), 0);
    EXPECT_EQ(watch.laps().size(), 1u);
    watch.pause();
    const auto paused = watch.elapsed();
    watch.start();
    watch.pause();
    EXPECT_GE(watch.elapsed(), paused);
    watch.reset();
    EXPECT_FALSE(watch.running());
    EXPECT_TRUE(watch.laps().empty());
}

TEST(ClockCore, DurationFormattingIsStable) {
    EXPECT_EQ(Clock::formatDuration(std::chrono::seconds(3661) + std::chrono::milliseconds(230)), "01:01:01.23");
    EXPECT_EQ(Clock::formatDuration(std::chrono::seconds(7), false), "00:00:07");
}

TEST(ClockStore, PersistsAndDeletesClocksAndAlarms) {
    const auto path =
        (std::filesystem::temp_directory_path() / ("lingmo-clock-" + std::to_string(getpid()) + ".db")).string();
    {
        Clock::Store store(path);
        store.addWorldClock({"tokyo", "Tokyo", "Asia/Tokyo"});
        EXPECT_EQ(store.worldClocks().size(), 1u);
        Clock::Alarm alarm;
        alarm.name = "Wake";
        alarm.hour = 6;
        alarm.weekdays = 0x7f;
        auto id = store.saveAlarm(alarm);
        ASSERT_GT(id, 0);
        ASSERT_EQ(store.alarms().size(), 1u);
        store.setAlarmEnabled(id, false);
        EXPECT_FALSE(store.alarms()[0].enabled);
        store.removeAlarm(id);
        EXPECT_TRUE(store.alarms().empty());
        store.removeWorldClock("tokyo");
        EXPECT_TRUE(store.worldClocks().empty());
    }
    std::filesystem::remove(path);
    std::filesystem::remove(path + "-wal");
    std::filesystem::remove(path + "-shm");
}

TEST(ClockCore, AlarmOccurrenceIsFutureLocalCivilTime) {
    const auto after = std::chrono::system_clock::now();
    const auto result = Clock::nextAlarmOccurrenceUtc(8, 30, 0x7f, after, "Asia/Tokyo");
    EXPECT_GT(result, std::chrono::system_clock::to_time_t(after));
}

TEST(ClockCore, TimeZoneSearchMatchesCitiesAndIanaIds) {
    const auto tokyo = Clock::searchTimeZones("Tokyo");
    ASSERT_FALSE(tokyo.empty());
    EXPECT_EQ(tokyo.front().zone, "Asia/Tokyo");
    const auto shanghai = Clock::searchTimeZones("Asia/Shanghai");
    ASSERT_FALSE(shanghai.empty());
    EXPECT_EQ(shanghai.front().zone, "Asia/Shanghai");
}

TEST(ClockStore, PersistsOneShotAlarmFlag) {
    const auto path =
        (std::filesystem::temp_directory_path() / ("lingmo-clock-one-shot-" + std::to_string(getpid()) + ".db"))
            .string();
    Clock::Alarm alarm;
    alarm.name = "Reminder";
    alarm.hour = 8;
    alarm.minute = 15;
    alarm.recurring = false;
    alarm.weekdays = 0x7f;
    alarm.nextOccurrenceUtc = 1900000000;
    {
        Clock::Store store(path);
        store.saveAlarm(alarm);
        ASSERT_EQ(store.alarms().size(), 1u);
        EXPECT_FALSE(store.alarms()[0].recurring);
    }
    {
        Clock::Store reopened(path);
        ASSERT_EQ(reopened.alarms().size(), 1u);
        EXPECT_FALSE(reopened.alarms()[0].recurring);
    }
    std::filesystem::remove(path);
    std::filesystem::remove(path + "-wal");
    std::filesystem::remove(path + "-shm");
}

TEST(ClockStore, AddsRecurringColumnToExistingDatabase) {
    const auto path =
        (std::filesystem::temp_directory_path() / ("lingmo-clock-migration-" + std::to_string(getpid()) + ".db"))
            .string();
    sqlite3* db = nullptr;
    ASSERT_EQ(sqlite3_open(path.c_str(), &db), SQLITE_OK);
    ASSERT_EQ(sqlite3_exec(db,
                           "CREATE TABLE alarms(id INTEGER PRIMARY KEY,name TEXT NOT NULL,hour INTEGER NOT NULL,minute "
                           "INTEGER NOT NULL,weekdays INTEGER NOT NULL,enabled INTEGER NOT NULL,sound TEXT NOT NULL "
                           "DEFAULT '',next_utc INTEGER NOT NULL DEFAULT 0); INSERT INTO "
                           "alarms(name,hour,minute,weekdays,enabled,sound,next_utc) VALUES('Old',7,0,127,1,'',0);",
                           nullptr, nullptr, nullptr),
              SQLITE_OK);
    sqlite3_close(db);
    {
        Clock::Store migrated(path);
        ASSERT_EQ(migrated.alarms().size(), 1u);
        EXPECT_TRUE(migrated.alarms()[0].recurring);
    }
    std::filesystem::remove(path);
    std::filesystem::remove(path + "-wal");
    std::filesystem::remove(path + "-shm");
}

TEST(ClockCore, NormalizesAlarmAcrossSpringDstGap) {
    std::tm local{};
    local.tm_year = 126;
    local.tm_mon = 2;
    local.tm_mday = 8;
    local.tm_hour = 6;
    local.tm_min = 59;
    const auto utc = std::chrono::system_clock::from_time_t(timegm(&local));
    const auto next = Clock::nextAlarmOccurrenceUtc(2, 30, 0x7f, utc, "America/New_York");
    EXPECT_EQ(next, std::chrono::system_clock::to_time_t(utc) + 31 * 60);
}
