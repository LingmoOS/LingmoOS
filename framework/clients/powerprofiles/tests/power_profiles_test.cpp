// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <memory>
#include <thread>

#include "base/event_loop.h"
#include "lingmo/powerprofiles/power_profiles.h"

using namespace Lingmo;
using namespace Lingmo::Base;
using namespace Lingmo::Base::dbus;
using namespace Lingmo::powerprofiles;
using namespace std::chrono_literals;

namespace {

constexpr const char* kService = "org.freedesktop.UPower.PowerProfiles";

// power-profiles-daemon on the session bus, on a thread of its own.
class FakeDaemon
{
public:
    FakeDaemon()
    {
        std::promise<bool> ready;
        m_thread = std::thread([this, &ready] { run(ready); });
        m_ready = ready.get_future().get();
    }
    ~FakeDaemon()
    {
        m_handle.post([this] { m_loop->quit(); });
        m_thread.join();
    }

    bool isReady() const { return m_ready; }
    template<typename F>
    auto inService(F f)
    {
        std::promise<decltype(f())> result;
        m_handle.post([&] {
            if constexpr (std::is_void_v<decltype(f())>) {
                f();
                result.set_value();
            } else {
                result.set_value(f());
            }
        });
        return result.get_future().get();
    }

    std::vector<std::string> profiles{"power-saver", "balanced", "performance"};
    std::string activeProfile = "balanced";
    std::string performanceDegraded;
    // As polkit does for a session it does not let through.
    bool refuse = false;
    std::unique_ptr<Object> object;

private:
    void run(std::promise<bool>& ready)
    {
        EventLoop loop;
        m_loop = &loop;
        m_handle = loop.handle();
        std::unique_ptr<Connection> connection = Connection::open(Connection::Bus::Session, &loop);
        if (!connection || !connection->requestName(kService)) {
            ready.set_value(false);
            return;
        }
        object = std::make_unique<Object>(connection.get(), "/org/freedesktop/UPower/PowerProfiles", kService);
        object->addProperty("Profiles", [this] {
            std::vector<VariantMap> list;
            for (const std::string& profile : profiles)
                list.push_back({{"Profile", Variant(profile)}, {"Driver", Variant(std::string("placeholder"))}});
            return list;
        });
        object->addProperty(
                "ActiveProfile", [this] { return activeProfile; },
                [this](const std::string& profile) {
                    if (refuse)
                        return false;
                    activeProfile = profile;
                    object->emitPropertiesChanged({"ActiveProfile"});
                    return true;
                });
        object->addProperty("PerformanceDegraded", [this] { return performanceDegraded; });
        ready.set_value(object->publish());
        loop.run();
        object.reset();
    }

    std::thread m_thread;
    EventLoop* m_loop = nullptr;
    LoopHandle m_handle;
    bool m_ready = false;
};

class PowerProfilesTest : public testing::Test
{
protected:
    void SetUp() override
    {
        if (!Connection::session(&m_loop))
            GTEST_SKIP() << "No session bus; run the tests in dbus-run-session";
        m_daemon = std::make_unique<FakeDaemon>();
        ASSERT_TRUE(m_daemon->isReady());
    }

    bool waitFor(const std::function<bool()>& condition)
    {
        const auto deadline = std::chrono::steady_clock::now() + 2s;
        while (!condition()) {
            if (std::chrono::steady_clock::now() > deadline)
                return false;
            runFor(5ms);
        }
        return true;
    }

    void runFor(std::chrono::milliseconds time)
    {
        const int timer = m_loop.startTimer(time, false, [this] { m_loop.quit(); });
        m_loop.run();
        m_loop.stopTimer(timer);
    }

    Connection* bus() { return Connection::session(&m_loop); }

    EventLoop m_loop;
    std::unique_ptr<FakeDaemon> m_daemon;
};

} // namespace

TEST_F(PowerProfilesTest, ReadsTheProfiles)
{
    PowerProfiles profiles(bus());
    ASSERT_TRUE(waitFor([&] { return profiles.isReady(); }));
    EXPECT_EQ(profiles.profiles(), (std::vector<std::string>{"power-saver", "balanced", "performance"}));
    EXPECT_EQ(profiles.activeProfile(), "balanced");
    EXPECT_EQ(profiles.performanceDegraded(), "");
}

TEST_F(PowerProfilesTest, AChosenProfileShowsAtOnce)
{
    PowerProfiles profiles(bus());
    ASSERT_TRUE(waitFor([&] { return profiles.isReady(); }));
    int changes = 0;
    Subscription subscription = profiles.activeProfileChanged.connect([&] { ++changes; });
    profiles.setActiveProfile("power-saver");
    EXPECT_EQ(profiles.activeProfile(), "power-saver");
    EXPECT_EQ(changes, 1);
    ASSERT_TRUE(waitFor([&] { return m_daemon->inService([&] { return m_daemon->activeProfile; }) == "power-saver"; }));
    runFor(50ms);
    EXPECT_EQ(changes, 1);

    // Profiles the daemon does not have are ignored.
    profiles.setActiveProfile("turbo");
    EXPECT_EQ(profiles.activeProfile(), "power-saver");
}

TEST_F(PowerProfilesTest, ARefusedProfileGoesBack)
{
    m_daemon->inService([&] { m_daemon->refuse = true; });
    PowerProfiles profiles(bus());
    ASSERT_TRUE(waitFor([&] { return profiles.isReady(); }));
    profiles.setActiveProfile("performance");
    EXPECT_EQ(profiles.activeProfile(), "performance");
    ASSERT_TRUE(waitFor([&] { return profiles.activeProfile() == "balanced"; }));
}

TEST_F(PowerProfilesTest, FollowsTheDaemon)
{
    PowerProfiles profiles(bus());
    ASSERT_TRUE(waitFor([&] { return profiles.isReady(); }));
    int degradedChanges = 0;
    int listChanges = 0;
    Subscription degraded = profiles.performanceDegradedChanged.connect([&] { ++degradedChanges; });
    Subscription list = profiles.profilesChanged.connect([&] { ++listChanges; });
    m_daemon->inService([&] {
        m_daemon->performanceDegraded = "lap-detected";
        m_daemon->profiles = {"power-saver", "balanced"};
        m_daemon->object->emitPropertiesChanged({"PerformanceDegraded", "Profiles"});
    });
    ASSERT_TRUE(waitFor([&] { return profiles.performanceDegraded() == "lap-detected"; }));
    EXPECT_EQ(degradedChanges, 1);
    EXPECT_EQ(listChanges, 1);
    EXPECT_EQ(profiles.profiles(), (std::vector<std::string>{"power-saver", "balanced"}));
}
