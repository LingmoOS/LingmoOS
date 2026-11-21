// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <future>
#include <thread>

#include "base/event_loop.h"
#include "lingmo/timedate/time_date.h"

using namespace Lingmo;
using namespace Lingmo::Base;
using namespace Lingmo::Base::dbus;
using namespace std::chrono_literals;

namespace {

// org.freedesktop.timedate1 on the session bus, on a thread of its own.
class FakeTimedated
{
public:
    FakeTimedated()
    {
        std::promise<bool> ready;
        m_thread = std::thread([this, &ready] { run(ready); });
        m_ready = ready.get_future().get();
    }
    ~FakeTimedated()
    {
        m_handle.post([this] { m_loop->quit(); });
        m_thread.join();
    }

    bool isReady() const { return m_ready; }
    bool askedInteractively() const { return m_interactive; }

private:
    void run(std::promise<bool>& ready)
    {
        EventLoop loop;
        m_loop = &loop;
        m_handle = loop.handle();
        std::unique_ptr<Connection> connection = Connection::open(Connection::Bus::Session, &loop);
        if (!connection || !connection->requestName("org.freedesktop.timedate1")) {
            ready.set_value(false);
            return;
        }
        Object object(connection.get(), "/org/freedesktop/timedate1", "org.freedesktop.timedate1");
        object.addProperty("Timezone", [this] { return m_timezone; });
        object.addProperty("NTP", [this] { return m_ntp; });
        object.addProperty("CanNTP", [] { return true; });
        object.addMethod("SetNTP", "bb", "", [this, &object](MethodCall& call) {
            bool enabled = false, interactive = false;
            call.read(enabled, interactive);
            m_interactive = call.message().allowsInteractiveAuthorization();
            m_ntp = enabled;
            object.emitPropertiesChanged({"NTP"});
            call.reply();
        });
        object.addMethod("SetTimezone", "sb", "", [this, &object](MethodCall& call) {
            std::string timezone;
            bool interactive = false;
            call.read(timezone, interactive);
            m_interactive = call.message().allowsInteractiveAuthorization();
            if (timezone == "Nowhere/Land") {
                call.replyError("org.freedesktop.timedate1.InvalidTimezone", "Invalid or not installed time zone");
                return;
            }
            m_timezone = timezone;
            call.reply();
            object.emitPropertiesChanged({"Timezone"});
        });
        ready.set_value(object.publish());
        loop.run();
    }

    std::thread m_thread;
    EventLoop* m_loop = nullptr;
    LoopHandle m_handle;
    std::string m_timezone = "Asia/Singapore";
    bool m_ntp = true;
    std::atomic<bool> m_interactive = false;
    bool m_ready = false;
};

class TimeDateTest : public testing::Test
{
protected:
    void SetUp() override
    {
        if (!Connection::session(&m_loop))
            GTEST_SKIP() << "No session bus; run the tests in dbus-run-session";
        m_service = std::make_unique<FakeTimedated>();
        ASSERT_TRUE(m_service->isReady());
    }

    bool waitFor(const std::function<bool()>& condition)
    {
        const auto deadline = std::chrono::steady_clock::now() + 2s;
        while (!condition()) {
            if (std::chrono::steady_clock::now() > deadline)
                return false;
            const int timer = m_loop.startTimer(5ms, false, [this] { m_loop.quit(); });
            m_loop.run();
            m_loop.stopTimer(timer);
        }
        return true;
    }

    EventLoop m_loop;
    std::unique_ptr<FakeTimedated> m_service;
};

} // namespace

TEST_F(TimeDateTest, ReadsAtOnceAndFollowsChanges)
{
    timedate::TimeDate timeDate(Connection::session(&m_loop));
    ASSERT_TRUE(timeDate.isAvailable());
    EXPECT_EQ(timeDate.timezone(), "Asia/Singapore");
    EXPECT_TRUE(timeDate.ntp());
    EXPECT_TRUE(timeDate.canNtp());

    int changes = 0;
    Subscription subscription = timeDate.changed.connect([&] { ++changes; });
    std::string error = "unset";
    timeDate.setNtp(false, [&](const std::string& message) { error = message; });
    ASSERT_TRUE(waitFor([&] { return error != "unset" && !timeDate.ntp(); }));
    EXPECT_EQ(error, "");
    EXPECT_GE(changes, 1);
    EXPECT_TRUE(m_service->askedInteractively());
}

TEST_F(TimeDateTest, SetsTheTimezone)
{
    timedate::TimeDate timeDate(Connection::session(&m_loop));
    std::string error = "unset";
    timeDate.setTimezone("Europe/Berlin", [&](const std::string& message) { error = message; });
    ASSERT_TRUE(waitFor([&] { return error != "unset"; }));
    EXPECT_EQ(error, "");
    // Known from the reply, before PropertiesChanged.
    EXPECT_EQ(timeDate.timezone(), "Europe/Berlin");

    error = "unset";
    timeDate.setTimezone("Nowhere/Land", [&](const std::string& message) { error = message; });
    ASSERT_TRUE(waitFor([&] { return error != "unset"; }));
    EXPECT_EQ(error, "Invalid or not installed time zone");
    EXPECT_EQ(timeDate.timezone(), "Europe/Berlin");
}
