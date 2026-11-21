// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <atomic>
#include <memory>
#include <vector>

#include "base/dbus.h"
#include "lingmo/timedate/time_date.h"
#include "input_test_support.h"
#include "pages/datetime/time.h"

using namespace Lingmo;
using namespace Lingmo::base;
using settings::testing::ServiceThread;
using settings::testing::waitFor;

namespace {

// timedated and the status bar on a private session bus: timedated refuses
// to turn network time off, as a cancelled password does.
struct TimeSettingsFixture : ::testing::Test
{
    void SetUp() override
    {
        dbus::Connection* bus = dbus::Connection::session();
        const dbus::Proxy daemon(bus, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus");
        if (!bus || daemon.call("NameHasOwner", std::string("org.lingmo.Statusbar")).value<bool>().value_or(true))
            GTEST_SKIP() << "Needs a private session bus: run the tests in dbus-run-session";

        timedated = std::make_unique<ServiceThread>("org.freedesktop.timedate1", [this](dbus::Connection& bus) {
            auto object = std::make_shared<dbus::Object>(&bus, "/org/freedesktop/timedate1", "org.freedesktop.timedate1");
            object->addProperty("NTP", [this] { return ntp.load(); });
            object->addProperty("CanNTP", [] { return true; });
            object->addProperty("Timezone", [] { return std::string("Asia/Singapore"); });
            object->addMethod("SetNTP", "bb", "", [this, object = object.get()](dbus::MethodCall& call) {
                bool enabled = false, interactive = false;
                call.read(enabled, interactive);
                if (!enabled) {
                    call.replyError("org.freedesktop.DBus.Error.AccessDenied", "Cancelled");
                    return;
                }
                ntp = enabled;
                object->emitPropertiesChanged({"NTP"});
                call.reply();
            });
            object->publish();
            return std::shared_ptr<void>(object);
        });
        statusBar = std::make_unique<ServiceThread>("org.lingmo.Statusbar", [this](dbus::Connection& bus) {
            auto object = std::make_shared<dbus::Object>(&bus, "/Statusbar", "org.lingmo.Statusbar");
            object->addMethod("setTwentyFourTime", [this](bool enabled) { twentyFour = enabled ? 1 : 0; });
            object->publish();
            return std::shared_ptr<void>(object);
        });
        ASSERT_TRUE(timedated->start());
        ASSERT_TRUE(statusBar->start());
    }

    void TearDown() override
    {
        // The next test checks the bus is private by the status bar's name.
        if (statusBar) {
            statusBar->stop();
            timedated->stop();
            settings::testing::waitForNameGone("org.lingmo.Statusbar");
            settings::testing::waitForNameGone("org.freedesktop.timedate1");
        }
    }

    std::unique_ptr<ServiceThread> timedated;
    std::unique_ptr<ServiceThread> statusBar;
    std::atomic<bool> ntp = true;
    std::atomic<int> twentyFour = -1;
};

} // namespace

TEST_F(TimeSettingsFixture, ARefusedChangeOfNetworkTimeGoesBack)
{
    timedate::TimeDate timeDate(dbus::Connection::session());
    settings::TimeSettings time(timeDate);
    ASSERT_TRUE(time.canNtp());
    ASSERT_TRUE(time.useNtp());
    std::vector<bool> shown;
    Subscription subscription = time.useNtpChanged.connect([&] { shown.push_back(time.useNtp()); });
    time.setUseNtp(false);
    // At once, then back once timedated refused.
    EXPECT_FALSE(time.useNtp());
    ASSERT_TRUE(waitFor([&] { return time.useNtp(); }));
    EXPECT_EQ(shown, (std::vector<bool>{false, true}));
}

TEST_F(TimeSettingsFixture, TheTwentyFourHourClockGoesToTheStatusBar)
{
    timedate::TimeDate timeDate(dbus::Connection::session());
    settings::TimeSettings time(timeDate);
    const bool was = time.twentyFour();
    time.setTwentyFour(!was);
    EXPECT_EQ(time.twentyFour(), !was);
    ASSERT_TRUE(waitFor([&] { return twentyFour != -1; }));
    EXPECT_EQ(twentyFour.load(), was ? 0 : 1);
}
