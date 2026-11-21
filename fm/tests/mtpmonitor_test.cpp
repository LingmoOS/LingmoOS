// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mtpmonitor.h"

#include "base/event_loop.h"

#include <gtest/gtest.h>

using namespace Lingmo;

TEST(MtpMonitorTest, DeviceBackWithinTheDelayStartsANewSession)
{
    base::EventLoop loop;
    MtpMonitor monitor;
    std::vector<std::string> events;
    const base::Subscription added = monitor.added.connect([&events](const std::string &id) {
        events.push_back("added " + id);
    });
    const base::Subscription removed = monitor.removed.connect([&events](const std::string &id) {
        events.push_back("removed " + id);
    });

    MtpMonitor::Device device;
    device.id = "mtp://test-serial";
    device.name = "Phone";
    device.bus = 1;
    device.number = 7;
    monitor.arrive("/sys/test/1-1", device);
    EXPECT_EQ(events, std::vector<std::string> {"added mtp://test-serial"});

    events.clear();
    monitor.leave("/sys/test/1-1");
    EXPECT_TRUE(events.empty());
    EXPECT_EQ(monitor.device(device.id).bus, 0u);

    // Plugged in again at once, as when access is allowed on the phone's screen.
    device.number = 8;
    monitor.arrive("/sys/test/1-1", device);
    EXPECT_EQ(events, (std::vector<std::string> {"removed mtp://test-serial", "added mtp://test-serial"}));
    EXPECT_EQ(monitor.device(device.id).number, 8);

    // Seen again without leaving, as udev's change events come.
    events.clear();
    monitor.arrive("/sys/test/1-1", device);
    EXPECT_TRUE(events.empty());
}
