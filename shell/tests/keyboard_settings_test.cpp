// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "input/keyboard_settings.h"

#include <gtest/gtest.h>

#include "base/event_loop.h"
#include "base/settings.h"
#include "input_test_util.h"

using Lingmo::base::EventLoop;
using Lingmo::base::Settings;
using Lingmo::input::KeyboardSettings;
using Lingmo::input::ShortcutConvention;
using input_test::RemoteObject;
using input_test::TemporaryDirectory;

TEST(KeyboardSettings, DefaultsAndClampedStoredValues)
{
    TemporaryDirectory directory;
    ASSERT_TRUE(directory.isValid());
    const std::string file = directory.filePath("compositor.conf");
    {
        KeyboardSettings settings(file);
        EXPECT_TRUE(settings.repeatEnabled());
        EXPECT_EQ(settings.repeatDelay(), 500);
        EXPECT_EQ(settings.repeatInterval(), 40);
        EXPECT_FALSE(settings.layout().empty());
    }
    {
        Settings stored(file);
        stored.setValue("Keyboard/repeatDelay", 1);
        stored.setValue("Keyboard/repeatInterval", "fast");
    }
    KeyboardSettings settings(file);
    EXPECT_EQ(settings.repeatDelay(), KeyboardSettings::kMinRepeatDelayMs);
    EXPECT_EQ(settings.repeatInterval(), 40);
}

TEST(KeyboardSettings, RepeatPersistsAndNotifiesOnlyOnChanges)
{
    TemporaryDirectory directory;
    ASSERT_TRUE(directory.isValid());
    const std::string file = directory.filePath("compositor.conf");
    {
        KeyboardSettings settings(file);
        int delay = 0;
        auto delayCounter = settings.repeatDelayChanged.connect([&delay] { ++delay; });
        int interval = 0;
        auto intervalCounter = settings.repeatIntervalChanged.connect([&interval] { ++interval; });
        int enabled = 0;
        auto enabledCounter = settings.repeatEnabledChanged.connect([&enabled] { ++enabled; });
        settings.setRepeatDelay(300);
        settings.setRepeatDelay(300);
        settings.setRepeatInterval(100000);
        settings.setRepeatEnabled(false);
        EXPECT_EQ(delay, 1);
        EXPECT_EQ(interval, 1);
        EXPECT_EQ(enabled, 1);
        EXPECT_EQ(settings.repeatInterval(), KeyboardSettings::kMaxRepeatIntervalMs);
    }
    KeyboardSettings restored(file);
    EXPECT_FALSE(restored.repeatEnabled());
    EXPECT_EQ(restored.repeatDelay(), 300);
    EXPECT_EQ(restored.repeatInterval(), KeyboardSettings::kMaxRepeatIntervalMs);
}

TEST(KeyboardSettings, LayoutIsSavedOnlyWhenItApplies)
{
    TemporaryDirectory directory;
    ASSERT_TRUE(directory.isValid());
    const std::string file = directory.filePath("compositor.conf");
    std::vector<std::string> applied;
    const auto applier = [&applied](const std::string &name) {
        if (name == "broken")
            return false;
        applied.push_back(name);
        return true;
    };
    {
        KeyboardSettings settings(file);
        const std::string fallback = settings.layout();
        EXPECT_FALSE(settings.setLayout("de"));
        settings.setLayoutApplier(applier);
        // Nothing saved, so nothing is installed up front.
        EXPECT_TRUE(applied.empty());

        int changes = 0;
        auto changesCounter = settings.layoutChanged.connect([&changes] { ++changes; });
        EXPECT_FALSE(settings.setLayout("broken"));
        EXPECT_EQ(settings.layout(), fallback);
        EXPECT_TRUE(settings.setLayout("de-neo"));
        EXPECT_TRUE(settings.setLayout("de-neo"));
        EXPECT_EQ(changes, 1);
        EXPECT_EQ(settings.layout(), "de-neo");
    }
    applied.clear();
    {
        KeyboardSettings restored(file);
        restored.setLayoutApplier(applier);
        EXPECT_EQ(applied, std::vector<std::string> {"de-neo"});
        EXPECT_TRUE(restored.setLayout({}));
        EXPECT_NE(restored.layout(), "de-neo");
    }
    Settings stored(file);
    EXPECT_FALSE(stored.contains("Keyboard/layout"));
}

TEST(KeyboardSettings, ASavedLayoutThatNoLongerAppliesFallsBack)
{
    TemporaryDirectory directory;
    ASSERT_TRUE(directory.isValid());
    const std::string file = directory.filePath("compositor.conf");
    {
        Settings stored(file);
        stored.setValue("Keyboard/layout", "broken");
    }
    KeyboardSettings settings(file);
    EXPECT_EQ(settings.layout(), "broken");
    int changes = 0;
        auto changesCounter = settings.layoutChanged.connect([&changes] { ++changes; });
    settings.setLayoutApplier([](const std::string &) {
        return false;
    });
    EXPECT_EQ(changes, 1);
    EXPECT_NE(settings.layout(), "broken");
}

TEST(KeyboardSettings, TheShortcutConventionIsSavedAndStartsAsPc)
{
    TemporaryDirectory directory;
    ASSERT_TRUE(directory.isValid());
    const std::string file = directory.filePath("compositor.conf");
    {
        KeyboardSettings settings(file);
        EXPECT_EQ(settings.shortcutConvention(), ShortcutConvention::Pc);
        int changes = 0;
        auto changesCounter = settings.shortcutConventionChanged.connect([&changes] { ++changes; });
        EXPECT_FALSE(settings.setShortcutConvention("mac"));
        EXPECT_TRUE(settings.setShortcutConvention("command"));
        EXPECT_TRUE(settings.setShortcutConvention("command"));
        EXPECT_EQ(changes, 1);
    }
    {
        KeyboardSettings restored(file);
        EXPECT_EQ(restored.shortcutConvention(), ShortcutConvention::Command);
        EXPECT_EQ(restored.shortcutConventionName(), "command");
        EXPECT_TRUE(restored.setShortcutConvention("pc"));
    }
    Settings stored(file);
    EXPECT_FALSE(stored.contains("Keyboard/shortcutConvention"));
}

TEST(KeyboardSettings, DBusPropertiesAndMethods)
{
    EventLoop loop;
    TemporaryDirectory directory;
    ASSERT_TRUE(directory.isValid());
    KeyboardSettings settings(directory.filePath("compositor.conf"));
    settings.setLayoutApplier([](const std::string &name) {
        return name != "broken";
    });
    ASSERT_TRUE(settings.registerService());
    RemoteObject proxy(loop, "org.lingmo.Keyboard1", "/org/lingmo/Keyboard1",
                       "org.lingmo.Keyboard1");
    ASSERT_TRUE(proxy.isValid());
    const auto invoke = [&proxy](const std::string &method, auto value) {
        const auto reply = proxy.call(method, value);
        EXPECT_FALSE(reply.isError()) << method << ": " << reply.error().message;
        return reply;
    };
    invoke("setRepeatEnabled", false);
    invoke("setRepeatDelay", int32_t(250));
    invoke("setRepeatInterval", int32_t(25));
    EXPECT_EQ(invoke("setLayout", std::string("broken")).value<bool>(), false);
    EXPECT_EQ(invoke("setLayout", std::string("fr")).value<bool>(), true);
    EXPECT_EQ(invoke("setShortcutConvention", std::string("command")).value<bool>(), true);
    EXPECT_EQ(proxy.property<bool>("repeatEnabled"), false);
    EXPECT_EQ(proxy.property<int32_t>("repeatDelay"), 250);
    EXPECT_EQ(proxy.property<int32_t>("repeatInterval"), 25);
    EXPECT_EQ(proxy.property<std::string>("layout"), "fr");
    EXPECT_EQ(proxy.property<std::string>("shortcutConvention"), "command");
}

// The argument names are the methods' parameter names, kept for existing
// callers. sd-bus names all arguments or none, so return values are "result".
TEST(KeyboardSettings, IntrospectionNamesTheArguments)
{
    EventLoop loop;
    TemporaryDirectory directory;
    ASSERT_TRUE(directory.isValid());
    KeyboardSettings settings(directory.filePath("compositor.conf"));
    ASSERT_TRUE(settings.registerService());
    RemoteObject proxy(loop, "org.lingmo.Keyboard1", "/org/lingmo/Keyboard1",
                       "org.lingmo.Keyboard1");
    ASSERT_TRUE(proxy.isValid());
    const std::string xml = proxy.introspect();
    EXPECT_NE(xml.find("<method name=\"setRepeatDelay\">\n"
                       "   <arg type=\"i\" name=\"milliseconds\" direction=\"in\"/>\n"
                       "  </method>"),
              std::string::npos)
            << xml;
    EXPECT_NE(xml.find("<method name=\"setLayout\">\n"
                       "   <arg type=\"s\" name=\"name\" direction=\"in\"/>\n"
                       "   <arg type=\"b\" name=\"result\" direction=\"out\"/>\n"
                       "  </method>"),
              std::string::npos)
            << xml;
}
