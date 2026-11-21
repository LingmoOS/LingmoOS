// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <mutex>
#include <optional>
#include <string>

#include "input_test_support.h"
#include "pages/touchpad/touchpad.h"
#include "ui/window.h"
#include "widgets/sliderrow.h"
#include "window/settingswindow.h"

using namespace Lingmo::base;
using namespace Lingmo::ui;
using namespace settings;
using settings::testing::ServiceThread;
using settings::testing::waitFor;

namespace {

// The compositor's org.lingmo.Touchpad1 as a stand-in, for a pad with
// physical buttons: no choice of secondary click.
struct FakeTouchpad
{
    std::mutex mutex;
    bool tapToClick = false;
    double pointerAcceleration = 0;
    double scrollSpeed = 0.5;
    dbus::Object* object = nullptr;
};

} // namespace

static std::shared_ptr<void> serve(dbus::Connection& bus, FakeTouchpad& fake)
{
    auto object = std::make_shared<dbus::Object>(&bus, "/org/lingmo/Touchpad1", "org.lingmo.Touchpad1");
    object->addProperty("available", [] { return true; });
    object->addProperty("enabled", [] { return true; });
    object->addProperty("disableWithMouse", [] { return false; });
    object->addProperty("disableWhileTyping", [] { return true; });
    object->addProperty("tapToClick", [&fake] {
        std::lock_guard guard(fake.mutex);
        return fake.tapToClick;
    });
    object->addProperty("tapDragging", [] { return false; });
    object->addProperty("naturalScroll", [] { return true; });
    object->addProperty("pointerAcceleration", [&fake] {
        std::lock_guard guard(fake.mutex);
        return fake.pointerAcceleration;
    });
    object->addProperty("scrollSpeed", [&fake] {
        std::lock_guard guard(fake.mutex);
        return fake.scrollSpeed;
    });
    object->addProperty("secondaryClickConfigurable", [] { return false; });
    object->addProperty("twoFingerSecondaryClick", [] { return true; });
    object->addSignal("tapToClickChanged", "");
    object->addSignal("scrollSpeedChanged", "");
    object->addMethod("setTapToClick", [&fake, object = object.get()](bool enabled) {
        {
            std::lock_guard guard(fake.mutex);
            fake.tapToClick = enabled;
        }
        object->emitSignal("tapToClickChanged");
    });
    object->addMethod("setScrollSpeed", [&fake, object = object.get()](double speed) {
        {
            std::lock_guard guard(fake.mutex);
            fake.scrollSpeed = speed;
        }
        object->emitSignal("scrollSpeedChanged");
    });
    object->publish();
    fake.object = object.get();
    return object;
}

namespace {

struct TouchpadTest : ::testing::Test
{
    void SetUp() override
    {
        if (!settings::testing::hasSessionBus())
            GTEST_SKIP() << "no session bus";
        service.emplace("org.lingmo.Touchpad1", [this](dbus::Connection& bus) { return serve(bus, fake); });
        ASSERT_TRUE(service->start());
    }

    void TearDown() override
    {
        if (!service)
            return;
        service.reset();
        settings::testing::waitForNameGone("org.lingmo.Touchpad1");
    }

    FakeTouchpad fake;
    std::optional<ServiceThread> service;
};

} // namespace

TEST_F(TouchpadTest, PageFollowsWhatThePadCanDo)
{
    SettingsWindow settingsWindow;
    Window& window = settingsWindow.window();
    window.setScale(1.5);
    window.resize({900, 610});
    settingsWindow.reveal();
    settingsWindow.switchToPage("touchpad");
    window.grab();

    // Both cards, the second without the secondary click row.
    EXPECT_EQ(settings::testing::windowRect(window, "touchpad:device").height, 3 * 44);
    const RectF gestures = settings::testing::windowRect(window, "touchpad:gestures");
    EXPECT_EQ(gestures.y, 56 + 3 * 44 + 24);
    EXPECT_EQ(gestures.height, 3 * 44 + 2 * 52);
    EXPECT_TRUE(settings::testing::isHidden(window, "touchpad:secondaryClick"));
    // Tap and drag needs tapping on.
    EXPECT_FALSE(settings::testing::findView(window.contentView(), "touchpad:tapDragging")->isEnabled());

    // Half of libinput's speed is the fourth of ten steps.
    auto* scroll = settings::testing::findView<SliderRow>(window.contentView(), "touchpad:scrollSpeed");
    ASSERT_NE(scroll, nullptr);
    EXPECT_EQ(scroll->value(), 3);
    {
        std::lock_guard guard(fake.mutex);
        fake.tapToClick = true;
        fake.scrollSpeed = 1;
    }
    service->post([this] {
        fake.object->emitSignal("tapToClickChanged");
        fake.object->emitSignal("scrollSpeedChanged");
    });
    ASSERT_TRUE(waitFor([&] { return scroll->value() == 6; }));
    EXPECT_TRUE(settings::testing::findView(window.contentView(), "touchpad:tapDragging")->isEnabled());
}
