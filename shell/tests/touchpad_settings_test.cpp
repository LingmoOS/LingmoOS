// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "input/touchpad_settings.h"

#include <gtest/gtest.h>
#include <limits>

#include "base/event_loop.h"
#include "base/settings.h"
#include "input_test_util.h"

using Lingmo::base::EventLoop;
using Lingmo::base::Settings;
using Lingmo::input::TouchpadSettings;
using input_test::RemoteObject;
using input_test::TemporaryDirectory;

TEST(TouchpadSettings, DefaultsFollowChromiumAndLibinput)
{
    TemporaryDirectory directory;
    ASSERT_TRUE(directory.isValid());
    TouchpadSettings settings(directory.filePath("compositor.conf"));
    EXPECT_FALSE(settings.available());
    EXPECT_TRUE(settings.enabled());
    EXPECT_FALSE(settings.disableWithMouse());
    EXPECT_TRUE(settings.disableWhileTyping());
    EXPECT_FALSE(settings.tapToClick());
    EXPECT_FALSE(settings.tapDragging());
    EXPECT_TRUE(settings.naturalScroll());
    EXPECT_DOUBLE_EQ(settings.pointerAcceleration(), 0);
    EXPECT_DOUBLE_EQ(settings.scrollSpeed(), TouchpadSettings::kDefaultScrollSpeed);
    EXPECT_TRUE(settings.twoFingerSecondaryClick());
    EXPECT_FALSE(settings.secondaryClickConfigurable());
}

TEST(TouchpadSettings, PersistsWithoutDevicesAndNotifiesOnlyOnChanges)
{
    TemporaryDirectory directory;
    ASSERT_TRUE(directory.isValid());
    const std::string file = directory.filePath("compositor.conf");
    {
        TouchpadSettings settings(file);
        int changes = 0;
        auto changesCounter = settings.settingsChanged.connect([&changes] { ++changes; });
        settings.setEnabled(false);
        settings.setEnabled(false);
        settings.setDisableWithMouse(true);
        settings.setDisableWhileTyping(false);
        settings.setTapToClick(true);
        settings.setTapDragging(true);
        settings.setNaturalScroll(false);
        settings.setPointerAcceleration(0.4);
        settings.setScrollSpeed(0.75);
        settings.setTwoFingerSecondaryClick(false);
        settings.setTwoFingerSecondaryClick(false);
        EXPECT_EQ(changes, 9);
        int availability = 0;
        auto availabilityCounter =
                settings.availableChanged.connect([&availability] { ++availability; });
        settings.setAvailable(true);
        settings.setAvailable(true);
        settings.setAvailable(false);
        EXPECT_EQ(availability, 2);
        int configurable = 0;
        auto configurableCounter = settings.secondaryClickConfigurableChanged.connect(
                [&configurable] { ++configurable; });
        settings.setSecondaryClickConfigurable(true);
        settings.setSecondaryClickConfigurable(true);
        EXPECT_EQ(configurable, 1);
        EXPECT_EQ(changes, 9);
    }
    TouchpadSettings restored(file);
    EXPECT_FALSE(restored.enabled());
    EXPECT_TRUE(restored.disableWithMouse());
    EXPECT_FALSE(restored.disableWhileTyping());
    EXPECT_TRUE(restored.tapToClick());
    EXPECT_TRUE(restored.tapDragging());
    EXPECT_FALSE(restored.naturalScroll());
    EXPECT_DOUBLE_EQ(restored.pointerAcceleration(), 0.4);
    EXPECT_DOUBLE_EQ(restored.scrollSpeed(), 0.75);
    EXPECT_FALSE(restored.twoFingerSecondaryClick());
    EXPECT_FALSE(restored.available());
    EXPECT_FALSE(restored.secondaryClickConfigurable());
}

TEST(TouchpadSettings, RejectsNonFiniteSpeedAndClampsRange)
{
    TemporaryDirectory directory;
    ASSERT_TRUE(directory.isValid());
    const std::string file = directory.filePath("compositor.conf");
    {
        Settings stored(file);
        stored.setValue("Touchpad/pointerAcceleration", "nan");
        stored.setValue("Touchpad/scrollSpeed", "inf");
    }
    TouchpadSettings settings(file);
    EXPECT_DOUBLE_EQ(settings.pointerAcceleration(), 0);
    EXPECT_DOUBLE_EQ(settings.scrollSpeed(), TouchpadSettings::kDefaultScrollSpeed);
    int changes = 0;
        auto changesCounter = settings.settingsChanged.connect([&changes] { ++changes; });
    settings.setPointerAcceleration(std::numeric_limits<double>::quiet_NaN());
    settings.setPointerAcceleration(std::numeric_limits<double>::infinity());
    EXPECT_EQ(changes, 0);
    settings.setPointerAcceleration(10);
    EXPECT_DOUBLE_EQ(settings.pointerAcceleration(), 1);
    settings.setPointerAcceleration(-10);
    EXPECT_DOUBLE_EQ(settings.pointerAcceleration(), -1);
    settings.setScrollSpeed(std::numeric_limits<double>::quiet_NaN());
    settings.setScrollSpeed(0);
    EXPECT_DOUBLE_EQ(settings.scrollSpeed(), TouchpadSettings::kMinimumScrollSpeed);
    settings.setScrollSpeed(100);
    EXPECT_DOUBLE_EQ(settings.scrollSpeed(), TouchpadSettings::kMaximumScrollSpeed);
    EXPECT_EQ(changes, 4);
}

TEST(TouchpadSettings, DBusPropertiesMethodsAndServiceRestart)
{
    EventLoop loop;
    TemporaryDirectory directory;
    ASSERT_TRUE(directory.isValid());
    const std::string file = directory.filePath("compositor.conf");
    for (int iteration = 0; iteration < 2; ++iteration) {
        TouchpadSettings settings(file);
        ASSERT_TRUE(settings.registerService());
        RemoteObject proxy(loop, "org.lingmo.Touchpad1", "/org/lingmo/Touchpad1",
                           "org.lingmo.Touchpad1");
        ASSERT_TRUE(proxy.isValid());
        EXPECT_EQ(proxy.property<bool>("tapToClick"), iteration != 0);
        const auto invoke = [&proxy](const std::string &method, auto value) {
            const auto reply = proxy.call(method, value);
            EXPECT_FALSE(reply.isError()) << method << ": " << reply.error().message;
        };
        invoke("setEnabled", false);
        invoke("setDisableWithMouse", true);
        invoke("setDisableWhileTyping", false);
        invoke("setTapToClick", true);
        invoke("setTapDragging", true);
        invoke("setNaturalScroll", false);
        invoke("setPointerAcceleration", -0.6);
        invoke("setScrollSpeed", 1.25);
        invoke("setTwoFingerSecondaryClick", false);
        EXPECT_EQ(proxy.property<bool>("enabled"), false);
        EXPECT_EQ(proxy.property<bool>("disableWithMouse"), true);
        EXPECT_EQ(proxy.property<bool>("disableWhileTyping"), false);
        EXPECT_EQ(proxy.property<bool>("tapToClick"), true);
        EXPECT_EQ(proxy.property<bool>("tapDragging"), true);
        EXPECT_EQ(proxy.property<bool>("naturalScroll"), false);
        EXPECT_EQ(proxy.property<double>("pointerAcceleration"), -0.6);
        EXPECT_EQ(proxy.property<double>("scrollSpeed"), 1.25);
        EXPECT_EQ(proxy.property<bool>("twoFingerSecondaryClick"), false);
        settings.setAvailable(true);
        EXPECT_EQ(proxy.property<bool>("available"), true);
        settings.setSecondaryClickConfigurable(true);
        EXPECT_EQ(proxy.property<bool>("secondaryClickConfigurable"), true);
    }
}
