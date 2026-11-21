// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "input/touchscreen_settings.h"

#include <gtest/gtest.h>

#include "base/event_loop.h"
#include "input_test_util.h"

using Lingmo::base::EventLoop;
using Lingmo::input::TouchscreenSettings;
using input_test::RemoteObject;
using input_test::TemporaryDirectory;
using Names = std::vector<std::string>;

namespace {
const std::string elan = "ELAN2514:00 04F3:2AF1";
const std::string wacom = "Wacom HID 5256 Finger";
} // namespace

TEST(TouchscreenSettings, EveryDeviceStartsEnabled)
{
    TemporaryDirectory directory;
    ASSERT_TRUE(directory.isValid());
    TouchscreenSettings settings(directory.filePath("compositor.conf"));
    EXPECT_TRUE(settings.devices().empty());
    EXPECT_TRUE(settings.disabledDevices().empty());
    EXPECT_TRUE(settings.deviceEnabled(elan));
}

TEST(TouchscreenSettings, PersistsPerDeviceAndNotifiesOnlyOnChanges)
{
    TemporaryDirectory directory;
    ASSERT_TRUE(directory.isValid());
    const std::string file = directory.filePath("compositor.conf");
    {
        TouchscreenSettings settings(file);
        int changes = 0;
        auto changesCounter = settings.settingsChanged.connect([&changes] { ++changes; });
        int disabled = 0;
        auto disabledCounter = settings.disabledDevicesChanged.connect([&disabled] { ++disabled; });
        settings.setDeviceEnabled(elan, false);
        settings.setDeviceEnabled(elan, false);
        settings.setDeviceEnabled(wacom, true);
        settings.setDeviceEnabled({}, false);
        EXPECT_EQ(changes, 1);
        EXPECT_EQ(disabled, 1);
        EXPECT_FALSE(settings.deviceEnabled(elan));
        EXPECT_TRUE(settings.deviceEnabled(wacom));

        int devices = 0;
        auto devicesCounter = settings.devicesChanged.connect([&devices] { ++devices; });
        settings.setDevices({elan});
        settings.setDevices({elan});
        settings.setDevices({});
        EXPECT_EQ(devices, 2);
        EXPECT_EQ(changes, 1);
    }
    {
        TouchscreenSettings restored(file);
        EXPECT_EQ(restored.disabledDevices(), Names {elan});
        EXPECT_TRUE(restored.devices().empty());
        restored.setDeviceEnabled(elan, true);
    }
    TouchscreenSettings restored(file);
    EXPECT_TRUE(restored.disabledDevices().empty());
}

TEST(TouchscreenSettings, DBusPropertiesMethodsAndServiceRestart)
{
    EventLoop loop;
    TemporaryDirectory directory;
    ASSERT_TRUE(directory.isValid());
    const std::string file = directory.filePath("compositor.conf");
    for (int iteration = 0; iteration < 2; ++iteration) {
        TouchscreenSettings settings(file);
        ASSERT_TRUE(settings.registerService());
        RemoteObject proxy(loop, "org.lingmo.Touchscreen1", "/org/lingmo/Touchscreen1",
                           "org.lingmo.Touchscreen1");
        ASSERT_TRUE(proxy.isValid());
        EXPECT_EQ(proxy.property<Names>("disabledDevices").value_or(Names {"?"}).empty(),
                  iteration == 0);
        settings.setDevices({elan, wacom});
        EXPECT_EQ(proxy.property<Names>("devices"), (Names {elan, wacom}));

        const auto reply = proxy.call("setDeviceEnabled", wacom, false);
        EXPECT_FALSE(reply.isError()) << reply.error().message;
        EXPECT_EQ(proxy.property<Names>("disabledDevices"), Names {wacom});
        EXPECT_FALSE(settings.deviceEnabled(wacom));
    }
}
