// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <cmath>
#include <cstdlib>
#include <memory>
#include <tuple>

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/file.h"
#include "base/settings.h"
#include "lingmo/display/display_types.h"
#include "nightlight.h"
#include "scheduleutils.h"
#include "support/wait.h"

using namespace Lingmo::base;
using namespace Lingmo::display;
using namespace ScheduleUtils;
using namespace std::chrono;
using namespace std::chrono_literals;

static const char displayService[] = "org.lingmo.DisplayConfig.NightLightTest";

static Date october(int day)
{
    return year(2026) / October / day;
}

static DateTime at(int day, int hour, int minute = 0)
{
    return ScheduleUtils::at(october(day), timeOfDay(hour, minute), zone("UTC"));
}

static TimesForDate fixed(Milliseconds start, Milliseconds end)
{
    return [start, end](Date) {
        return Times {start, end};
    };
}

static int64_t minutesBetween(const DateTime &a, Milliseconds b)
{
    return std::abs(duration_cast<seconds>(a.timeOfDay() - b).count()) / 60;
}

static int minutesOfDay(const DateTime &turn)
{
    return int(floor<minutes>(turn.timeOfDay()).count());
}

namespace {

struct SetNightLightCall {
    double strength;
    uint32_t durationMs;
};

// Answers the way the compositor's DisplayConfigService does, on a connection
// of its own.
class FakeDisplayConfig
{
public:
    explicit FakeDisplayConfig(bool tintable)
        : m_bus(dbus::Connection::open(dbus::Connection::Bus::Session))
        , m_object(m_bus.get(), "/org/lingmo/DisplayConfig1", "org.lingmo.DisplayConfig1")
    {
        m_object.addMethod("GetState", [tintable] {
            PhysicalDisplay display;
            display.sessionId = "internal";
            display.capabilities = tintable ? ColorTransformCapability : 0;
            return std::make_tuple(std::string("test-instance"), std::string("seat0"), uint64_t(1), uint64_t(1),
                                   std::vector<PhysicalDisplay> {display}, std::vector<LogicalDisplay>(),
                                   std::vector<Placement>(), std::string(), uint64_t(0));
        });
        m_object.addMethod("SetNightLight", [this](double strength, uint32_t durationMs) {
            calls.push_back({strength, durationMs});
        });
        m_object.addSignal("StateChanged", "tt");
        EXPECT_TRUE(m_object.publish());
        EXPECT_TRUE(m_bus->requestName(displayService));
    }

    std::vector<SetNightLightCall> calls;

private:
    std::unique_ptr<dbus::Connection> m_bus;
    dbus::Object m_object;
};

class NightLightTest : public testing::Test
{
protected:
    EventLoop loop;
    file::TemporaryDirectory directory {"/tmp", "night-light-test"};
    std::unique_ptr<FakeDisplayConfig> display;
    // The file as each service read it, kept while any of them lives.
    std::vector<std::unique_ptr<Settings>> files;
    DateTime now = at(2, 12);

    void SetUp() override
    {
        ASSERT_TRUE(directory.isValid());
        startCompositor();
    }

    std::string path() const { return directory.path() + "/services.conf"; }

    void startCompositor(bool tintable = true) { display = std::make_unique<FakeDisplayConfig>(tintable); }

    void stopCompositor() { display.reset(); }

    Settings &settings()
    {
        files.push_back(std::make_unique<Settings>(path()));
        return *files.back();
    }

    std::unique_ptr<NightLight> start()
    {
        return std::make_unique<NightLight>(
                settings(),
                [this] {
                    return now;
                },
                displayService);
    }

    std::unique_ptr<ScheduledFeature> darkMode()
    {
        auto feature = std::make_unique<ScheduledFeature>(
                ScheduledFeature::Keys {"Appearance", "DarkMode", "DarkModeSchedule", "DarkModeHoldUntil", {}, {}},
                settings(), [this] {
                    return now;
                });
        feature->start();
        return feature;
    }

    // What the file holds, as another process reads it.
    std::optional<std::string> saved(const std::string &key, const std::string &group = "NightLight")
    {
        return Settings(path()).rawValue(group + '/' + key);
    }

    void write(const std::string &key, const std::string &value)
    {
        Settings file(path());
        file.setValue(key, value);
    }

    bool callsReach(size_t count)
    {
        return testsupport::waitFor(loop, [&] { return display && display->calls.size() == count; });
    }
};

} // namespace

// --- Where a day stands on a schedule. ---

TEST(SchedulePositionTest, AnOvernightScheduleIsOnAcrossMidnight)
{
    const auto times = fixed(timeOfDay(18, 0), timeOfDay(6, 0));
    auto position = ScheduleUtils::position(at(2, 12), times);
    EXPECT_FALSE(position.enabled);
    EXPECT_EQ(position.next, at(2, 18));
    position = ScheduleUtils::position(at(2, 18), times);
    EXPECT_TRUE(position.enabled);
    EXPECT_EQ(position.next, at(3, 6));
    position = ScheduleUtils::position(at(3, 3), times);
    EXPECT_TRUE(position.enabled);
    EXPECT_EQ(position.next, at(3, 6));
    position = ScheduleUtils::position(at(3, 6), times);
    EXPECT_FALSE(position.enabled);
    EXPECT_EQ(position.next, at(3, 18));
}

TEST(SchedulePositionTest, ADaytimeScheduleIsOnBetweenItsTimes)
{
    const auto times = fixed(timeOfDay(9, 0), timeOfDay(17, 0));
    auto position = ScheduleUtils::position(at(2, 8), times);
    EXPECT_FALSE(position.enabled);
    EXPECT_EQ(position.next, at(2, 9));
    position = ScheduleUtils::position(at(2, 12), times);
    EXPECT_TRUE(position.enabled);
    EXPECT_EQ(position.next, at(2, 17));
    position = ScheduleUtils::position(at(2, 20), times);
    EXPECT_FALSE(position.enabled);
    EXPECT_EQ(position.next, at(3, 9));
}

TEST(SchedulePositionTest, EqualTimesNeverTurnItOn)
{
    const auto position = ScheduleUtils::position(at(2, 12), fixed(timeOfDay(7, 0), timeOfDay(7, 0)));
    EXPECT_FALSE(position.enabled);
    EXPECT_FALSE(position.next.isValid());
}

// Tomorrow's sunrise is not today's: the turn comes from the day it falls on.
TEST(SchedulePositionTest, TheNextTurnUsesItsOwnDaysTimes)
{
    const TimesForDate times = [](Date date) {
        return Times {timeOfDay(18, 0), timeOfDay(6, int(unsigned(date.day())))};
    };
    const auto position = ScheduleUtils::position(at(2, 22), times);
    EXPECT_TRUE(position.enabled);
    EXPECT_EQ(position.next, at(3, 6, 3));
}

// Europe/Berlin repeats 02:00 to 03:00 on 2026-10-25: each 02:30 waits for its own 02:45.
TEST(SchedulePositionTest, ARepeatedHourTurnsAtItsOwnOccurrence)
{
    const Zone berlin = zone("Europe/Berlin");
    ASSERT_TRUE(berlin);
    const Date date = october(25);
    const auto times = fixed(timeOfDay(18, 0), timeOfDay(2, 45));
    const DateTime first = ScheduleUtils::at(date, timeOfDay(2, 30), berlin, false);
    const DateTime second = ScheduleUtils::at(date, timeOfDay(2, 30), berlin, true);
    EXPECT_EQ(first.msecsTo(second), 3600 * 1000);

    auto position = ScheduleUtils::position(first, times);
    EXPECT_TRUE(position.enabled);
    EXPECT_EQ(position.next, ScheduleUtils::at(date, timeOfDay(2, 45), berlin, false));

    position = ScheduleUtils::position(second, times);
    EXPECT_TRUE(position.enabled);
    EXPECT_GT(position.next, second);
    EXPECT_EQ(position.next, ScheduleUtils::at(date, timeOfDay(2, 45), berlin, true));
}

// Against NOAA's solar calculator and the Julian-date sunrise equation,
// which agree to a few minutes.
TEST(SunTimesTest, SunTimesMatchTheAlmanac)
{
    auto sun = sunTimes(year(2026) / June / 21, 31.2333, 121.4667, zone("Asia/Shanghai"));
    ASSERT_TRUE(sun);
    EXPECT_LE(minutesBetween(sun->sunrise, timeOfDay(4, 50)), 3);
    EXPECT_LE(minutesBetween(sun->sunset, timeOfDay(19, 0)), 3);
    EXPECT_EQ(sun->sunrise.date(), year(2026) / June / 21);

    sun = sunTimes(year(2026) / December / 21, 51.5083, -0.1253, zone("Europe/London"));
    ASSERT_TRUE(sun);
    EXPECT_LE(minutesBetween(sun->sunrise, timeOfDay(8, 3)), 3);
    EXPECT_LE(minutesBetween(sun->sunset, timeOfDay(15, 53)), 3);

    sun = sunTimes(year(2026) / July / 4, 40.7142, -74.0064, zone("America/New_York"));
    ASSERT_TRUE(sun);
    EXPECT_LE(minutesBetween(sun->sunrise, timeOfDay(5, 29)), 3);
    EXPECT_LE(minutesBetween(sun->sunset, timeOfDay(20, 30)), 3);
    EXPECT_EQ(sun->sunset.date(), year(2026) / July / 4);
}

TEST(SunTimesTest, ThePolarDayAndNightHaveNoTurns)
{
    EXPECT_FALSE(sunTimes(year(2026) / June / 21, 69.6489, 18.9551, zone("Europe/Oslo")));
    EXPECT_FALSE(sunTimes(year(2026) / December / 21, 69.6489, 18.9551, zone("Europe/Oslo")));
}

TEST(SunTimesTest, ZoneTablesGiveTheZonesCity)
{
    file::TemporaryFile table("/tmp", "zone-table");
    ASSERT_TRUE(table.isValid());
    ASSERT_TRUE(table.write("# comment\n"
                            "CN\t+3114+12128\tAsia/Shanghai\tBeijing Time\n"
                            "US\t+404251-0740023\tAmerica/New_York\tEastern (most areas)\n"
                            "XX\tnonsense\tBroken/Zone\n"));

    auto coordinates = zoneCoordinates("Asia/Shanghai", {table.path()});
    ASSERT_TRUE(coordinates);
    EXPECT_LT(std::abs(coordinates->latitude - (31 + 14 / 60.0)), 1e-9);
    EXPECT_LT(std::abs(coordinates->longitude - (121 + 28 / 60.0)), 1e-9);
    coordinates = zoneCoordinates("America/New_York", {table.path()});
    ASSERT_TRUE(coordinates);
    EXPECT_LT(std::abs(coordinates->latitude - (40 + 42 / 60.0 + 51 / 3600.0)), 1e-9);
    EXPECT_LT(std::abs(coordinates->longitude + (74 + 0 / 60.0 + 23 / 3600.0)), 1e-9);
    EXPECT_FALSE(zoneCoordinates("Broken/Zone", {table.path()}));
    EXPECT_FALSE(zoneCoordinates("UTC", {table.path()}));
}

// --- The service. ---

TEST_F(NightLightTest, AvailabilityFollowsTheCompositorsDisplays)
{
    auto nightLight = start();
    EXPECT_TRUE(testsupport::waitFor(loop, [&] { return nightLight->available(); }));

    stopCompositor();
    EXPECT_TRUE(testsupport::waitFor(loop, [&] { return !nightLight->available(); }));
    startCompositor(false);
    testsupport::wait(loop, 100ms);
    EXPECT_FALSE(nightLight->available());
}

TEST_F(NightLightTest, TheSavedStateReachesTheCompositorOnceItIsLoaded)
{
    write("NightLight/Enabled", "true");
    write("NightLight/Temperature", "0.3");
    auto nightLight = start();
    ASSERT_TRUE(callsReach(1));
    EXPECT_EQ(display->calls.front().strength, 0.3);
    EXPECT_EQ(display->calls.front().durationMs, 1000u);

    // A compositor that starts again is told again.
    stopCompositor();
    startCompositor();
    ASSERT_TRUE(callsReach(1));
    EXPECT_EQ(display->calls.front().strength, 0.3);
}

TEST_F(NightLightTest, AManualToggleTakesASecondAndIsSaved)
{
    auto nightLight = start();
    ASSERT_TRUE(testsupport::waitFor(loop, [&] { return nightLight->available(); }));
    int changed = 0;
    auto subscription = nightLight->enabledChanged.connect([&changed](bool) { ++changed; });

    nightLight->setEnabled(true);
    EXPECT_EQ(changed, 1);
    ASSERT_TRUE(callsReach(2));
    EXPECT_EQ(display->calls.back().strength, 0.5);
    EXPECT_EQ(display->calls.back().durationMs, 1000u);
    EXPECT_EQ(saved("Enabled"), "true");

    nightLight->setEnabled(false);
    ASSERT_TRUE(callsReach(3));
    EXPECT_EQ(display->calls.back().strength, 0.0);
}

TEST_F(NightLightTest, ATemperatureChangedWhileOffIsOnlySaved)
{
    auto nightLight = start();
    ASSERT_TRUE(testsupport::waitFor(loop, [&] { return nightLight->available(); }));
    ASSERT_TRUE(callsReach(1));

    nightLight->setTemperature(0.8);
    testsupport::wait(loop, 100ms);
    EXPECT_EQ(display->calls.size(), 1u);
    EXPECT_EQ(saved("Temperature"), "0.8");

    nightLight->setEnabled(true);
    ASSERT_TRUE(callsReach(2));
    EXPECT_EQ(display->calls.back().strength, 0.8);
    nightLight->setTemperature(2);
    EXPECT_EQ(nightLight->temperature(), 1.0);
}

TEST_F(NightLightTest, APreviewShowsTheTemperatureWhileOff)
{
    auto nightLight = start();
    ASSERT_TRUE(testsupport::waitFor(loop, [&] { return nightLight->available(); }));
    ASSERT_TRUE(callsReach(1));

    nightLight->setPreview(true);
    ASSERT_TRUE(callsReach(2));
    EXPECT_EQ(display->calls.back().strength, 0.5);
    EXPECT_EQ(display->calls.back().durationMs, 1000u);

    nightLight->setTemperature(0.8);
    ASSERT_TRUE(callsReach(3));
    EXPECT_EQ(display->calls.back().strength, 0.8);
    EXPECT_EQ(display->calls.back().durationMs, 100u);

    nightLight->setPreview(false);
    ASSERT_TRUE(callsReach(4));
    EXPECT_EQ(display->calls.back().strength, 0.0);
    EXPECT_EQ(display->calls.back().durationMs, 1000u);
    EXPECT_FALSE(nightLight->enabled());
    EXPECT_EQ(saved("Temperature"), "0.8");
    EXPECT_EQ(saved("Enabled").value_or("false"), "false");
}

TEST_F(NightLightTest, APreviewWhileOnOnlyQuickensTheSlider)
{
    auto nightLight = start();
    ASSERT_TRUE(testsupport::waitFor(loop, [&] { return nightLight->available(); }));
    nightLight->setEnabled(true);
    ASSERT_TRUE(callsReach(2));

    nightLight->setPreview(true);
    nightLight->setTemperature(0.8);
    ASSERT_TRUE(callsReach(3));
    EXPECT_EQ(display->calls.back().durationMs, 100u);
    nightLight->setPreview(false);
    testsupport::wait(loop, 100ms);
    EXPECT_EQ(display->calls.size(), 3u);
    EXPECT_EQ(display->calls.back().strength, 0.8);
}

TEST_F(NightLightTest, APreviewEndsWhenItsCallerLeaves)
{
    auto nightLight = start();
    ASSERT_TRUE(testsupport::waitFor(loop, [&] { return nightLight->available(); }));
    ASSERT_TRUE(callsReach(1));
    // The service's own route: through its D-Bus object, which hears the caller.
    dbus::Connection *service = dbus::Connection::session();
    ASSERT_TRUE(nightLight->publish(service));

    auto client = dbus::Connection::open(dbus::Connection::Bus::Session);
    dbus::Message call = client->newMethodCall(service->uniqueName(), "/org/lingmo/Services/NightLight",
                                                 "org.lingmo.Services.NightLight", "setPreview");
    call.append(true);
    client->send(call);
    ASSERT_TRUE(testsupport::waitFor(loop, [&] { return nightLight->previewing(); }));
    ASSERT_TRUE(callsReach(2));
    EXPECT_EQ(display->calls.back().strength, 0.5);

    client.reset();
    EXPECT_TRUE(testsupport::waitFor(loop, [&] { return !nightLight->previewing(); }));
    ASSERT_TRUE(callsReach(3));
    EXPECT_EQ(display->calls.back().strength, 0.0);
}

TEST_F(NightLightTest, ACustomScheduleTurnsOnWithinItsHours)
{
    now = at(2, 20);
    auto nightLight = start();
    ASSERT_TRUE(testsupport::waitFor(loop, [&] { return nightLight->available(); }));

    nightLight->setSchedule(NightLight::Custom);
    EXPECT_TRUE(nightLight->enabled());
    EXPECT_EQ(nightLight->nextCheckpoint(), at(3, 6));
    ASSERT_TRUE(callsReach(2));
    EXPECT_EQ(display->calls.back().strength, 0.5);
    EXPECT_EQ(display->calls.back().durationMs, 1000u);

    // Moving the end before now turns it off at once.
    nightLight->setCustomEnd(19 * 60);
    EXPECT_FALSE(nightLight->enabled());
    EXPECT_EQ(nightLight->nextCheckpoint(), at(3, 18));
}

TEST_F(NightLightTest, AScheduledTurnTakesAMinute)
{
    now = at(2, 17);
    auto nightLight = start();
    nightLight->setSchedule(NightLight::Custom);
    ASSERT_TRUE(testsupport::waitFor(loop, [&] { return nightLight->available(); }));
    EXPECT_FALSE(nightLight->enabled());
    EXPECT_EQ(nightLight->nextCheckpoint(), at(2, 18));

    now = at(2, 18);
    nightLight->checkpointForTesting();
    EXPECT_TRUE(nightLight->enabled());
    ASSERT_TRUE(callsReach(2));
    EXPECT_EQ(display->calls.back().durationMs, 60000u);
    EXPECT_EQ(nightLight->nextCheckpoint(), at(3, 6));
}

// ScheduledFeature: the user's toggle holds until the schedule's next turn.
TEST_F(NightLightTest, AManualToggleHoldsUntilTheNextTurn)
{
    now = at(2, 20);
    auto nightLight = start();
    nightLight->setSchedule(NightLight::Custom);
    EXPECT_TRUE(nightLight->enabled());

    now = at(2, 21);
    nightLight->setEnabled(false);
    now = at(2, 23);
    nightLight->checkpointForTesting();
    EXPECT_FALSE(nightLight->enabled());

    now = at(3, 6);
    nightLight->checkpointForTesting();
    EXPECT_FALSE(nightLight->enabled());
    EXPECT_EQ(nightLight->nextCheckpoint(), at(3, 18));
    now = at(3, 18);
    nightLight->checkpointForTesting();
    EXPECT_TRUE(nightLight->enabled());
}

TEST_F(NightLightTest, AManualToggleHoldsAcrossSessionsUntilItsTurn)
{
    now = at(2, 20);
    auto nightLight = start();
    nightLight->setSchedule(NightLight::Custom);
    now = at(2, 21);
    nightLight->setEnabled(false);
    nightLight.reset();

    now = at(2, 23);
    nightLight = start();
    EXPECT_FALSE(nightLight->enabled());
    EXPECT_EQ(nightLight->nextCheckpoint(), at(3, 6));
    ASSERT_TRUE(callsReach(1));
    EXPECT_EQ(display->calls.back().strength, 0.0);
    nightLight.reset();

    // Once its turn has passed, the schedule decides again.
    now = at(3, 19);
    nightLight = start();
    EXPECT_TRUE(nightLight->enabled());
    EXPECT_FALSE(saved("HoldUntil"));
}

TEST_F(NightLightTest, AResumeKeepsAToggleUnlessItsTurnPassedDuringTheSleep)
{
    now = at(2, 12);
    auto nightLight = start();
    nightLight->setSchedule(NightLight::Custom);
    nightLight->setEnabled(true);

    now = at(2, 13);
    nightLight->resumeForTesting();
    EXPECT_TRUE(nightLight->enabled());
    EXPECT_EQ(nightLight->nextCheckpoint(), at(2, 18));

    // Asleep through the evening and the night.
    now = at(3, 7);
    nightLight->resumeForTesting();
    EXPECT_FALSE(nightLight->enabled());
    EXPECT_EQ(nightLight->nextCheckpoint(), at(3, 18));
}

TEST_F(NightLightTest, TheSunTimesAreTheScheduleTurnsToCome)
{
    now = at(2, 12);
    auto nightLight = start();
    nightLight->setSchedule(NightLight::SunsetToSunrise);
    EXPECT_FALSE(nightLight->enabled());
    EXPECT_EQ(nightLight->sunset(), minutesOfDay(nightLight->nextCheckpoint()));

    now = nightLight->nextCheckpoint();
    nightLight->checkpointForTesting();
    EXPECT_TRUE(nightLight->enabled());
    EXPECT_EQ(nightLight->sunrise(), minutesOfDay(nightLight->nextCheckpoint()));
    // Today's sunset has passed; tomorrow's is a minute or two away from it.
    EXPECT_LE(std::abs(nightLight->sunset() - minutesOfDay(now)), 3);
}

TEST_F(NightLightTest, ASessionStartedDuringAHoldShowsTheSunTimes)
{
    now = at(2, 12);
    auto nightLight = start();
    nightLight->setSchedule(NightLight::SunsetToSunrise);
    nightLight->setEnabled(true);
    const int sunset = nightLight->sunset();
    const int sunrise = nightLight->sunrise();
    nightLight.reset();

    nightLight = start();
    EXPECT_TRUE(nightLight->enabled());
    EXPECT_EQ(nightLight->sunset(), sunset);
    EXPECT_EQ(nightLight->sunrise(), sunrise);
    EXPECT_EQ(nightLight->sunset(), minutesOfDay(nightLight->nextCheckpoint()));
}

TEST_F(NightLightTest, TheSessionStartsWhereTheScheduleIs)
{
    write("NightLight/Schedule", std::to_string(int(NightLight::Custom)));
    write("NightLight/Enabled", "false");
    now = at(2, 22);
    auto nightLight = start();
    EXPECT_TRUE(nightLight->enabled());
    ASSERT_TRUE(callsReach(1));
    EXPECT_EQ(display->calls.front().strength, 0.5);
}

// --- A feature without custom times, the way dark mode keeps one. ---

TEST_F(NightLightTest, AFeatureWithoutCustomTimesFollowsOnlyTheSun)
{
    write("Appearance/DarkModeSchedule", std::to_string(int(ScheduledFeature::Custom)));
    auto feature = darkMode();
    EXPECT_EQ(feature->schedule(), int(ScheduledFeature::SunsetToSunrise));

    feature->setSchedule(ScheduledFeature::Never);
    feature->setSchedule(ScheduledFeature::Custom);
    EXPECT_EQ(feature->schedule(), int(ScheduledFeature::Never));
    EXPECT_FALSE(feature->nextCheckpoint().isValid());

    feature->setSchedule(ScheduledFeature::SunsetToSunrise);
    EXPECT_FALSE(feature->enabled());
    now = feature->nextCheckpoint();
    feature->checkpointForTesting();
    EXPECT_TRUE(feature->enabled());
    EXPECT_EQ(saved("DarkMode", "Appearance"), "true");
}

TEST_F(NightLightTest, AFeatureKeepsItsHoldUnderItsOwnKeys)
{
    auto feature = darkMode();
    feature->setSchedule(ScheduledFeature::SunsetToSunrise);
    feature->setEnabled(true);
    EXPECT_EQ(saved("DarkModeHoldUntil", "Appearance"), std::to_string(feature->nextCheckpoint().toMSecsSinceEpoch()));
    EXPECT_FALSE(saved("HoldUntil"));

    auto restarted = darkMode();
    EXPECT_TRUE(restarted->enabled());
}
