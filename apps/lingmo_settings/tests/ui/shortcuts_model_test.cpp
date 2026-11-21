// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// ShortcutsModel against a stand-in for the compositor's
// org.lingmo.Shortcuts1 service, answering in the JSON the real one
// produces.

#include <gtest/gtest.h>

#include <algorithm>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "input_test_support.h"
#include "pages/shortcuts/shortcuteditdialog.h"
#include "pages/shortcuts/shortcutsmodel.h"
#include "ui/keys.h"
#include "ui/test_input.h"
#include "ui/window.h"
#include "window/settingswindow.h"

using namespace Lingmo::base;
using namespace Lingmo::ui;
using namespace settings;
using settings::testing::ServiceThread;
using settings::testing::waitFor;

static std::string accelerator(int keysym, const std::string& keyName, const std::string& keyText, int modifiers,
                               const std::string& type = "default")
{
    return "{\"keysym\":" + std::to_string(keysym) + ",\"keyName\":\"" + keyName + "\",\"keyText\":\"" + keyText
            + "\",\"modifiers\":" + std::to_string(modifiers) + ",\"keyState\":\"pressed\",\"type\":\"" + type
            + "\"}";
}

namespace {

// What the stand-in serves; shared with the test thread under the mutex.
struct FakeShortcuts
{
    std::mutex mutex;
    std::string convention = "pc";
    std::string maximize = "[" + accelerator(0x3d, "equal", "=", 4) + "," + accelerator(0xff52, "Up", "", 8) + "]";
    std::string maximizeDefaults = maximize;
    std::string lock = "[" + accelerator(0x6c, "l", "L", 8) + "]";
    std::vector<std::string> calls;
    std::string nextResult = "{\"result\":\"success\"}";
    // On the service thread only.
    dbus::Object* object = nullptr;

    std::vector<std::string> takeCalls()
    {
        std::lock_guard guard(mutex);
        return calls;
    }

    void record(std::string call)
    {
        std::lock_guard guard(mutex);
        calls.push_back(std::move(call));
    }

    std::string accelerators()
    {
        std::lock_guard guard(mutex);
        return "{\"actions\":[{\"name\":\"lock-screen\",\"accelerators\":" + lock + ",\"defaults\":" + lock
                + "},{\"name\":\"toggle-maximized\",\"accelerators\":" + maximize
                + ",\"defaults\":" + maximizeDefaults
                + "},{\"name\":\"not-a-lingmo-action\",\"accelerators\":[],\"defaults\":[]}]}";
    }
};

} // namespace

static std::shared_ptr<void> serve(dbus::Connection& bus, FakeShortcuts& fake)
{
    auto object = std::make_shared<dbus::Object>(&bus, "/org/lingmo/Shortcuts1", "org.lingmo.Shortcuts1");
    object->addProperty("shortcutConvention", [&fake] {
        std::lock_guard guard(fake.mutex);
        return fake.convention;
    });
    object->addMethod("accelerators", [&fake] { return fake.accelerators(); });
    object->addMethod("addAccelerator", [&fake](const std::string& action, uint32_t keycode, uint32_t keysym,
                                                uint32_t modifiers) {
        fake.record("add " + action + " " + std::to_string(keycode) + " " + std::to_string(keysym) + " "
                    + std::to_string(modifiers));
        std::lock_guard guard(fake.mutex);
        return fake.nextResult;
    });
    object->addMethod("removeAccelerator", [&fake](const std::string& action, uint32_t keysym, uint32_t modifiers) {
        fake.record("remove " + action + " " + std::to_string(keysym) + " " + std::to_string(modifiers));
        std::lock_guard guard(fake.mutex);
        return fake.nextResult;
    });
    object->addMethod("replaceAccelerator", [&fake](const std::string& action, uint32_t oldKeysym,
                                                    uint32_t oldModifiers, uint32_t keycode, uint32_t keysym,
                                                    uint32_t modifiers) {
        fake.record("replace " + action + " " + std::to_string(oldKeysym) + " " + std::to_string(oldModifiers) + " "
                    + std::to_string(keycode) + " " + std::to_string(keysym) + " " + std::to_string(modifiers));
        std::lock_guard guard(fake.mutex);
        return fake.nextResult;
    });
    object->addMethod("restoreDefault", [&fake](const std::string& action) {
        fake.record("restore " + action);
        std::lock_guard guard(fake.mutex);
        return fake.nextResult;
    });
    object->addMethod("restoreAllDefaults", [&fake] {
        fake.record("restore-all");
        std::lock_guard guard(fake.mutex);
        return fake.nextResult;
    });
    object->addMethod("describeKey", [&fake](uint32_t keycode, uint32_t keysym) {
        fake.record("describe " + std::to_string(keycode) + " " + std::to_string(keysym));
        return std::string("{\"keysym\":113,\"keyName\":\"q\",\"keyText\":\"Q\"}");
    });
    object->addMethod("preventProcessingAccelerators",
                      [&fake](bool prevent) { fake.record("prevent " + std::to_string(prevent ? 1 : 0)); });
    object->addMethod("hasCustomAccelerators", [&fake] {
        std::lock_guard guard(fake.mutex);
        return fake.maximize != fake.maximizeDefaults;
    });
    object->addSignal("acceleratorsChanged", "");
    object->publish();
    fake.object = object.get();
    return object;
}

namespace {

struct ShortcutsModelTest : ::testing::Test
{
    void SetUp() override
    {
        if (!settings::testing::hasSessionBus())
            GTEST_SKIP() << "no session bus";
        service.emplace("org.lingmo.Shortcuts1", [this](dbus::Connection& bus) { return serve(bus, fake); });
    }

    void TearDown() override
    {
        if (!service)
            return;
        service.reset();
        settings::testing::waitForNameGone("org.lingmo.Shortcuts1");
    }

    FakeShortcuts fake;
    std::optional<ServiceThread> service;
};

} // namespace

TEST_F(ShortcutsModelTest, ListsKnownActionsInLayoutOrderWithReadableKeys)
{
    ASSERT_TRUE(service->start());
    ShortcutsModel model;
    ASSERT_TRUE(waitFor([&] { return model.isAvailable() && model.actions().size() == 2; }));
    // Layout order, not the service's order: lock-screen is in General after
    // the launcher entries, toggle-maximized is in Windows.
    EXPECT_EQ(model.actions()[0].name, "lock-screen");
    EXPECT_EQ(model.actions()[1].name, "toggle-maximized");
    EXPECT_EQ(model.actions()[1].category, "Windows");
    EXPECT_TRUE(model.actions()[1].isDefault());

    const std::vector<ShortcutsModel::Accelerator>& accelerators = model.actions()[1].accelerators;
    ASSERT_EQ(accelerators.size(), 2u);
    EXPECT_EQ(accelerators[0].parts, (std::vector<std::string>{"Alt", "="}));
    EXPECT_EQ(accelerators[1].parts, (std::vector<std::string>{"Meta", "↑"}));
    EXPECT_EQ(accelerators[1].text, "Meta + ↑");
    EXPECT_FALSE(model.hasCustomAccelerators());

    // A change on the service side reloads the model.
    {
        std::lock_guard guard(fake.mutex);
        fake.maximize = "[" + accelerator(0x6b, "k", "K", 8, "user") + "]";
    }
    service->post([this] { fake.object->emitSignal("acceleratorsChanged"); });
    ASSERT_TRUE(waitFor([&] { return model.hasCustomAccelerators(); }));
    EXPECT_FALSE(model.actions()[1].isDefault());

    // Requests reach the service with the compositor's modifier bits, and
    // results come back as messages the dialog shows.
    ShortcutsModel::Result result = model.addAccelerator("toggle-maximized", 16, 0, ShortcutsModel::MetaDown);
    EXPECT_EQ(result.result, "success");
    EXPECT_TRUE(result.message.empty());
    {
        std::lock_guard guard(fake.mutex);
        fake.nextResult = "{\"result\":\"conflict-can-override\",\"conflictAction\":\"lock-screen\"}";
    }
    result = model.replaceAccelerator("toggle-maximized", 0x6b, 8, 38, 0, 8);
    EXPECT_EQ(result.result, "conflict-can-override");
    EXPECT_EQ(result.conflictDescription, "Lock the screen");
    EXPECT_NE(result.message.find("Lock the screen"), std::string::npos);
    model.removeAccelerator("toggle-maximized", 0x6b, 8);
    model.restoreDefault("toggle-maximized");
    model.restoreAllDefaults();
    model.setRecording(true);
    EXPECT_EQ(model.partsForKey(16, 'q', ShortcutsModel::ControlDown | ShortcutsModel::ShiftDown),
              (std::vector<std::string>{"Ctrl", "Shift", "Q"}));
    model.setRecording(false);
    ASSERT_TRUE(waitFor([&] { return fake.takeCalls().size() == 8; }));
    const std::vector<std::string> calls = fake.takeCalls();
    EXPECT_EQ(calls[0], "add toggle-maximized 16 0 8");
    EXPECT_EQ(calls[1], "replace toggle-maximized 107 8 38 0 8");
    EXPECT_EQ(calls[2], "remove toggle-maximized 107 8");
    EXPECT_EQ(calls[3], "restore toggle-maximized");
    EXPECT_EQ(calls[4], "restore-all");
    EXPECT_EQ(calls[5], "prevent 1");
    EXPECT_EQ(calls[6], "describe 16 81");
    EXPECT_EQ(calls[7], "prevent 0");

    EXPECT_EQ(ShortcutsModel::serviceKeysym(0xffc1), 0xffc1u);
    EXPECT_EQ(ShortcutsModel::serviceKeysym(0xff52), 0xff52u);
    EXPECT_EQ(ShortcutsModel::serviceKeysym('q'), uint32_t('Q'));
    EXPECT_TRUE(ShortcutsModel::isModifierKey(0xffeb));
    EXPECT_FALSE(ShortcutsModel::isModifierKey('q'));

    service->stop();
    ASSERT_TRUE(waitFor([&] { return !model.isAvailable(); }));
    EXPECT_TRUE(model.actions().empty());
}

TEST_F(ShortcutsModelTest, TheCommandConventionNamesTheKeys)
{
    fake.convention = "command";
    ASSERT_TRUE(service->start());
    ShortcutsModel model;
    ASSERT_TRUE(waitFor([&] { return model.isAvailable() && model.commandKeys(); }));
    EXPECT_EQ(model.modifierParts(ShortcutsModel::ControlDown | ShortcutsModel::AltDown | ShortcutsModel::MetaDown),
              (std::vector<std::string>{"Control", "Option", "Command"}));
}

TEST_F(ShortcutsModelTest, ReportsTheServiceMissing)
{
    ShortcutsModel model;
    EXPECT_FALSE(model.isAvailable());
    EXPECT_TRUE(model.actions().empty());
    const ShortcutsModel::Result result = model.restoreAllDefaults();
    EXPECT_EQ(result.result, "unavailable");
    EXPECT_FALSE(result.message.empty());
}

TEST_F(ShortcutsModelTest, PageListsTheActionsInTheirSections)
{
    ASSERT_TRUE(service->start());
    SettingsWindow settingsWindow;
    Window& window = settingsWindow.window();
    window.setScale(1.5);
    window.resize({900, 610});
    settingsWindow.reveal();
    settingsWindow.pushPage("shortcuts");
    window.grab();
    // One shortcut fits the plain row; two stack, with 4 px between and around them.
    EXPECT_EQ(settings::testing::windowRect(window, "shortcut:lock-screen").height, 44);
    EXPECT_EQ(settings::testing::windowRect(window, "shortcut:toggle-maximized").height, 72);
    EXPECT_FALSE(settings::testing::findView(window.contentView(), "shortcuts:restoreAll")->isEnabled());
}

TEST_F(ShortcutsModelTest, DialogRecordsAShortcutWithItsKeyCode)
{
    ASSERT_TRUE(service->start());
    SettingsWindow settingsWindow;
    settingsWindow.window().setScale(1.5);
    ShortcutsModel model;
    ShortcutEditDialog dialog(settingsWindow, model, "toggle-maximized");
    dialog.show();
    Window& window = *dialog.window();
    window.setScale(1.5);
    TestInput::setKeyboardFocus(window, true);
    window.grab();
    EXPECT_TRUE(settings::testing::isHidden(window, "shortcutdialog:recorder"));

    dialog.startRecording(std::nullopt);
    window.grab();
    EXPECT_FALSE(settings::testing::isHidden(window, "shortcutdialog:recorder"));
    // Escape alone stops recording and leaves the dialog open.
    TestInput::pressKey(window, {.key = key::Escape, .keycode = 1});
    EXPECT_FALSE(dialog.isRecording());
    EXPECT_TRUE(dialog.isOpen());

    dialog.startRecording(std::nullopt);
    TestInput::pressKey(window, {.key = 0xffe3, .keycode = 29});
    TestInput::pressKey(window, {.key = 'q', .keycode = 16, .text = "q", .modifiers = ControlModifier});
    // Sent as the Qt version sent it: the evdev code, Q upper-cased, Ctrl.
    ASSERT_TRUE(waitFor([&] {
        const std::vector<std::string> calls = fake.takeCalls();
        return std::find(calls.begin(), calls.end(), "add toggle-maximized 16 81 2") != calls.end();
    }));
    EXPECT_FALSE(dialog.isRecording());
    EXPECT_TRUE(dialog.message().empty());

    // A conflict that can be overridden is a warning, and recording goes on.
    {
        std::lock_guard guard(fake.mutex);
        fake.nextResult = "{\"result\":\"conflict-can-override\",\"conflictAction\":\"lock-screen\"}";
    }
    dialog.startRecording(std::nullopt);
    TestInput::pressKey(window, {.key = 'l', .keycode = 38, .text = "l", .modifiers = SuperModifier});
    EXPECT_TRUE(dialog.isRecording());
    EXPECT_NE(dialog.message().find("Lock the screen"), std::string::npos);
}
