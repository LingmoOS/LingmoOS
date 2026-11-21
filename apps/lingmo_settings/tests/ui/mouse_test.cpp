// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <mutex>
#include <optional>
#include <string>

#include "input_test_support.h"
#include "pages/mouse/mouse.h"
#include "ui/switch.h"
#include "ui/test_input.h"
#include "ui/window.h"
#include "widgets/listrow.h"
#include "widgets/sliderrow.h"
#include "window/settingswindow.h"

using namespace Lingmo::base;
using namespace Lingmo::ui;
using namespace settings;
using settings::testing::ServiceThread;
using settings::testing::waitFor;

namespace {

// The compositor's org.lingmo.Mouse1 as a stand-in: its settings outlive a
// restart of the service, as the compositor keeps them in a file.
struct FakeMouse
{
    std::mutex mutex;
    bool shakeToLocate = true;
    bool leftHanded = false;
    bool naturalScroll = true;
    bool acceleration = true;
    double pointerAcceleration = 0;
    // On the service thread only.
    dbus::Object* object = nullptr;
};

} // namespace

template<typename T>
static void addSetting(dbus::Object& object, FakeMouse& fake, const std::string& name, const std::string& setter,
                       T FakeMouse::*field)
{
    object.addProperty(name, [&fake, field] {
        std::lock_guard guard(fake.mutex);
        return fake.*field;
    });
    object.addSignal(name + "Changed", "");
    object.addMethod(setter, [&fake, &object, field, name](T value) {
        {
            std::lock_guard guard(fake.mutex);
            fake.*field = value;
        }
        object.emitSignal(name + "Changed");
    });
}

static std::shared_ptr<void> serve(dbus::Connection& bus, FakeMouse& fake)
{
    auto object = std::make_shared<dbus::Object>(&bus, "/org/lingmo/Mouse1", "org.lingmo.Mouse1");
    object->addProperty("available", [] { return true; });
    object->addSignal("availableChanged", "");
    addSetting(*object, fake, "shakeToLocate", "setShakeToLocate", &FakeMouse::shakeToLocate);
    addSetting(*object, fake, "leftHanded", "setLeftHanded", &FakeMouse::leftHanded);
    addSetting(*object, fake, "naturalScroll", "setNaturalScroll", &FakeMouse::naturalScroll);
    addSetting(*object, fake, "acceleration", "setAcceleration", &FakeMouse::acceleration);
    addSetting(*object, fake, "pointerAcceleration", "setPointerAcceleration", &FakeMouse::pointerAcceleration);
    object->publish();
    fake.object = object.get();
    return object;
}

namespace {

struct MouseTest : ::testing::Test
{
    void SetUp() override
    {
        if (!settings::testing::hasSessionBus())
            GTEST_SKIP() << "no session bus";
        service.emplace("org.lingmo.Mouse1", [this](dbus::Connection& bus) { return serve(bus, fake); });
    }

    void TearDown() override
    {
        if (!service)
            return;
        service.reset();
        settings::testing::waitForNameGone("org.lingmo.Mouse1");
    }

    FakeMouse fake;
    std::optional<ServiceThread> service;
};

} // namespace

TEST_F(MouseTest, FollowsTheServiceThroughChangesAndRestarts)
{
    Mouse first;
    Mouse second;
    EXPECT_FALSE(first.isAvailable());

    ASSERT_TRUE(service->start());
    ASSERT_TRUE(waitFor([&] { return first.isAvailable() && second.isAvailable(); }));
    EXPECT_FALSE(second.leftHanded());
    EXPECT_TRUE(second.naturalScroll());

    first.setLeftHanded(true);
    first.setNaturalScroll(false);
    first.setAcceleration(false);
    first.setPointerAcceleration(0.6);
    ASSERT_TRUE(waitFor([&] {
        return second.leftHanded() && !second.naturalScroll() && !second.acceleration()
                && second.pointerAcceleration() == 0.6;
    }));

    service->stop();
    ASSERT_TRUE(waitFor([&] { return !first.isAvailable() && !second.isAvailable(); }));
    // Without the service the switches show the compositor's defaults.
    EXPECT_TRUE(first.acceleration());
    EXPECT_FALSE(first.leftHanded());

    ASSERT_TRUE(service->start());
    ASSERT_TRUE(waitFor([&] { return first.isAvailable() && second.isAvailable(); }));
    EXPECT_TRUE(first.leftHanded());
    EXPECT_FALSE(first.naturalScroll());
    EXPECT_FALSE(first.acceleration());
    EXPECT_EQ(first.pointerAcceleration(), 0.6);

    second.setAcceleration(true);
    ASSERT_TRUE(waitFor([&] { return first.acceleration(); }));
}

TEST_F(MouseTest, PageLaysOutAsTheQtVersionAndSetsTheService)
{
    {
        std::lock_guard guard(fake.mutex);
        fake.pointerAcceleration = 0.6;
    }
    ASSERT_TRUE(service->start());
    SettingsWindow settingsWindow;
    Window& window = settingsWindow.window();
    window.setScale(1.5);
    window.resize({900, 610});
    settingsWindow.reveal();
    settingsWindow.switchToPage("mouse");
    window.grab();

    // The list card under the header, three switch rows and a slider row.
    const RectF card = settings::testing::windowRect(window, "mouse:card");
    EXPECT_EQ(card.x, 254);
    EXPECT_EQ(card.y, 56);
    EXPECT_EQ(card.width, 622);
    EXPECT_EQ(card.height, 3 * 44 + 52);
    EXPECT_EQ(settings::testing::windowRect(window, "mouse:speed").height, 52);
    // Speed 0.6 is the seventh of the ten curves after the default.
    auto* speed = settings::testing::findView<SliderRow>(window.contentView(), "mouse:speed");
    ASSERT_NE(speed, nullptr);
    EXPECT_EQ(speed->value(), 7);

    auto* leftHanded = settings::testing::findView<SwitchRow>(window.contentView(), "mouse:leftHanded");
    ASSERT_NE(leftHanded, nullptr);
    const RectF control = leftHanded->control()->geometry();
    TestInput::click(window, leftHanded->control()->mapToWindow({control.width / 2, control.height / 2}));
    ASSERT_TRUE(waitFor([&] {
        std::lock_guard guard(fake.mutex);
        return fake.leftHanded;
    }));

    // Without the service the card is there but disabled.
    service->stop();
    ASSERT_TRUE(waitFor([&] { return !settings::testing::findView(window.contentView(), "mouse:card")->isEnabled(); }));
}

TEST_F(MouseTest, ShakeLocatorSwitchFollowsServiceAndPersistsAcrossRestart)
{
    ASSERT_TRUE(service->start());
    SettingsWindow settingsWindow;
    auto &window = settingsWindow.window();
    settingsWindow.reveal(); settingsWindow.switchToPage("mouse"); window.grab();
    auto *row = settings::testing::findView<SwitchRow>(window.contentView(), "mouse:shakeToLocate");
    ASSERT_NE(nullptr, row);
    ASSERT_TRUE(waitFor([&] { return row->isEnabled(); }));
    EXPECT_TRUE(row->isChecked());
    row->onToggled(false);
    ASSERT_TRUE(waitFor([&] { return !row->isChecked(); }));
    service->stop();
    ASSERT_TRUE(waitFor([&] { return !row->isEnabled(); }));
    ASSERT_TRUE(service->start());
    ASSERT_TRUE(waitFor([&] { return row->isEnabled() && !row->isChecked(); }));
}
