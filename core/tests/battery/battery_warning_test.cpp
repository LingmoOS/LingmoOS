// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/event_loop.h"
#include "battery/batterywarning.h"
#include "support/wait.h"

using namespace Lingmo::base;
using namespace std::chrono_literals;

static const char displayDevice[] = "/org/freedesktop/UPower/devices/DisplayDevice";
static const char deviceInterface[] = "org.freedesktop.UPower.Device";

namespace {

struct Notice {
    uint32_t replacesId;
    std::string summary;
    std::string body;
    int urgency;
};

// UPower, its display device and the notification server, on a connection of
// their own, so every call crosses the bus.
class Fakes
{
public:
    Fakes()
        : m_bus(dbus::Connection::open(dbus::Connection::Bus::Session))
    {
        m_upower = std::make_unique<dbus::Object>(m_bus.get(), "/org/freedesktop/UPower", "org.freedesktop.UPower");
        m_upower->addMethod("GetCriticalAction", [this] { return criticalAction; });
        EXPECT_TRUE(m_upower->publish());

        m_device = std::make_unique<dbus::Object>(m_bus.get(), displayDevice, deviceInterface);
        m_device->addProperty("WarningLevel", [this] { return warningLevel; });
        m_device->addProperty("Percentage", [this] { return percentage; });
        m_device->addProperty("TimeToEmpty", [this] { return timeToEmpty; });
        EXPECT_TRUE(m_device->publish());

        m_notifications = std::make_unique<dbus::Object>(m_bus.get(), "/org/freedesktop/Notifications",
                                                         "org.freedesktop.Notifications");
        m_notifications->addMethod("Notify", "susssasa{sv}i", "u", [this](dbus::MethodCall &call) {
            std::string appName;
            uint32_t replacesId = 0;
            std::string icon;
            std::string summary;
            std::string body;
            std::vector<std::string> actions;
            dbus::VariantMap hints;
            int32_t timeout = 0;
            ASSERT_TRUE(call.read(appName, replacesId, icon, summary, body, actions, hints, timeout));
            if (failures > 0) {
                --failures;
                call.replyError("org.freedesktop.DBus.Error.ServiceUnknown", "No server yet");
                return;
            }
            const auto urgency = hints.find("urgency");
            notices.push_back({replacesId, summary, body,
                               urgency == hints.end() ? -1 : int(urgency->second.value<uint8_t>())});
            call.reply(replacesId ? replacesId : uint32_t(notices.size()));
        });
        m_notifications->addMethod("CloseNotification", [this](uint32_t id) { closed.push_back(id); });
        m_notifications->addSignal("NotificationClosed", "uu");
        EXPECT_TRUE(m_notifications->publish());

        EXPECT_TRUE(m_bus->requestName("org.freedesktop.UPower"));
        EXPECT_TRUE(m_bus->requestName("org.freedesktop.Notifications"));
    }

    void change(uint32_t level, double percent, int64_t time = 0)
    {
        warningLevel = level;
        percentage = percent;
        timeToEmpty = time;
        m_bus->emitSignal(displayDevice, "org.freedesktop.DBus.Properties", "PropertiesChanged",
                          std::string(deviceInterface),
                          dbus::VariantMap {{"WarningLevel", dbus::Variant(warningLevel)},
                                            {"Percentage", dbus::Variant(percentage)},
                                            {"TimeToEmpty", dbus::Variant(timeToEmpty)}},
                          std::vector<std::string>());
    }

    void dismiss(uint32_t id)
    {
        m_notifications->emitSignal("NotificationClosed", id, uint32_t(2));
    }

    std::string criticalAction = "PowerOff";
    uint32_t warningLevel = BatteryWarning::None;
    double percentage = 80;
    int64_t timeToEmpty = 0;
    std::vector<Notice> notices;
    std::vector<uint32_t> closed;
    int failures = 0;

private:
    std::unique_ptr<dbus::Connection> m_bus;
    std::unique_ptr<dbus::Object> m_upower;
    std::unique_ptr<dbus::Object> m_device;
    std::unique_ptr<dbus::Object> m_notifications;
};

class BatteryWarningTest : public testing::Test
{
protected:
    EventLoop loop;
    Fakes fakes;

    // UPower is on the system bus, which the session bus stands in for.
    std::unique_ptr<BatteryWarning> start()
    {
        return std::make_unique<BatteryWarning>(dbus::Connection::session(), dbus::Connection::session());
    }

    // Lets a change that should be ignored reach the warning first.
    void settle() { testsupport::wait(loop, 150ms); }

    bool noticesReach(size_t count)
    {
        return testsupport::waitFor(loop, [&] { return fakes.notices.size() == count; });
    }
};

} // namespace

TEST_F(BatteryWarningTest, EscalatesInOneNotification)
{
    auto warning = start();
    settle();
    EXPECT_EQ(fakes.notices.size(), 0u);

    fakes.change(BatteryWarning::Low, 20, 65 * 60);
    ASSERT_TRUE(noticesReach(1));
    Notice notice = fakes.notices[0];
    EXPECT_EQ(notice.replacesId, 0u);
    EXPECT_EQ(notice.summary, "Low Battery");
    EXPECT_EQ(notice.body, "20% of battery remaining (1h 5m).");
    EXPECT_EQ(notice.urgency, 1);

    fakes.change(BatteryWarning::Low, 19, 62 * 60);
    settle();
    EXPECT_EQ(fakes.notices.size(), 1u);

    fakes.change(BatteryWarning::Critical, 5);
    ASSERT_TRUE(noticesReach(2));
    notice = fakes.notices[1];
    EXPECT_EQ(notice.replacesId, 1u);
    EXPECT_EQ(notice.summary, "Battery Critically Low");
    EXPECT_NE(notice.body.find("will shut down"), std::string::npos);
    EXPECT_EQ(notice.urgency, 2);

    fakes.change(BatteryWarning::Action, 2);
    ASSERT_TRUE(noticesReach(3));
    notice = fakes.notices[2];
    EXPECT_EQ(notice.replacesId, 1u);
    EXPECT_EQ(notice.summary, "Battery Empty");
    EXPECT_NE(notice.body.find("about to shut down"), std::string::npos);
    EXPECT_EQ(notice.urgency, 2);
}

TEST_F(BatteryWarningTest, UnknownTimeLeavesItOut)
{
    auto warning = start();
    fakes.change(BatteryWarning::Low, 18);
    ASSERT_TRUE(noticesReach(1));
    EXPECT_EQ(fakes.notices[0].body, "18% of battery remaining.");
}

TEST_F(BatteryWarningTest, PowerClosesTheWarning)
{
    auto warning = start();
    fakes.change(BatteryWarning::Critical, 4);
    ASSERT_TRUE(noticesReach(1));

    fakes.change(BatteryWarning::None, 4);
    EXPECT_TRUE(testsupport::waitFor(loop, [&] { return fakes.closed == std::vector<uint32_t> {1}; }));

    // The next discharge warns again from the start.
    fakes.change(BatteryWarning::Low, 15);
    ASSERT_TRUE(noticesReach(2));
    EXPECT_EQ(fakes.notices[1].replacesId, 0u);
    EXPECT_EQ(fakes.notices[1].summary, "Low Battery");
}

TEST_F(BatteryWarningTest, DismissedWarningStaysDismissed)
{
    auto warning = start();
    fakes.change(BatteryWarning::Low, 20);
    ASSERT_TRUE(noticesReach(1));

    fakes.dismiss(1);
    fakes.change(BatteryWarning::Low, 12);
    settle();
    EXPECT_EQ(fakes.notices.size(), 1u);

    fakes.change(BatteryWarning::Critical, 5);
    ASSERT_TRUE(noticesReach(2));
    EXPECT_EQ(fakes.notices[1].replacesId, 0u);

    fakes.change(BatteryWarning::None, 5);
    EXPECT_TRUE(testsupport::waitFor(loop, [&] { return fakes.closed == std::vector<uint32_t> {2}; }));
}

TEST_F(BatteryWarningTest, StartsAtTheCurrentLevel)
{
    fakes.warningLevel = BatteryWarning::Critical;
    fakes.percentage = 4;
    auto warning = start();
    ASSERT_TRUE(noticesReach(1));
    EXPECT_EQ(fakes.notices[0].summary, "Battery Critically Low");
    settle();
    EXPECT_EQ(fakes.notices.size(), 1u);
}

struct CriticalAction {
    const char *action;
    const char *critical;
    const char *empty;
};

class BatteryWarningActionTest : public BatteryWarningTest, public testing::WithParamInterface<CriticalAction>
{
};

TEST_P(BatteryWarningActionTest, NamesTheCriticalAction)
{
    const CriticalAction &expected = GetParam();
    fakes.criticalAction = expected.action;
    auto warning = start();
    fakes.change(BatteryWarning::Critical, 5);
    ASSERT_TRUE(noticesReach(1));
    EXPECT_NE(fakes.notices[0].body.find(expected.critical), std::string::npos) << fakes.notices[0].body;

    fakes.change(BatteryWarning::Action, 2);
    ASSERT_TRUE(noticesReach(2));
    EXPECT_NE(fakes.notices[1].body.find(expected.empty), std::string::npos) << fakes.notices[1].body;
}

INSTANTIATE_TEST_SUITE_P(Actions, BatteryWarningActionTest,
                         testing::Values(CriticalAction {"Hibernate", "will hibernate", "about to hibernate"},
                                         CriticalAction {"HybridSleep", "will hibernate", "about to hibernate"},
                                         CriticalAction {"Suspend", "will go to sleep", "about to go to sleep"},
                                         CriticalAction {"Ignore", "Connect to power soon.",
                                                         "Connect to power now."}));

TEST_F(BatteryWarningTest, RetriesOnceTheServerAnswers)
{
    fakes.failures = 1;
    auto warning = start();
    fakes.change(BatteryWarning::Low, 20);
    settle();
    EXPECT_EQ(fakes.notices.size(), 0u);

    fakes.change(BatteryWarning::Low, 19);
    ASSERT_TRUE(noticesReach(1));
    EXPECT_EQ(fakes.notices[0].summary, "Low Battery");
}
