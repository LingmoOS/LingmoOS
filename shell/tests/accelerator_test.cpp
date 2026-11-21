// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Keyboard shortcuts. Behaviour modelled on chromium/src/ash/accelerators
// (accelerator_controller_unittest.cc, ash_accelerator_configuration_unittest.cc,
// accelerator_launcher_state_machine_unittest.cc, accelerator_history_unittest.cc)
// and ash/wm/window_cycle/window_cycle_controller_unittest.cc, rewritten
// against the Lingmo abstractions. No DRM, no GL, no Wayland client.

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include "base/geometry.h"
#include "accelerators/accelerator.h"
#include "accelerators/accelerator_configuration.h"
#include "accelerators/accelerator_controller.h"
#include "accelerators/accelerator_delegate.h"
#include "accelerators/accelerator_filter.h"
#include "accelerators/accelerator_history.h"
#include "accelerators/accelerator_launcher_state_machine.h"
#include "accelerators/accelerator_manager.h"
#include "accelerators/accelerator_map.h"
#include "accelerators/accelerator_service.h"
#include "accelerators/accelerator_table.h"
#include "base/event_loop.h"
#include "base/json.h"
#include "base/settings.h"
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "core/main_loop.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/keyboard.h"
#include "input/keyboard_delegate.h"
#include "input/seat.h"
#include "input/xkb_tracker.h"
#include "input_test_util.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "scene/overview_node.h"
#include "scene/scene.h"
#include "session/secure_container.h"
#include "session/session_controller.h"
#include "session/session_teardown.h"
#include "shm_test_util.h"
#include "wm/activation.h"
#include "wm/desks/desks_controller.h"
#include "wm/mru_window_tracker.h"
#include "wm/overview/overview_controller.h"
#include "wm/show_desktop_controller.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/window_cycle_controller.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"

using namespace Lingmo;
using namespace Lingmo::Accelerators;
using Lingmo::base::Settings;

namespace {

input::Timestamp at(int ms)
{
    return input::Timestamp(std::chrono::milliseconds(ms));
}

const base::Rect kOutput(0, 0, 1000, 800);

class RecordingDelegate : public AcceleratorDelegate
{
public:
    void toggleAppList() override { ++appListToggles; }
    void toggleSearch() override { ++searchToggles; }
    void toggleScreenKeyboard() override { ++screenKeyboardToggles; }
    void takeScreenshot(bool toClipboard) override
    {
        captures.push_back(toClipboard ? "screen-clipboard" : "screen");
    }
    void takePartialScreenshot(bool toClipboard) override
    {
        captures.push_back(toClipboard ? "partial-clipboard" : "partial");
    }
    void startCaptureMode() override { captures.push_back("capture-mode"); }
    void stopScreenRecording() override { captures.push_back("stop-recording"); }
    void toggleClipboardHistory() override { ++clipboardHistoryToggles; }
    void volumeDown() override { levels.push_back("volume-down"); }
    void volumeUp() override { levels.push_back("volume-up"); }
    void toggleMute() override { levels.push_back("mute"); }
    void toggleMicrophoneMute() override { levels.push_back("microphone-mute"); }
    void brightnessDown() override { levels.push_back("brightness-down"); }
    void brightnessUp() override { levels.push_back("brightness-up"); }
    void keyboardBrightnessDown() override { levels.push_back("keyboard-brightness-down"); }
    void keyboardBrightnessUp() override { levels.push_back("keyboard-brightness-up"); }
    void mediaPlayPause() override { players.push_back("play-pause"); }
    void mediaNext() override { players.push_back("next"); }
    void mediaPrevious() override { players.push_back("previous"); }
    void mediaStop() override { players.push_back("stop"); }

    int appListToggles = 0;
    int searchToggles = 0;
    int screenKeyboardToggles = 0;
    int clipboardHistoryToggles = 0;
    std::vector<std::string> captures;
    std::vector<std::string> levels;
    std::vector<std::string> players;
};

// Records what reaches a client keyboard.
class RecordingKeyboardDelegate : public input::KeyboardDelegate
{
public:

    bool canAcceptKeyboardEventsForSurface(compositor::Surface *) const override { return true; }

    void onKeyboardEnter(compositor::Surface *, const PressedKeys &) override { }

    void onKeyboardLeave(compositor::Surface *) override { }

    uint32_t onKeyboardKey(input::Timestamp, uint32_t evdevCode, bool pressed) override
    {
        keys.push_back({evdevCode, pressed});
        return 0;
    }

    void onKeyboardModifiers(const input::KeyboardModifiers &) override { }

    void onKeyboardLayoutUpdated(const std::string &) override { }

    void onKeyRepeatSettingsChanged(bool, std::chrono::milliseconds,
                                    std::chrono::milliseconds) override
    {
    }

    std::vector<std::pair<uint32_t, bool>> keys;
};

class AcceleratorTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node *surfaceContainer =
            scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    scene::Node *secureNode =
            scene.root()->addChild(std::make_unique<scene::Node>("secure"));
    compositor::Compositor compositor {surfaceContainer};
    testsupport::FakeRenderer renderer;
    input::Seat seat {&compositor};
    input::InputManager input {&seat};
    wm::WindowManager manager {&compositor, &seat};
    session::SecureContainer secure {secureNode};
    session::SessionTeardown teardown {&seat, &manager};
    session::SessionController session {&secure, &teardown};

    input_test::TemporaryDirectory directory;
    std::unique_ptr<AcceleratorConfiguration> configuration;
    RecordingDelegate delegate;
    std::unique_ptr<wm::MruWindowTracker> mru;
    std::unique_ptr<wm::WindowCycleController> cycle;
    std::unique_ptr<AcceleratorController> controller;
    std::unique_ptr<AcceleratorFilter> filter;

    std::vector<std::unique_ptr<compositor::Surface>> surfaces;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 20);

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
        input.setBounds(kOutput.size());
        seat.setSessionGate(&session);
        manager.setSessionGate(&session);
        secure.setOutputs({{"out", kOutput}});
        ASSERT_TRUE(seat.xkbTracker()->updateKeyboardLayout("us"));

        configuration = std::make_unique<AcceleratorConfiguration>(
                directory.filePath("compositor.conf"));
        configuration->initialize();
        mru = std::make_unique<wm::MruWindowTracker>(&manager);
        cycle = std::make_unique<wm::WindowCycleController>(&manager, mru.get());
        teardown.setWindowCycle(cycle.get());
        CommandContext context;
        context.windowManager = &manager;
        context.windowCycle = cycle.get();
        context.session = &session;
        context.delegate = &delegate;
        controller = std::make_unique<AcceleratorController>(configuration.get(), context,
                                                             [this] {
                                                                 return !seat.pressedKeys()
                                                                                 .empty();
                                                             });
        filter = std::make_unique<AcceleratorFilter>(controller.get(), seat.xkbTracker(),
                                                     &manager, cycle.get(), &manager);
        input.setEventSink(filter.get());
    }

    void TearDown() override
    {
        input.setEventSink(nullptr);
        filter.reset();
        controller.reset();
        teardown.setWindowCycle(nullptr);
        cycle.reset();
        mru.reset();
        seat.shutdown();
    }

    compositor::Surface *makeSurface(const base::Size &size)
    {
        surfaces.push_back(compositor.createSurface());
        compositor::Surface *surface = surfaces.back().get();
        auto buffer = pool->createBuffer(size, 0, size.width() * 4,
                                         render::PixelFormat::Bgrx8888,
                                         nullptr);
        surface->attach(buffer, {});
        surface->damage(base::Rect(base::Point(), size));
        surface->commit();
        compositor.updateTextures(renderer);
        return surface;
    }

    wm::Window *makeWindow(const base::Rect &bounds)
    {
        wm::Window *window =
                manager.createWindow(makeSurface(bounds.size()), {.requestedSize = bounds.size()});
        window->setBounds(bounds);
        return window;
    }

    int time = 0;

    void press(uint32_t code) { input.onKey(at(++time), code, true); }

    void release(uint32_t code) { input.onKey(at(++time), code, false); }

    void tap(uint32_t code)
    {
        press(code);
        release(code);
    }

    void chord(std::initializer_list<uint32_t> modifiers, uint32_t key)
    {
        for (uint32_t modifier : modifiers)
            press(modifier);
        tap(key);
        for (auto it = std::rbegin(modifiers); it != std::rend(modifiers); ++it)
            release(*it);
    }
};

base::json::Value parse(const std::string &json)
{
    return base::json::Value::parse(json).value_or(base::json::Value());
}

} // namespace

////////////////////////////////////////////////////////////////////////////////
// Accelerator and AcceleratorMap

TEST(Accelerator, IdentityIgnoresTheEvdevCodeAndTextNamesTheKey)
{
    const Accelerator a(XKB_KEY_q, KEY_Q, ControlDown | ShiftDown, Accelerator::KeyState::Pressed);
    const Accelerator b(XKB_KEY_q, ControlDown | ShiftDown);
    EXPECT_EQ(a, b);
    EXPECT_EQ(a.hash(), b.hash());
    EXPECT_NE(a, Accelerator(XKB_KEY_q, ControlDown | ShiftDown, Accelerator::KeyState::Released));
    EXPECT_EQ("Ctrl+Shift+Q", a.shortcutText());
    EXPECT_EQ("Meta+Up", Accelerator(XKB_KEY_Up, MetaDown).shortcutText());
    EXPECT_EQ("Up", keysymName(XKB_KEY_Up));
    EXPECT_EQ(XKB_KEY_XF86AudioRaiseVolume, keysymFromName("XF86AudioRaiseVolume"));
    EXPECT_TRUE(keysymText(XKB_KEY_Tab).empty());
    EXPECT_EQ("[", keysymText(XKB_KEY_bracketleft));
    // Upper-cased as text is, beyond what keysyms have cases for.
    EXPECT_EQ("\u00c9", keysymText(XKB_KEY_eacute));
    EXPECT_EQ("SS", keysymText(XKB_KEY_ssharp));
}

TEST(AcceleratorMap, PositionalLookupPinsPunctuationToTheUsLayout)
{
    AcceleratorMap<int> map;
    map.insertNew(Accelerator(XKB_KEY_bracketright, AltDown), 1);
    map.insertNew(Accelerator(XKB_KEY_semicolon, AltDown), 2);

    // A German layout has "+" where the US layout has "]".
    const Accelerator germanPlus(XKB_KEY_plus, KEY_RIGHTBRACE, AltDown,
                                 Accelerator::KeyState::Pressed);
    EXPECT_EQ(nullptr, map.find(germanPlus));
    map.setUsePositionalLookup(true);
    ASSERT_NE(nullptr, map.find(germanPlus));
    EXPECT_EQ(1, *map.find(germanPlus));

    // A French layout has "m" where the US layout has ";": letters keep their
    // meaning and are never remapped by position.
    const Accelerator frenchM(XKB_KEY_m, KEY_SEMICOLON, AltDown, Accelerator::KeyState::Pressed);
    EXPECT_EQ(nullptr, map.find(frenchM));
    EXPECT_EQ(0u, positionalUsKeysym(KEY_A, XKB_KEY_a));
}

TEST(AcceleratorMap, EraseRemovesWhatFindReturns)
{
    AcceleratorMap<int> map;
    map.insertNew(Accelerator(XKB_KEY_bracketright, AltDown), 1);
    map.insertNew(Accelerator(XKB_KEY_plus, AltDown), 2);
    map.setUsePositionalLookup(true);

    const Accelerator germanPlus(XKB_KEY_plus, KEY_RIGHTBRACE, AltDown,
                                 Accelerator::KeyState::Pressed);
    EXPECT_TRUE(map.erase(germanPlus));
    EXPECT_EQ(nullptr, map.find(germanPlus));
    ASSERT_NE(nullptr, map.find(Accelerator(XKB_KEY_plus, AltDown)));
    EXPECT_EQ(2, *map.find(Accelerator(XKB_KEY_plus, AltDown)));
}

////////////////////////////////////////////////////////////////////////////////
// AcceleratorManager

namespace {

class CountingTarget : public AcceleratorTarget
{
public:
    explicit CountingTarget(bool handles)
        : handles(handles)
    {
    }

    bool acceleratorPressed(const Accelerator &) override
    {
        ++presses;
        return handles;
    }

    bool canHandleAccelerators() const override { return true; }

    bool handles;
    int presses = 0;
};

} // namespace

TEST(AcceleratorManager, LaterTargetsWinAndPriorityHandlersGoFirst)
{
    AcceleratorManager manager;
    const Accelerator accelerator(XKB_KEY_a, ControlDown);
    CountingTarget first(true), second(false), priority(true);
    manager.registerAccelerators({accelerator}, AcceleratorManager::NormalPriority, &first);
    manager.registerAccelerators({accelerator}, AcceleratorManager::NormalPriority, &second);
    EXPECT_TRUE(manager.process(accelerator));
    EXPECT_EQ(1, second.presses);
    EXPECT_EQ(1, first.presses);

    manager.registerAccelerators({accelerator}, AcceleratorManager::HighPriority, &priority);
    EXPECT_TRUE(manager.hasPriorityHandler(accelerator));
    EXPECT_TRUE(manager.process(accelerator));
    EXPECT_EQ(1, priority.presses);
    EXPECT_EQ(1, second.presses);

    manager.unregisterAll(&priority);
    manager.unregisterAccelerator(accelerator, &second);
    manager.unregisterAccelerator(accelerator, &first);
    EXPECT_FALSE(manager.isRegistered(accelerator));
    EXPECT_FALSE(manager.process(accelerator));
}

TEST(AcceleratorManager, UnregisteringByPositionRemovesTheUsKey)
{
    AcceleratorManager manager;
    manager.setUsePositionalLookup(true);
    const Accelerator usBracket(XKB_KEY_bracketright, AltDown);
    const Accelerator germanPlus(XKB_KEY_plus, KEY_RIGHTBRACE, AltDown,
                                 Accelerator::KeyState::Pressed);
    const Accelerator plainPlus(XKB_KEY_plus, AltDown);
    CountingTarget target(true), other(true);
    manager.registerAccelerators({usBracket}, AcceleratorManager::NormalPriority, &target);
    manager.registerAccelerators({plainPlus}, AcceleratorManager::NormalPriority, &other);
    ASSERT_TRUE(manager.isRegistered(germanPlus));

    manager.unregisterAccelerator(germanPlus, &target);
    EXPECT_FALSE(manager.isRegistered(usBracket));
    EXPECT_FALSE(manager.process(germanPlus));
    EXPECT_TRUE(manager.isRegistered(plainPlus));
    EXPECT_TRUE(manager.process(plainPlus));
    EXPECT_EQ(1, other.presses);
}

////////////////////////////////////////////////////////////////////////////////
// AcceleratorConfiguration

TEST(AcceleratorConfiguration, AddRemoveReplaceAndRestore)
{
    input_test::TemporaryDirectory directory;
    AcceleratorConfiguration configuration(directory.filePath("compositor.conf"));
    configuration.initialize();
    int updates = 0;
    auto counted = configuration.acceleratorsUpdated.connect([&updates] { ++updates; });

    const auto maximize = toActionId(AcceleratorAction::ToggleMaximized);
    const auto minimize = toActionId(AcceleratorAction::WindowMinimize);
    const Accelerator metaUp(XKB_KEY_Up, MetaDown);
    const Accelerator metaDown(XKB_KEY_Down, MetaDown);
    const Accelerator custom(XKB_KEY_k, MetaDown | ControlDown);

    ASSERT_TRUE(configuration.findAcceleratorAction(metaUp));
    EXPECT_EQ(AcceleratorAction::ToggleMaximized, *configuration.findAcceleratorAction(metaUp));
    EXPECT_FALSE(configuration.hasCustomAccelerators());

    EXPECT_EQ(AcceleratorConfigResult::Success, configuration.addUserAccelerator(maximize, custom));
    EXPECT_EQ(AcceleratorAction::ToggleMaximized, *configuration.findAcceleratorAction(custom));
    EXPECT_TRUE(configuration.hasCustomAccelerators());
    EXPECT_EQ(1, updates);

    // Giving another action's default to this one takes it away from there.
    EXPECT_EQ(AcceleratorConfigResult::Success,
              configuration.addUserAccelerator(maximize, metaDown));
    EXPECT_EQ(AcceleratorAction::ToggleMaximized, *configuration.findAcceleratorAction(metaDown));
    EXPECT_EQ(1u, configuration.acceleratorsForAction(minimize)->size());

    EXPECT_EQ(AcceleratorConfigResult::Success,
              configuration.replaceAccelerator(maximize, custom,
                                               Accelerator(XKB_KEY_j, MetaDown | ControlDown)));
    EXPECT_FALSE(configuration.findAcceleratorAction(custom));
    EXPECT_EQ(AcceleratorConfigResult::NotFound,
              configuration.replaceAccelerator(minimize, custom, metaUp));

    EXPECT_EQ(AcceleratorConfigResult::Success, configuration.removeAccelerator(maximize, metaUp));
    EXPECT_FALSE(configuration.findAcceleratorAction(metaUp));
    EXPECT_EQ(AcceleratorConfigResult::NotFound,
              configuration.removeAccelerator(maximize, metaUp));

    // Meta+Down is still held by maximize, so minimize's restore reports it.
    EXPECT_EQ(AcceleratorConfigResult::RestoreSuccessWithConflicts,
              configuration.restoreDefault(minimize));
    // Restoring maximize gives the taken default back to minimize, which is
    // what the override file, with no entry left for either, now describes.
    EXPECT_EQ(AcceleratorConfigResult::Success, configuration.restoreDefault(maximize));
    EXPECT_EQ(AcceleratorAction::ToggleMaximized, *configuration.findAcceleratorAction(metaUp));
    EXPECT_EQ(AcceleratorAction::WindowMinimize, *configuration.findAcceleratorAction(metaDown));
    EXPECT_FALSE(configuration.hasCustomAccelerators());

    // Removing a taken default from its new owner does the same, unless the
    // default owner had itself disabled it.
    EXPECT_EQ(AcceleratorConfigResult::Success,
              configuration.addUserAccelerator(maximize, metaDown));
    EXPECT_EQ(AcceleratorConfigResult::Success,
              configuration.removeAccelerator(maximize, metaDown));
    EXPECT_EQ(AcceleratorAction::WindowMinimize, *configuration.findAcceleratorAction(metaDown));
    EXPECT_FALSE(configuration.hasCustomAccelerators());
    EXPECT_EQ(AcceleratorConfigResult::Success,
              configuration.removeAccelerator(minimize, metaDown));
    EXPECT_EQ(AcceleratorConfigResult::Success,
              configuration.addUserAccelerator(maximize, metaDown));
    EXPECT_EQ(AcceleratorConfigResult::Success,
              configuration.removeAccelerator(maximize, metaDown));
    EXPECT_FALSE(configuration.findAcceleratorAction(metaDown));
    EXPECT_EQ(AcceleratorConfigResult::Success, configuration.restoreAllDefaults());
    EXPECT_EQ(AcceleratorAction::WindowMinimize, *configuration.findAcceleratorAction(metaDown));
    EXPECT_FALSE(configuration.hasCustomAccelerators());
    EXPECT_EQ(configuration.defaultAcceleratorsForId(maximize),
              *configuration.acceleratorsForAction(maximize));
}

TEST(AcceleratorConfiguration, RemovingByPositionRemovesTheUsKey)
{
    input_test::TemporaryDirectory directory;
    AcceleratorConfiguration configuration(directory.filePath("compositor.conf"));
    configuration.initialize();
    configuration.setUsePositionalLookup(true);

    const auto snapRight = toActionId(AcceleratorAction::WindowCycleSnapRight);
    const Accelerator usBracket(XKB_KEY_bracketright, AltDown);
    const Accelerator germanPlus(XKB_KEY_plus, KEY_RIGHTBRACE, AltDown,
                                 Accelerator::KeyState::Pressed);
    ASSERT_TRUE(configuration.findAcceleratorAction(germanPlus));
    const size_t accelerators = configuration.acceleratorsForAction(snapRight)->size();

    EXPECT_EQ(AcceleratorConfigResult::Success,
              configuration.removeAccelerator(snapRight, germanPlus));
    EXPECT_FALSE(configuration.findAcceleratorAction(usBracket));
    EXPECT_EQ(accelerators - 1, configuration.acceleratorsForAction(snapRight)->size());

    AcceleratorConfiguration reloaded(directory.filePath("compositor.conf"));
    reloaded.initialize();
    EXPECT_FALSE(reloaded.findAcceleratorAction(usBracket));
}

TEST(AcceleratorConfiguration, OverridesPersistAcrossInstances)
{
    input_test::TemporaryDirectory directory;
    const std::string file = directory.filePath("compositor.conf");
    const auto lock = toActionId(AcceleratorAction::LockScreen);
    const auto maximize = toActionId(AcceleratorAction::ToggleMaximized);
    const Accelerator metaL(XKB_KEY_l, MetaDown);
    const Accelerator custom(XKB_KEY_Escape, MetaDown | ShiftDown);
    {
        AcceleratorConfiguration configuration(file);
        configuration.initialize();
        EXPECT_EQ(AcceleratorConfigResult::Success,
                  configuration.removeAccelerator(lock, metaL));
        EXPECT_EQ(AcceleratorConfigResult::Success,
                  configuration.addUserAccelerator(maximize, custom));
        // Stealing a default records it under the new owner only.
        EXPECT_EQ(AcceleratorConfigResult::Success,
                  configuration.addUserAccelerator(maximize, Accelerator(XKB_KEY_Down, MetaDown)));
    }
    {
        AcceleratorConfiguration configuration(file);
        configuration.initialize();
        EXPECT_FALSE(configuration.findAcceleratorAction(metaL));
        ASSERT_TRUE(configuration.findAcceleratorAction(custom));
        EXPECT_EQ(AcceleratorAction::ToggleMaximized,
                  *configuration.findAcceleratorAction(custom));
        EXPECT_EQ(AcceleratorAction::ToggleMaximized,
                  *configuration.findAcceleratorAction(Accelerator(XKB_KEY_Down, MetaDown)));
        EXPECT_EQ(1u, configuration.acceleratorsForAction(
                              toActionId(AcceleratorAction::WindowMinimize))->size());
        // Re-adding a removed default cancels the removal instead of stacking.
        EXPECT_EQ(AcceleratorConfigResult::Success,
                  configuration.addUserAccelerator(lock, metaL));
        configuration.restoreAllDefaults();
    }
    AcceleratorConfiguration configuration(file);
    configuration.initialize();
    EXPECT_FALSE(configuration.hasCustomAccelerators());
    EXPECT_EQ(AcceleratorAction::LockScreen, *configuration.findAcceleratorAction(metaL));
}

TEST(AcceleratorConfiguration, EachConventionKeepsItsOwnOverrides)
{
    input_test::TemporaryDirectory directory;
    const std::string file = directory.filePath("compositor.conf");
    const auto maximize = toActionId(AcceleratorAction::ToggleMaximized);
    const auto launcher = toActionId(AcceleratorAction::ToggleAppList);
    const Accelerator pcBinding(XKB_KEY_j, MetaDown | ControlDown);
    const Accelerator commandBinding(XKB_KEY_space, MetaDown | ShiftDown);

    AcceleratorConfiguration configuration(file);
    configuration.initialize();
    ASSERT_EQ(AcceleratorConfigResult::Success,
              configuration.addUserAccelerator(maximize, pcBinding));

    // Meta is another key under Command, so the PC binding waits and the
    // Command convention takes edits of its own.
    configuration.setConvention(input::ShortcutConvention::Command);
    EXPECT_FALSE(configuration.findAcceleratorAction(pcBinding));
    EXPECT_FALSE(configuration.hasCustomAccelerators());
    ASSERT_EQ(AcceleratorConfigResult::Success,
              configuration.addUserAccelerator(launcher, commandBinding));
    EXPECT_EQ(AcceleratorAction::ToggleAppList,
              *configuration.findAcceleratorAction(commandBinding));

    configuration.setConvention(input::ShortcutConvention::Pc);
    ASSERT_TRUE(configuration.findAcceleratorAction(pcBinding));
    EXPECT_EQ(AcceleratorAction::ToggleMaximized, *configuration.findAcceleratorAction(pcBinding));
    EXPECT_FALSE(configuration.findAcceleratorAction(commandBinding));

    AcceleratorConfiguration reread(file);
    reread.setConvention(input::ShortcutConvention::Command);
    ASSERT_TRUE(reread.findAcceleratorAction(commandBinding));
    EXPECT_EQ(AcceleratorAction::ToggleAppList, *reread.findAcceleratorAction(commandBinding));
    EXPECT_FALSE(reread.findAcceleratorAction(pcBinding));
}

TEST(AcceleratorConfiguration, MalformedOverridesAreDropped)
{
    input_test::TemporaryDirectory directory;
    const std::string file = directory.filePath("compositor.conf");
    {
        Settings settings(file);
        settings.setValue("Shortcuts/no-such-action", "[]");
        settings.setValue("Shortcuts/lock-screen", "not json");
        settings.setValue("Shortcuts/toggle-maximized",
                          "[{\"key\":\"NoSuchKey\",\"action\":\"add\"},"
                          "{\"key\":\"Home\",\"modifiers\":[\"Meta\"],"
                          "\"state\":\"pressed\",\"action\":\"add\"}]");
    }
    AcceleratorConfiguration configuration(file);
    configuration.initialize();
    ASSERT_TRUE(configuration.findAcceleratorAction(Accelerator(XKB_KEY_Home, MetaDown)));
    EXPECT_EQ(AcceleratorAction::LockScreen,
              *configuration.findAcceleratorAction(Accelerator(XKB_KEY_l, MetaDown)));
    Settings settings(file);
    EXPECT_EQ(std::vector<std::string> {"toggle-maximized"}, settings.childKeys("Shortcuts"));
    // The file is written as it always was: members sorted, compact, quoted.
    EXPECT_EQ(settings.rawValue("Shortcuts/toggle-maximized"),
              R"("[{\"action\":\"add\",\"key\":\"Home\",\"modifiers\":[\"Meta\"],\"state\":\"pressed\"}]")");
}

////////////////////////////////////////////////////////////////////////////////
// AcceleratorHistory and the launcher state machine

TEST(AcceleratorHistory, TracksCurrentAndPreviousWithoutDoubleCountingHeldKeys)
{
    AcceleratorHistory history;
    const Accelerator altDown(XKB_KEY_Alt_L, AltDown);
    const Accelerator metaDown(XKB_KEY_Super_L, AltDown | MetaDown);
    history.storeCurrentAccelerator(altDown);
    history.storeCurrentAccelerator(metaDown);
    EXPECT_EQ(metaDown, history.currentAccelerator());
    EXPECT_EQ(altDown, history.previousAccelerator());
    // Alt goes up while Meta stays down: Meta is not stored as pressed again.
    history.storeCurrentAccelerator(Accelerator(XKB_KEY_Alt_L, MetaDown, Accelerator::KeyState::Released));
    history.storeCurrentAccelerator(Accelerator(XKB_KEY_Super_L, MetaDown));
    EXPECT_EQ(XKB_KEY_Alt_L, history.currentAccelerator().keysym());
    EXPECT_EQ(1u, history.currentlyPressedKeys().size());
}

TEST(AcceleratorLauncherStateMachine, OnlyALoneMetaTapTriggers)
{
    int pressed = 0;
    AcceleratorLauncherStateMachine machine([&pressed] {
        return pressed > 0;
    });
    using State = AcceleratorLauncherStateMachine::LauncherState;

    ++pressed;
    machine.onKey(XKB_KEY_Super_L, true);
    EXPECT_EQ(State::Primed, machine.currentState());
    --pressed;
    machine.onKey(XKB_KEY_Super_L, false);
    EXPECT_EQ(State::Trigger, machine.currentState());
    EXPECT_TRUE(machine.canHandleLauncher());

    // Meta+L is not a launcher tap.
    ++pressed;
    machine.onKey(XKB_KEY_Super_L, true);
    ++pressed;
    machine.onKey(XKB_KEY_l, true);
    EXPECT_EQ(State::Suppress, machine.currentState());
    --pressed;
    machine.onKey(XKB_KEY_l, false);
    EXPECT_EQ(State::Suppress, machine.currentState());
    --pressed;
    machine.onKey(XKB_KEY_Super_L, false);
    EXPECT_EQ(State::Start, machine.currentState());
    EXPECT_FALSE(machine.canHandleLauncher());

    // A click while Meta is held suppresses the release.
    ++pressed;
    machine.onKey(XKB_KEY_Super_L, true);
    machine.onPointerButton(true);
    machine.onPointerButton(false);
    --pressed;
    machine.onKey(XKB_KEY_Super_L, false);
    EXPECT_FALSE(machine.canHandleLauncher());
}

////////////////////////////////////////////////////////////////////////////////
// Controller and filter, end to end from physical keys

TEST_F(AcceleratorTest, ALoneMetaOpensTheLauncherAndMetaChordsDoNot)
{
    wm::Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    manager.activation()->activate(window);
    auto delegateOwner = std::make_unique<RecordingKeyboardDelegate>();
    RecordingKeyboardDelegate *keyboardDelegate = delegateOwner.get();
    input::Keyboard keyboard(std::move(delegateOwner), &seat);

    // Ash's default: the launcher on Meta's release, the key still reaching
    // the window.
    tap(KEY_LEFTMETA);
    EXPECT_EQ(1, delegate.appListToggles);
    const std::vector<std::pair<uint32_t, bool>> expected = {{KEY_LEFTMETA, true},
                                                             {KEY_LEFTMETA, false}};
    EXPECT_EQ(expected, keyboardDelegate->keys);

    ASSERT_EQ(AcceleratorConfigResult::Success,
              configuration->addUserAccelerator(toActionId(AcceleratorAction::ToggleAppList),
                                                Accelerator(XKB_KEY_space, MetaDown | ShiftDown)));
    chord({KEY_LEFTMETA, KEY_LEFTSHIFT}, KEY_SPACE);
    EXPECT_EQ(2, delegate.appListToggles);

    chord({KEY_LEFTMETA}, KEY_L);
    EXPECT_EQ(2, delegate.appListToggles);
    // Meta+L locks; the session is Locking until the lock screen is presented.
    EXPECT_TRUE(session.isBlocked());
}

TEST_F(AcceleratorTest, WindowActionsApplyToTheActiveWindow)
{
    wm::Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    manager.activation()->activate(window);

    chord({KEY_LEFTMETA}, KEY_UP);
    EXPECT_EQ(wm::WindowStateType::Maximized, window->stateType());
    chord({KEY_LEFTMETA}, KEY_UP);
    EXPECT_EQ(wm::WindowStateType::Normal, window->stateType());

    chord({KEY_LEFTALT}, KEY_LEFTBRACE);
    EXPECT_EQ(wm::WindowStateType::PrimarySnapped, window->stateType());
    chord({KEY_LEFTMETA}, KEY_RIGHT);
    EXPECT_EQ(wm::WindowStateType::SecondarySnapped, window->stateType());
    // Snapping to the side the window is already on restores it.
    chord({KEY_LEFTMETA}, KEY_RIGHT);
    EXPECT_EQ(wm::WindowStateType::Normal, window->stateType());

    chord({KEY_LEFTMETA}, KEY_F);
    EXPECT_EQ(wm::WindowStateType::Fullscreen, window->stateType());
    chord({KEY_LEFTMETA}, KEY_F);
    EXPECT_EQ(wm::WindowStateType::Normal, window->stateType());

    chord({KEY_LEFTALT}, KEY_MINUS);
    EXPECT_EQ(wm::WindowStateType::Minimized, window->stateType());

    int closes = 0;
    window->restore();
    manager.activation()->activate(window);
    window->setCloseCallback([&closes] {
        ++closes;
    });
    chord({KEY_LEFTALT}, KEY_F4);
    EXPECT_EQ(1, closes);
}

TEST_F(AcceleratorTest, ActionsRunWithoutAKeyUnderTheSameRestrictions)
{
    // A hot corner runs an action with no key behind it.
    makeWindow(base::Rect(100, 100, 300, 200));
    wm::ShowDesktopController *showDesktop = manager.showDesktop();
    EXPECT_TRUE(controller->performActionWithoutKey(AcceleratorAction::ToggleShowDesktop));
    EXPECT_TRUE(showDesktop->isActive());
    EXPECT_TRUE(controller->performActionWithoutKey(AcceleratorAction::ToggleShowDesktop));
    EXPECT_FALSE(showDesktop->isActive());

    ASSERT_TRUE(session.requestLock());
    session.notifyFramePresented();
    EXPECT_FALSE(controller->performActionWithoutKey(AcceleratorAction::ToggleShowDesktop));
    EXPECT_FALSE(showDesktop->isActive());
}

TEST_F(AcceleratorTest, WindowActionsLeaveTheShellsOwnWindowsAlone)
{
    // The desktop takes the keyboard once every application window is
    // minimized; minimizing or closing it would leave no wallpaper.
    wm::Window *application = makeWindow(base::Rect(100, 100, 300, 200));
    application->minimize();
    wm::Window *desktop = makeWindow(base::Rect(0, 0, 800, 600));
    desktop->setRole(wm::WindowRole::Desktop);
    int closes = 0;
    desktop->setCloseCallback([&closes] {
        ++closes;
    });
    manager.activation()->activate(desktop);
    ASSERT_EQ(desktop, manager.activation()->activeWindow());

    chord({KEY_LEFTALT}, KEY_MINUS);
    chord({KEY_LEFTMETA}, KEY_DOWN);
    chord({KEY_LEFTMETA}, KEY_UP);
    chord({KEY_LEFTMETA}, KEY_F);
    chord({KEY_LEFTMETA}, KEY_LEFT);
    chord({KEY_LEFTALT}, KEY_F4);
    EXPECT_EQ(wm::WindowStateType::Normal, desktop->stateType());
    EXPECT_TRUE(desktop->isVisible());
    EXPECT_EQ(0, closes);

    seat.setShortcutConvention(input::ShortcutConvention::Command);
    configuration->setConvention(input::ShortcutConvention::Command);
    // Command+M, with Alt where Command is on a PC keyboard.
    chord({KEY_LEFTALT}, KEY_M);
    chord({KEY_LEFTCTRL, KEY_LEFTALT}, KEY_F);
    EXPECT_EQ(wm::WindowStateType::Normal, desktop->stateType());
    EXPECT_TRUE(desktop->isVisible());
}

TEST_F(AcceleratorTest, TouchReachesTheWindowManagerBehindTheFilter)
{
    const auto normalized = [](const base::PointF &location) {
        return base::PointF(location.x() / kOutput.width(), location.y() / kOutput.height());
    };
    wm::Window *lower = makeWindow(base::Rect(100, 100, 300, 200));
    wm::Window *upper = makeWindow(base::Rect(500, 100, 300, 200));
    manager.activation()->activate(upper);

    input.onTouchDown(at(++time), 1, normalized(base::PointF(150, 150)));
    input.onTouchFrame(at(time));
    EXPECT_TRUE(lower->isActive());

    ASSERT_TRUE(manager.eventHandler()->beginMove(lower, base::PointF(150, 150), 1));
    input.onTouchMotion(at(++time), 1, normalized(base::PointF(170, 160)));
    input.onTouchFrame(at(time));
    EXPECT_EQ(base::Rect(120, 110, 300, 200), lower->bounds());
    input.onTouchUp(at(++time), 1);
    input.onTouchFrame(at(time));
    EXPECT_FALSE(manager.eventHandler()->isDragging());
}

TEST_F(AcceleratorTest, ConsumedKeysNeverReachTheClientAndOthersDo)
{
    wm::Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    manager.activation()->activate(window);
    auto delegateOwner = std::make_unique<RecordingKeyboardDelegate>();
    RecordingKeyboardDelegate *keyboardDelegate = delegateOwner.get();
    input::Keyboard keyboard(std::move(delegateOwner), &seat);

    chord({KEY_LEFTMETA}, KEY_UP);
    // Meta itself reaches the client; Up, the accelerator key, does not, and
    // neither does its release.
    std::vector<std::pair<uint32_t, bool>> expected = {{KEY_LEFTMETA, true}, {KEY_LEFTMETA, false}};
    EXPECT_EQ(expected, keyboardDelegate->keys);

    keyboardDelegate->keys.clear();
    chord({KEY_LEFTCTRL}, KEY_C);
    expected = {{KEY_LEFTCTRL, true}, {KEY_C, true}, {KEY_C, false}, {KEY_LEFTCTRL, false}};
    EXPECT_EQ(expected, keyboardDelegate->keys);

    // Device keys are the system's, with or without an action behind them.
    keyboardDelegate->keys.clear();
    tap(KEY_VOLUMEUP);
    chord({KEY_LEFTSHIFT}, KEY_BRIGHTNESSDOWN);
    tap(KEY_MUTE);
    tap(KEY_MICMUTE);
    tap(KEY_KBDILLUMUP);
    expected = {{KEY_LEFTSHIFT, true}, {KEY_LEFTSHIFT, false}};
    EXPECT_EQ(expected, keyboardDelegate->keys);
    const std::vector<std::string> levels = {"volume-up", "mute", "microphone-mute",
                                             "keyboard-brightness-up"};
    EXPECT_EQ(levels, delegate.levels);

    // Media keys have no action, so they belong to the client.
    keyboardDelegate->keys.clear();
    tap(KEY_PLAYPAUSE);
    EXPECT_EQ(2u, keyboardDelegate->keys.size());
}

TEST_F(AcceleratorTest, HeldVolumeAndBrightnessKeysRepeatAtTheKeyboardRate)
{
    filter->setKeyRepeatSettings({.enabled = true,
                                  .delay = std::chrono::milliseconds(40),
                                  .interval = std::chrono::milliseconds(10)});

    // Repeats are timers: a loaded machine fires them late, not fewer.
    press(KEY_BRIGHTNESSUP);
    EXPECT_EQ(1u, delegate.levels.size());
    EXPECT_TRUE(Lingmo::test::runUntil([this] { return delegate.levels.size() >= 3; }));
    release(KEY_BRIGHTNESSUP);
    const size_t held = delegate.levels.size();
    EXPECT_GE(held, 3u);
    EXPECT_EQ(std::vector<std::string>(held, "brightness-up"), delegate.levels);
    Lingmo::test::runFor(std::chrono::milliseconds(50));
    EXPECT_EQ(held, delegate.levels.size());

    // Another key going down ends it, as it does for a client.
    delegate.levels.clear();
    press(KEY_VOLUMEDOWN);
    tap(KEY_A);
    Lingmo::test::runFor(std::chrono::milliseconds(100));
    release(KEY_VOLUMEDOWN);
    EXPECT_EQ(1u, delegate.levels.size());

    // Mute toggles once however long it is held.
    delegate.levels.clear();
    press(KEY_MUTE);
    Lingmo::test::runFor(std::chrono::milliseconds(100));
    release(KEY_MUTE);
    EXPECT_EQ(1u, delegate.levels.size());

    delegate.levels.clear();
    press(KEY_MICMUTE);
    Lingmo::test::runFor(std::chrono::milliseconds(100));
    release(KEY_MICMUTE);
    EXPECT_EQ(1u, delegate.levels.size());

    delegate.levels.clear();
    press(KEY_KBDILLUMDOWN);
    EXPECT_TRUE(Lingmo::test::runUntil([this] { return delegate.levels.size() >= 3; }));
    release(KEY_KBDILLUMDOWN);
    EXPECT_GE(delegate.levels.size(), 3u);

    filter->setKeyRepeatSettings({.enabled = false});
    delegate.levels.clear();
    press(KEY_VOLUMEUP);
    Lingmo::test::runFor(std::chrono::milliseconds(100));
    release(KEY_VOLUMEUP);
    EXPECT_EQ(1u, delegate.levels.size());
}

TEST_F(AcceleratorTest, AltTabSwitchesApplicationsInMruOrderOnReleaseAndEscapeCancels)
{
    // Windows without an application id or a process are applications of
    // their own.
    wm::Window *first = makeWindow(base::Rect(0, 0, 300, 200));
    wm::Window *second = makeWindow(base::Rect(50, 50, 300, 200));
    wm::Window *third = makeWindow(base::Rect(100, 100, 300, 200));
    manager.activation()->activate(first);
    manager.activation()->activate(second);
    manager.activation()->activate(third);

    // Stepping only moves the selection; the previous application is first.
    press(KEY_LEFTALT);
    tap(KEY_TAB);
    EXPECT_TRUE(cycle->isCycling());
    const std::vector<wm::Window *> order = {third, second, first};
    EXPECT_EQ(order, cycle->entries());
    EXPECT_EQ(1, cycle->selectedIndex());
    EXPECT_EQ(third, manager.activation()->activeWindow());
    tap(KEY_TAB);
    EXPECT_EQ(2, cycle->selectedIndex());
    // Shift+Alt+Tab, the arrow keys and ` step too.
    press(KEY_LEFTSHIFT);
    tap(KEY_TAB);
    release(KEY_LEFTSHIFT);
    EXPECT_EQ(1, cycle->selectedIndex());
    tap(KEY_RIGHT);
    EXPECT_EQ(2, cycle->selectedIndex());
    tap(KEY_GRAVE);
    tap(KEY_LEFT);
    EXPECT_EQ(0, cycle->selectedIndex());
    tap(KEY_RIGHT);
    EXPECT_EQ(third, manager.activation()->activeWindow());
    release(KEY_LEFTALT);
    EXPECT_FALSE(cycle->isCycling());
    EXPECT_EQ(second, manager.activation()->activeWindow());

    // The MRU order now starts with the application just chosen.
    const auto list = mru->buildMruWindowList();
    ASSERT_EQ(3u, list.size());
    EXPECT_EQ(second, list[0]);
    EXPECT_EQ(third, list[1]);

    press(KEY_LEFTALT);
    tap(KEY_TAB);
    tap(KEY_ESC);
    EXPECT_FALSE(cycle->isCycling());
    EXPECT_EQ(second, manager.activation()->activeWindow());
    release(KEY_LEFTALT);

    // An application whose windows are all minimized is still switched to,
    // and its window comes back once the keys are released.
    third->minimize();
    press(KEY_LEFTALT);
    tap(KEY_TAB);
    EXPECT_TRUE(third->state()->isMinimized());
    release(KEY_LEFTALT);
    EXPECT_FALSE(third->state()->isMinimized());
    EXPECT_EQ(third, manager.activation()->activeWindow());
}

TEST_F(AcceleratorTest, ThePcConventionTakesEveryKeyboardAsItIs)
{
    wm::Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    manager.activation()->activate(window);
    auto delegateOwner = std::make_unique<RecordingKeyboardDelegate>();
    RecordingKeyboardDelegate *keyboardDelegate = delegateOwner.get();
    input::Keyboard keyboard(std::move(delegateOwner), &seat);

    // An Apple keyboard's Command is Meta, as on any Linux desktop: a lone tap
    // opens the launcher and the client sees Meta.
    input.onKey(at(++time), KEY_LEFTMETA, true, input::KeyboardModifierLayout::Apple);
    input.onKey(at(++time), KEY_LEFTMETA, false, input::KeyboardModifierLayout::Apple);
    EXPECT_EQ(1, delegate.appListToggles);
    input.onKey(at(++time), KEY_LEFTALT, true, input::KeyboardModifierLayout::Apple);
    input.onKey(at(++time), KEY_C, true, input::KeyboardModifierLayout::Apple);
    input.onKey(at(++time), KEY_C, false, input::KeyboardModifierLayout::Apple);
    input.onKey(at(++time), KEY_LEFTALT, false, input::KeyboardModifierLayout::Apple);
    const std::vector<std::pair<uint32_t, bool>> expected = {
            {KEY_LEFTMETA, true}, {KEY_LEFTMETA, false}, {KEY_LEFTALT, true},
            {KEY_C, true},        {KEY_C, false},        {KEY_LEFTALT, false}};
    EXPECT_EQ(expected, keyboardDelegate->keys);
}

TEST_F(AcceleratorTest, UnderTheCommandConventionTheKeyNextToTheSpaceBarSwitchesWindows)
{
    seat.setShortcutConvention(input::ShortcutConvention::Command);
    configuration->setConvention(input::ShortcutConvention::Command);
    wm::Window *first = makeWindow(base::Rect(0, 0, 300, 200));
    wm::Window *second = makeWindow(base::Rect(50, 50, 300, 200));
    manager.activation()->activate(first);
    manager.activation()->activate(second);
    auto delegateOwner = std::make_unique<RecordingKeyboardDelegate>();
    RecordingKeyboardDelegate *keyboardDelegate = delegateOwner.get();
    input::Keyboard keyboard(std::move(delegateOwner), &seat);

    // Alt on a PC keyboard is Command here.
    press(KEY_LEFTALT);
    tap(KEY_TAB);
    EXPECT_TRUE(cycle->isCycling());
    release(KEY_LEFTALT);
    EXPECT_FALSE(cycle->isCycling());
    EXPECT_EQ(first, manager.activation()->activeWindow());
    // Command+Tab never reached the window, not even as a Ctrl.
    EXPECT_TRUE(keyboardDelegate->keys.empty());

    // So is Command on an Apple keyboard.
    input.onKey(at(++time), KEY_LEFTMETA, true, input::KeyboardModifierLayout::Apple);
    tap(KEY_TAB);
    EXPECT_TRUE(cycle->isCycling());
    input.onKey(at(++time), KEY_LEFTMETA, false, input::KeyboardModifierLayout::Apple);
    EXPECT_FALSE(cycle->isCycling());
    EXPECT_EQ(second, manager.activation()->activeWindow());
}

TEST_F(AcceleratorTest, ACycleTheLockInterruptedIsOverAfterUnlock)
{
    seat.setShortcutConvention(input::ShortcutConvention::Command);
    configuration->setConvention(input::ShortcutConvention::Command);
    wm::Window *first = makeWindow(base::Rect(0, 0, 300, 200));
    wm::Window *second = makeWindow(base::Rect(50, 50, 300, 200));
    manager.activation()->activate(first);
    manager.activation()->activate(second);
    auto delegateOwner = std::make_unique<RecordingKeyboardDelegate>();
    RecordingKeyboardDelegate *keyboardDelegate = delegateOwner.get();
    input::Keyboard keyboard(std::move(delegateOwner), &seat);
    press(KEY_LEFTALT);
    tap(KEY_TAB);
    ASSERT_TRUE(cycle->isCycling());

    ASSERT_TRUE(session.requestLock());
    session.notifyFramePresented();
    release(KEY_LEFTALT);
    ASSERT_TRUE(session.requestUnlock());
    session.notifyFramePresented();

    EXPECT_FALSE(cycle->isCycling());
    keyboardDelegate->keys.clear();
    tap(KEY_A);
    const std::vector<std::pair<uint32_t, bool>> expected = {{KEY_A, true}, {KEY_A, false}};
    EXPECT_EQ(expected, keyboardDelegate->keys);
}

TEST_F(AcceleratorTest, UnderTheCommandConventionApplicationsGetCommandAsCtrl)
{
    seat.setShortcutConvention(input::ShortcutConvention::Command);
    configuration->setConvention(input::ShortcutConvention::Command);
    wm::Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    manager.activation()->activate(window);
    auto delegateOwner = std::make_unique<RecordingKeyboardDelegate>();
    RecordingKeyboardDelegate *keyboardDelegate = delegateOwner.get();
    input::Keyboard keyboard(std::move(delegateOwner), &seat);

    chord({KEY_LEFTALT}, KEY_C);
    const std::vector<std::pair<uint32_t, bool>> expected = {
            {KEY_LEFTCTRL, true}, {KEY_C, true}, {KEY_C, false}, {KEY_LEFTCTRL, false}};
    EXPECT_EQ(expected, keyboardDelegate->keys);

    // The launcher has no default here; Command+Space is the application's.
    keyboardDelegate->keys.clear();
    chord({KEY_LEFTALT}, KEY_SPACE);
    EXPECT_EQ(0, delegate.appListToggles);
    const std::vector<std::pair<uint32_t, bool>> space = {{KEY_LEFTCTRL, true},
                                                          {KEY_SPACE, true},
                                                          {KEY_SPACE, false},
                                                          {KEY_LEFTCTRL, false}};
    EXPECT_EQ(space, keyboardDelegate->keys);

    // Command+Q, Command+H and Command+Option+H are the application menu's
    // Quit, Hide and Hide Others; the window sees none of them.
    int closes = 0;
    window->setCloseCallback([&closes] {
        ++closes;
    });
    wm::Window *other = makeWindow(base::Rect(0, 0, 200, 100));
    manager.activation()->activate(window);
    keyboardDelegate->keys.clear();
    chord({KEY_LEFTALT}, KEY_Q);
    // Quit waits a moment after the keys.
    EXPECT_EQ(0, closes);
    for (int waited = 0; closes == 0 && waited < 1000; waited += 10)
        Lingmo::test::runFor(std::chrono::milliseconds(10));
    EXPECT_EQ(1, closes);
    chord({KEY_LEFTMETA, KEY_LEFTALT}, KEY_H);
    EXPECT_TRUE(other->isHidden());
    EXPECT_FALSE(window->isHidden());
    chord({KEY_LEFTALT}, KEY_H);
    EXPECT_TRUE(window->isHidden());
    EXPECT_TRUE(keyboardDelegate->keys.empty());
    window->setHidden(false);
    manager.activation()->activate(window);

    // Control+Command+F is the compositor's, and Control+Tab the window's.
    chord({KEY_LEFTCTRL, KEY_LEFTALT}, KEY_F);
    EXPECT_TRUE(window->state()->isFullscreen());
    keyboardDelegate->keys.clear();
    chord({KEY_LEFTCTRL}, KEY_TAB);
    const std::vector<std::pair<uint32_t, bool>> tab = {
            {KEY_LEFTCTRL, true}, {KEY_TAB, true}, {KEY_TAB, false}, {KEY_LEFTCTRL, false}};
    EXPECT_EQ(tab, keyboardDelegate->keys);

    // Control+Up shows all windows, and Command+Up stays the window's Ctrl+Up.
    scene::OverviewNode node([] {
        return int64_t(0);
    });
    wm::OverviewController *overview = manager.overview();
    overview->setNodes(&node, nullptr);
    keyboardDelegate->keys.clear();
    chord({KEY_LEFTCTRL}, KEY_UP);
    EXPECT_EQ(wm::OverviewController::State::Entering, overview->state());
    chord({KEY_LEFTCTRL}, KEY_UP);
    EXPECT_EQ(wm::OverviewController::State::Exiting, overview->state());
    overview->exitImmediately();
    EXPECT_TRUE(keyboardDelegate->keys.empty());
    chord({KEY_LEFTALT}, KEY_UP);
    const std::vector<std::pair<uint32_t, bool>> up = {
            {KEY_LEFTCTRL, true}, {KEY_UP, true}, {KEY_UP, false}, {KEY_LEFTCTRL, false}};
    EXPECT_EQ(up, keyboardDelegate->keys);
    overview->setNodes(nullptr, nullptr);
}

TEST_F(AcceleratorTest, ScreenCaptureKeysGoToTheShellUnderBothConventions)
{
    auto delegateOwner = std::make_unique<RecordingKeyboardDelegate>();
    RecordingKeyboardDelegate *keyboardDelegate = delegateOwner.get();
    input::Keyboard keyboard(std::move(delegateOwner), &seat);

    tap(KEY_SYSRQ);
    chord({KEY_LEFTSHIFT}, KEY_SYSRQ);
    chord({KEY_LEFTMETA, KEY_LEFTSHIFT}, KEY_SYSRQ);
    chord({KEY_LEFTCTRL, KEY_LEFTMETA}, KEY_ESC);
    std::vector<std::string> expected = {"capture-mode", "screen", "partial", "stop-recording"};
    EXPECT_EQ(expected, delegate.captures);

    seat.setShortcutConvention(input::ShortcutConvention::Command);
    configuration->setConvention(input::ShortcutConvention::Command);
    delegate.captures.clear();
    keyboardDelegate->keys.clear();

    // Alt on a PC keyboard is Command here.
    chord({KEY_LEFTALT, KEY_LEFTSHIFT}, KEY_3);
    chord({KEY_LEFTCTRL, KEY_LEFTALT, KEY_LEFTSHIFT}, KEY_3);
    chord({KEY_LEFTALT, KEY_LEFTSHIFT}, KEY_4);
    chord({KEY_LEFTCTRL, KEY_LEFTALT, KEY_LEFTSHIFT}, KEY_4);
    chord({KEY_LEFTALT, KEY_LEFTSHIFT}, KEY_5);
    chord({KEY_LEFTCTRL, KEY_LEFTALT}, KEY_ESC);
    // Print works as under PC.
    tap(KEY_SYSRQ);
    chord({KEY_LEFTSHIFT}, KEY_SYSRQ);
    chord({KEY_LEFTALT, KEY_LEFTSHIFT}, KEY_SYSRQ);
    expected = {"screen",       "screen-clipboard", "partial", "partial-clipboard",
                "capture-mode", "stop-recording",   "capture-mode", "screen",
                "partial"};
    EXPECT_EQ(expected, delegate.captures);
    for (const auto &[code, pressed] : keyboardDelegate->keys) {
        EXPECT_NE(static_cast<uint32_t>(KEY_3), code);
        EXPECT_NE(static_cast<uint32_t>(KEY_4), code);
        EXPECT_NE(static_cast<uint32_t>(KEY_5), code);
    }
}

TEST_F(AcceleratorTest, TheClipboardHistoryKeyGoesToTheShellUnderBothConventions)
{
    wm::Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    manager.activation()->activate(window);
    auto delegateOwner = std::make_unique<RecordingKeyboardDelegate>();
    RecordingKeyboardDelegate *keyboardDelegate = delegateOwner.get();
    input::Keyboard keyboard(std::move(delegateOwner), &seat);

    chord({KEY_LEFTMETA}, KEY_V);
    EXPECT_EQ(1, delegate.clipboardHistoryToggles);

    // Control+Command+V, with Alt where Command is on a PC keyboard; Command+V
    // stays the application's paste.
    seat.setShortcutConvention(input::ShortcutConvention::Command);
    configuration->setConvention(input::ShortcutConvention::Command);
    keyboardDelegate->keys.clear();
    chord({KEY_LEFTCTRL, KEY_LEFTALT}, KEY_V);
    EXPECT_EQ(2, delegate.clipboardHistoryToggles);
    for (const auto &[code, pressed] : keyboardDelegate->keys)
        EXPECT_NE(static_cast<uint32_t>(KEY_V), code);
    keyboardDelegate->keys.clear();
    chord({KEY_LEFTALT}, KEY_V);
    EXPECT_EQ(2, delegate.clipboardHistoryToggles);
    const std::vector<std::pair<uint32_t, bool>> paste = {
            {KEY_LEFTCTRL, true}, {KEY_V, true}, {KEY_V, false}, {KEY_LEFTCTRL, false}};
    EXPECT_EQ(paste, keyboardDelegate->keys);
}

TEST_F(AcceleratorTest, SendPasteTypesTheApplicationsPasteShortcut)
{
    auto delegateOwner = std::make_unique<RecordingKeyboardDelegate>();
    RecordingKeyboardDelegate *keyboardDelegate = delegateOwner.get();
    input::Keyboard keyboard(std::move(delegateOwner), &seat);
    wm::Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    manager.activation()->activate(window);

    // Under the Command convention too: the keys go out as clients see them.
    seat.setShortcutConvention(input::ShortcutConvention::Command);
    configuration->setConvention(input::ShortcutConvention::Command);
    manager.sendPaste();
    const std::vector<std::pair<uint32_t, bool>> paste = {
            {KEY_LEFTCTRL, true}, {KEY_V, true}, {KEY_V, false}, {KEY_LEFTCTRL, false}};
    EXPECT_EQ(paste, keyboardDelegate->keys);

    keyboardDelegate->keys.clear();
    wm::Window *terminal = manager.createWindow(
            makeSurface({300, 200}),
            {.requestedSize = {300, 200}, .applicationId = "lingmo-terminal"});
    manager.activation()->activate(terminal);
    manager.sendPaste();
    const std::vector<std::pair<uint32_t, bool>> terminalPaste = {
            {KEY_LEFTCTRL, true}, {KEY_LEFTSHIFT, true}, {KEY_V, true},
            {KEY_V, false},       {KEY_LEFTSHIFT, false}, {KEY_LEFTCTRL, false}};
    EXPECT_EQ(terminalPaste, keyboardDelegate->keys);

    // A held key, such as the Return that picked the entry, is waited for.
    press(KEY_ENTER);
    keyboardDelegate->keys.clear();
    manager.sendPaste();
    EXPECT_TRUE(keyboardDelegate->keys.empty());
    release(KEY_ENTER);
    keyboardDelegate->keys.clear();
    Lingmo::test::runFor(std::chrono::milliseconds(100));
    EXPECT_EQ(terminalPaste, keyboardDelegate->keys);

    // Nothing to the desktop, or while locked.

    wm::Window *desktop = makeWindow(base::Rect(0, 0, 800, 600));
    desktop->setRole(wm::WindowRole::Desktop);
    manager.activation()->activate(desktop);
    ASSERT_EQ(desktop, manager.activation()->activeWindow());
    keyboardDelegate->keys.clear();
    manager.sendPaste();
    EXPECT_TRUE(keyboardDelegate->keys.empty());

    manager.activation()->activate(window);
    ASSERT_TRUE(session.requestLock());
    session.notifyFramePresented();
    ASSERT_EQ(session::SessionState::Locked, session.state());
    keyboardDelegate->keys.clear();
    manager.sendPaste();
    EXPECT_TRUE(keyboardDelegate->keys.empty());
}

TEST_F(AcceleratorTest, APickerBorrowsTheKeyboardUntilAnotherWindowIsActivated)
{
    wm::Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    manager.activation()->activate(window);
    wm::Window *picker = makeWindow(base::Rect(200, 150, 400, 300));
    picker->setRole(wm::WindowRole::Picker);
    EXPECT_FALSE(manager.activation()->canActivate(picker));

    manager.setKeyboardGrab(picker);
    EXPECT_EQ(picker->rootSurface(), seat.focusedSurface());
    EXPECT_EQ(window, manager.activation()->activeWindow());

    wm::Window *other = makeWindow(base::Rect(0, 0, 200, 100));
    manager.activation()->activate(other);
    EXPECT_EQ(nullptr, manager.keyboardGrab());
    EXPECT_EQ(other->rootSurface(), seat.focusedSurface());
}

TEST_F(AcceleratorTest, DeskKeysAddSwitchSendAndRemoveDesksUnderBothConventions)
{
    wm::DesksController *desks = manager.desks();
    wm::Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    manager.activation()->activate(window);

    // Meta+Shift+= adds a desk and goes there; Meta+[ comes back.
    chord({KEY_LEFTMETA, KEY_LEFTSHIFT}, KEY_EQUAL);
    ASSERT_EQ(2u, desks->desks().size());
    EXPECT_EQ(1, desks->activeDeskIndex());
    EXPECT_FALSE(window->isVisible());
    chord({KEY_LEFTMETA}, KEY_LEFTBRACE);
    EXPECT_EQ(0, desks->activeDeskIndex());
    EXPECT_EQ(window, manager.activation()->activeWindow());

    // Meta+Shift+] sends the window right without taking the user along.
    chord({KEY_LEFTMETA, KEY_LEFTSHIFT}, KEY_RIGHTBRACE);
    EXPECT_EQ(0, desks->activeDeskIndex());
    EXPECT_EQ(desks->deskAt(1), desks->deskOf(window));
    // Meta+Shift+2 goes straight to the second desk.
    chord({KEY_LEFTMETA, KEY_LEFTSHIFT}, KEY_2);
    EXPECT_EQ(1, desks->activeDeskIndex());
    EXPECT_EQ(window, manager.activation()->activeWindow());
    // Meta+Shift+A puts it on every desk, and again takes it back.
    chord({KEY_LEFTMETA, KEY_LEFTSHIFT}, KEY_A);
    EXPECT_TRUE(desks->isVisibleOnAllDesks(window));
    chord({KEY_LEFTMETA, KEY_LEFTSHIFT}, KEY_A);
    EXPECT_EQ(desks->deskAt(1), desks->deskOf(window));
    // Meta+Shift+- removes it; the window goes to the desk on the left.
    chord({KEY_LEFTMETA, KEY_LEFTSHIFT}, KEY_MINUS);
    ASSERT_EQ(1u, desks->desks().size());
    EXPECT_TRUE(window->isVisible());

    // Control+Command where PC has Meta; Alt is Command here.
    seat.setShortcutConvention(input::ShortcutConvention::Command);
    configuration->setConvention(input::ShortcutConvention::Command);
    chord({KEY_LEFTCTRL, KEY_LEFTALT}, KEY_EQUAL);
    ASSERT_EQ(2u, desks->desks().size());
    EXPECT_EQ(1, desks->activeDeskIndex());
    chord({KEY_LEFTCTRL, KEY_LEFTALT}, KEY_LEFTBRACE);
    EXPECT_EQ(0, desks->activeDeskIndex());
    chord({KEY_LEFTCTRL, KEY_LEFTALT}, KEY_2);
    EXPECT_EQ(1, desks->activeDeskIndex());
    chord({KEY_LEFTCTRL, KEY_LEFTALT}, KEY_1);
    // Control alone is the application's Ctrl.
    chord({KEY_LEFTCTRL}, KEY_RIGHT);
    chord({KEY_LEFTCTRL}, KEY_2);
    EXPECT_EQ(0, desks->activeDeskIndex());
    // With Shift the window goes along to the next desk.
    chord({KEY_LEFTCTRL, KEY_LEFTALT, KEY_LEFTSHIFT}, KEY_RIGHTBRACE);
    EXPECT_EQ(desks->deskAt(1), desks->deskOf(window));
    chord({KEY_LEFTCTRL, KEY_LEFTALT}, KEY_MINUS);
    EXPECT_EQ(1u, desks->desks().size());
}

TEST_F(AcceleratorTest, SwitchingToAHiddenApplicationBringsItAllBack)
{
    const auto make = [this](const std::string &applicationId) {
        wm::Window *window = manager.createWindow(
                makeSurface({300, 200}),
                {.requestedSize = {300, 200}, .applicationId = applicationId});
        window->setBounds(base::Rect(0, 0, 300, 200));
        return window;
    };
    wm::Window *editor = make("editor");
    wm::Window *editor2 = make("editor");
    wm::Window *viewer = make("viewer");
    for (wm::Window *window : {editor2, editor, viewer})
        manager.activation()->activate(window);
    manager.hideApplication(editor);
    ASSERT_TRUE(editor->isHidden());
    ASSERT_TRUE(editor2->isHidden());

    // The hidden application is still in the list, once, and switching to it
    // shows all of its windows.
    press(KEY_LEFTALT);
    tap(KEY_TAB);
    EXPECT_TRUE(cycle->isCycling());
    const std::vector<wm::Window *> entries = {viewer, editor};
    EXPECT_EQ(entries, cycle->entries());
    EXPECT_TRUE(editor->isHidden());
    release(KEY_LEFTALT);
    EXPECT_FALSE(cycle->isCycling());
    EXPECT_FALSE(editor2->isHidden());
    EXPECT_EQ(editor, manager.activation()->activeWindow());
}

TEST_F(AcceleratorTest, NoCycleStartsWhileAWindowIsDragged)
{
    wm::Window *first = makeWindow(base::Rect(0, 0, 300, 200));
    wm::Window *second = makeWindow(base::Rect(50, 50, 300, 200));
    manager.activation()->activate(first);
    manager.activation()->activate(second);
    ASSERT_TRUE(manager.eventHandler()->beginMove(second, base::PointF(100, 60)));
    press(KEY_LEFTALT);
    tap(KEY_TAB);
    EXPECT_FALSE(cycle->isCycling());
    release(KEY_LEFTALT);
    EXPECT_EQ(second, manager.activation()->activeWindow());

    manager.eventHandler()->completeDrag(wm::ToplevelEventHandler::DragResult::Success);
    press(KEY_LEFTALT);
    tap(KEY_TAB);
    EXPECT_TRUE(cycle->isCycling());
    release(KEY_LEFTALT);
    EXPECT_EQ(first, manager.activation()->activeWindow());
}

TEST_F(AcceleratorTest, CycleSurvivesAWindowClosingUnderIt)
{
    wm::Window *first = makeWindow(base::Rect(0, 0, 300, 200));
    wm::Window *second = makeWindow(base::Rect(50, 50, 300, 200));
    manager.activation()->activate(first);
    manager.activation()->activate(second);
    press(KEY_LEFTALT);
    tap(KEY_TAB);
    EXPECT_EQ(1, cycle->selectedIndex());
    manager.destroyWindow(first);
    EXPECT_TRUE(cycle->isCycling());
    EXPECT_EQ(1u, cycle->entries().size());
    EXPECT_EQ(0, cycle->selectedIndex());
    tap(KEY_TAB);
    release(KEY_LEFTALT);
    EXPECT_FALSE(cycle->isCycling());
    EXPECT_EQ(second, manager.activation()->activeWindow());
}

namespace {

// Lays its entries out as 100 px cells from (0, 600).
struct RecordingCycleView : wm::WindowCycleView {
    std::vector<std::string> calls;
    std::vector<wm::Window *> entries;
    bool shown = false;

    void showWindowCycle(const std::vector<wm::Window *> &list, int selected,
                         const std::string &) override
    {
        entries = list;
        shown = true;
        calls.push_back("show " + std::to_string(selected));
    }
    void selectWindowCycleEntry(int index) override
    {
        calls.push_back("select " + std::to_string(index));
    }
    void hideWindowCycle() override
    {
        shown = false;
        calls.push_back("hide");
    }
    bool windowCycleViewContains(const base::PointF &location) const override
    {
        return shown && base::RectF(0, 600, 100 * entries.size(), 100).contains(location);
    }
    int windowCycleEntryAt(const base::PointF &location) const override
    {
        return windowCycleViewContains(location) ? int(location.x() / 100) : -1;
    }
};

} // namespace

TEST_F(AcceleratorTest, TheApplicationListShowsOnlyOnceTheKeysAreHeld)
{
    RecordingCycleView view;
    cycle->setView(&view);
    wm::Window *first = makeWindow(base::Rect(0, 0, 300, 200));
    wm::Window *second = makeWindow(base::Rect(50, 50, 300, 200));
    manager.activation()->activate(first);
    manager.activation()->activate(second);

    // A quick press switches without it.
    chord({KEY_LEFTALT}, KEY_TAB);
    EXPECT_TRUE(view.calls.empty());
    EXPECT_EQ(first, manager.activation()->activeWindow());

    press(KEY_LEFTALT);
    tap(KEY_TAB);
    cycle->showViewNowForTesting();
    tap(KEY_TAB);
    release(KEY_LEFTALT);
    const std::vector<std::string> expected = {"show 1", "select 0", "hide"};
    EXPECT_EQ(expected, view.calls);
    cycle->setView(nullptr);
}

TEST_F(AcceleratorTest, TheApplicationListTakesThePointer)
{
    RecordingCycleView view;
    cycle->setView(&view);
    wm::Window *first = makeWindow(base::Rect(0, 0, 300, 200));
    wm::Window *second = makeWindow(base::Rect(400, 0, 300, 200));
    wm::Window *third = makeWindow(base::Rect(0, 300, 300, 200));
    for (wm::Window *window : {first, second, third})
        manager.activation()->activate(window);
    input.onPointerMotionAbsolute(at(++time), base::PointF(52, 652));

    press(KEY_LEFTALT);
    tap(KEY_TAB);
    cycle->showViewNowForTesting();
    // The list appeared under a resting pointer; a nudge does not select.
    input.onPointerMotionAbsolute(at(++time), base::PointF(55, 654));
    EXPECT_EQ(1, cycle->selectedIndex());
    // Moving on to an entry does.
    input.onPointerMotionAbsolute(at(++time), base::PointF(250, 650));
    EXPECT_EQ(2, cycle->selectedIndex());
    // A click beside the list does nothing at all.
    input.onPointerMotionAbsolute(at(++time), base::PointF(450, 50));
    input.onPointerButton(at(++time), BTN_LEFT, true);
    input.onPointerButton(at(++time), BTN_LEFT, false);
    EXPECT_TRUE(cycle->isCycling());
    EXPECT_EQ(third, manager.activation()->activeWindow());

    // A click on an entry switches to it.
    input.onPointerMotionAbsolute(at(++time), base::PointF(150, 650));
    input.onPointerButton(at(++time), BTN_LEFT, true);
    input.onPointerButton(at(++time), BTN_LEFT, false);
    EXPECT_FALSE(cycle->isCycling());
    EXPECT_EQ(second, manager.activation()->activeWindow());
    release(KEY_LEFTALT);
    EXPECT_EQ(second, manager.activation()->activeWindow());
    cycle->setView(nullptr);
}

TEST_F(AcceleratorTest, AltTabSwitchesOverAFullscreenWindowButNotPastAnInhibitor)
{
    wm::Window *first = makeWindow(base::Rect(0, 0, 300, 200));
    wm::Window *game = makeWindow(base::Rect(50, 50, 300, 200));
    manager.activation()->activate(first);
    manager.activation()->activate(game);
    game->setFullscreen(true);

    chord({KEY_LEFTALT}, KEY_TAB);
    EXPECT_EQ(first, manager.activation()->activeWindow());

    // A remote desktop or a virtual machine asked for every key.
    struct Inhibitor : input::ShortcutsInhibitorDelegate {
        compositor::Surface *surface = nullptr;
        compositor::Surface *inhibitedSurface() const override { return surface; }
        void onInhibitorActivated() override { }
        void onInhibitorDeactivated() override { }
    };
    manager.activation()->activate(game);
    Inhibitor inhibitor;
    inhibitor.surface = game->rootSurface();
    ASSERT_TRUE(seat.shortcutsInhibition()->add(&inhibitor));
    chord({KEY_LEFTALT}, KEY_TAB);
    EXPECT_FALSE(cycle->isCycling());
    EXPECT_EQ(game, manager.activation()->activeWindow());
    seat.shortcutsInhibition()->remove(&inhibitor);
}

TEST_F(AcceleratorTest, SwitchingBringsTheApplicationsWindowsForwardTogether)
{
    const auto make = [this](const std::string &applicationId, const base::Rect &bounds) {
        wm::Window *window = manager.createWindow(
                makeSurface(bounds.size()),
                {.requestedSize = bounds.size(), .applicationId = applicationId});
        window->setBounds(bounds);
        return window;
    };
    wm::Window *editor = make("editor", base::Rect(0, 0, 300, 200));
    wm::Window *editor2 = make("editor", base::Rect(50, 50, 300, 200));
    wm::Window *viewer = make("viewer", base::Rect(100, 100, 300, 200));
    for (wm::Window *window : {editor, editor2, viewer})
        manager.activation()->activate(window);

    chord({KEY_LEFTALT}, KEY_TAB);
    EXPECT_EQ(editor2, manager.activation()->activeWindow());
    const auto &stacking = manager.windows();
    const auto position = [&stacking](wm::Window *window) {
        return std::find(stacking.begin(), stacking.end(), window) - stacking.begin();
    };
    EXPECT_LT(position(viewer), position(editor));
    EXPECT_LT(position(editor), position(editor2));
}

TEST_F(AcceleratorTest, QAndHQuitAndHideTheSelectedApplication)
{
    int closes = 0;
    const auto make = [this, &closes](const std::string &applicationId) {
        wm::Window *window = manager.createWindow(
                makeSurface({300, 200}), {.requestedSize = {300, 200},
                                          .applicationId = applicationId,
                                          .closeCallback = [&closes] { ++closes; }});
        window->setBounds(base::Rect(0, 0, 300, 200));
        return window;
    };
    wm::Window *editor = make("editor");
    wm::Window *viewer = make("viewer");
    manager.activation()->activate(editor);
    manager.activation()->activate(viewer);

    press(KEY_LEFTALT);
    tap(KEY_TAB);
    tap(KEY_H);
    EXPECT_TRUE(editor->isHidden());
    EXPECT_FALSE(viewer->isHidden());
    tap(KEY_H);
    EXPECT_FALSE(editor->isHidden());
    tap(KEY_Q);
    EXPECT_EQ(1, closes);
    // Its window is still there until the application closes it.
    EXPECT_TRUE(cycle->isCycling());
    manager.destroyWindow(editor);
    const std::vector<wm::Window *> left = {viewer};
    EXPECT_EQ(left, cycle->entries());
    tap(KEY_ESC);
    release(KEY_LEFTALT);
    EXPECT_EQ(viewer, manager.activation()->activeWindow());
}

TEST_F(AcceleratorTest, ABlockedSessionPerformsNothing)
{
    wm::Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    manager.activation()->activate(window);
    ASSERT_TRUE(session.requestLock());
    session.notifyFramePresented();
    ASSERT_EQ(session::SessionState::Locked, session.state());

    // The seat routes keys to the lock screen while blocked; the controller
    // refuses everything as well, should it be asked.
    chord({KEY_LEFTMETA}, KEY_UP);
    EXPECT_EQ(wm::WindowStateType::Normal, window->stateType());
    EXPECT_FALSE(controller->process(Accelerator(XKB_KEY_Up, MetaDown)));
    EXPECT_FALSE(controller->process(Accelerator(XKB_KEY_l, MetaDown)));

    // Apart from the volume, the microphone and the brightness.
    tap(KEY_VOLUMEDOWN);
    tap(KEY_MICMUTE);
    tap(KEY_BRIGHTNESSUP);
    tap(KEY_KBDILLUMDOWN);
    const std::vector<std::string> levels = {"volume-down", "microphone-mute", "brightness-up",
                                             "keyboard-brightness-down"};
    EXPECT_EQ(levels, delegate.levels);
}

TEST_F(AcceleratorTest, WindowActionsAreSwallowedWithoutAWindowAndWhilePrevented)
{
    auto delegateOwner = std::make_unique<RecordingKeyboardDelegate>();
    RecordingKeyboardDelegate *keyboardDelegate = delegateOwner.get();
    input::Keyboard keyboard(std::move(delegateOwner), &seat);
    // No window: the key is consumed all the same, as Ash's
    // PREVENT_PROCESSING_AND_PROPAGATION does.
    EXPECT_TRUE(controller->process(Accelerator(XKB_KEY_Up, MetaDown)));

    controller->setPreventProcessingAccelerators(true);
    chord({KEY_LEFTMETA}, KEY_L);
    EXPECT_FALSE(session.isBlocked());
    controller->setPreventProcessingAccelerators(false);
}

TEST_F(AcceleratorTest, WindowActionsDoNothingInTheOverview)
{
    scene::OverviewNode node([] {
        return int64_t(0);
    });
    wm::OverviewController *overview = manager.overview();
    overview->setNodes(&node, nullptr);
    wm::Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    manager.activation()->activate(window);
    int closes = 0;
    window->setCloseCallback([&closes] {
        ++closes;
    });

    ASSERT_TRUE(overview->enter());
    chord({KEY_LEFTMETA}, KEY_UP);
    chord({KEY_LEFTALT}, KEY_F4);
    EXPECT_EQ(wm::WindowStateType::Normal, window->stateType());
    EXPECT_EQ(0, closes);
    // The overview's own shortcut still works.
    chord({KEY_LEFTMETA}, KEY_TAB);
    EXPECT_EQ(wm::OverviewController::State::Exiting, overview->state());

    // On its way out the window has the shortcuts again.
    chord({KEY_LEFTMETA}, KEY_UP);
    EXPECT_EQ(wm::WindowStateType::Maximized, window->stateType());
    overview->exitImmediately();
    overview->setNodes(nullptr, nullptr);
}

TEST_F(AcceleratorTest, FullscreenWindowsKeepNonReservedKeysExceptMetaOnes)
{
    wm::Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    manager.activation()->activate(window);
    window->setFullscreen(true);
    auto delegateOwner = std::make_unique<RecordingKeyboardDelegate>();
    RecordingKeyboardDelegate *keyboardDelegate = delegateOwner.get();
    input::Keyboard keyboard(std::move(delegateOwner), &seat);

    // Alt+F4 is not reserved: the fullscreen window keeps it.
    int closes = 0;
    window->setCloseCallback([&closes] {
        ++closes;
    });
    chord({KEY_LEFTALT}, KEY_F4);
    EXPECT_EQ(0, closes);
    EXPECT_EQ(4u, keyboardDelegate->keys.size());

    chord({KEY_LEFTMETA}, KEY_F);
    EXPECT_EQ(wm::WindowStateType::Normal, window->stateType());

    // The volume is the system's even over a game.
    keyboardDelegate->keys.clear();
    tap(KEY_VOLUMEUP);
    EXPECT_TRUE(keyboardDelegate->keys.empty());
    EXPECT_EQ(std::vector<std::string> {"volume-up"}, delegate.levels);
}

TEST_F(AcceleratorTest, AShortcutsInhibitorTakesEveryKeyButTheReservedOnesAndMetaEscape)
{
    struct Inhibitor : input::ShortcutsInhibitorDelegate {
        compositor::Surface *surface = nullptr;
        bool active = false;

        compositor::Surface *inhibitedSurface() const override { return surface; }

        void onInhibitorActivated() override { active = true; }

        void onInhibitorDeactivated() override { active = false; }
    };

    wm::Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    manager.activation()->activate(window);
    auto delegateOwner = std::make_unique<RecordingKeyboardDelegate>();
    RecordingKeyboardDelegate *keyboardDelegate = delegateOwner.get();
    input::Keyboard keyboard(std::move(delegateOwner), &seat);
    Inhibitor inhibitor;
    inhibitor.surface = window->rootSurface();
    ASSERT_TRUE(seat.shortcutsInhibition()->add(&inhibitor));
    EXPECT_TRUE(inhibitor.active);

    // Meta ones included, which even a fullscreen window does not get.
    chord({KEY_LEFTMETA}, KEY_UP);
    EXPECT_EQ(wm::WindowStateType::Normal, window->stateType());
    chord({KEY_LEFTALT}, KEY_TAB);
    EXPECT_FALSE(cycle->isCycling());
    EXPECT_EQ(8u, keyboardDelegate->keys.size());

    // Exo's inhibitor exists for the system keys.
    tap(KEY_VOLUMEUP);
    EXPECT_EQ(10u, keyboardDelegate->keys.size());
    EXPECT_TRUE(delegate.levels.empty());

    // Meta+Escape takes the shortcuts back, until a click on the window.
    keyboardDelegate->keys.clear();
    chord({KEY_LEFTMETA}, KEY_ESC);
    EXPECT_FALSE(inhibitor.active);
    std::vector<std::pair<uint32_t, bool>> expected = {{KEY_LEFTMETA, true}, {KEY_LEFTMETA, false}};
    EXPECT_EQ(expected, keyboardDelegate->keys);
    chord({KEY_LEFTMETA}, KEY_UP);
    EXPECT_EQ(wm::WindowStateType::Maximized, window->stateType());
    input.onPointerMotionAbsolute(at(++time), base::PointF(window->contentBounds().topLeft() + base::Point(10, 10)));
    input.onPointerButton(at(++time), BTN_LEFT, true);
    input.onPointerButton(at(++time), BTN_LEFT, false);
    EXPECT_TRUE(inhibitor.active);

    // Nothing to take back: Meta+Escape is an ordinary key again.
    seat.shortcutsInhibition()->remove(&inhibitor);
    keyboardDelegate->keys.clear();
    chord({KEY_LEFTMETA}, KEY_ESC);
    EXPECT_EQ(4u, keyboardDelegate->keys.size());

    ASSERT_TRUE(seat.shortcutsInhibition()->add(&inhibitor));
    chord({KEY_LEFTMETA}, KEY_L);
    EXPECT_TRUE(session.isBlocked());
    EXPECT_FALSE(inhibitor.active);
    seat.shortcutsInhibition()->remove(&inhibitor);
}

////////////////////////////////////////////////////////////////////////////////
// The D-Bus service

TEST_F(AcceleratorTest, ServiceValidatesConfirmsAndReportsConflicts)
{
    AcceleratorService service(configuration.get(), controller.get(), seat.xkbTracker());
    const std::string maximize = "toggle-maximized";

    EXPECT_EQ("missing-modifier",
              parse(service.addAccelerator(maximize, KEY_K, 0, NoModifier))
                      ["result"].toString());
    EXPECT_EQ("shift-only-not-allowed",
              parse(service.addAccelerator(maximize, KEY_K, 0, ShiftDown))
                      ["result"].toString());
    EXPECT_EQ("key-not-allowed",
              parse(service.addAccelerator(maximize, KEY_LEFTSHIFT, 0, MetaDown))
                      ["result"].toString());
    EXPECT_EQ("reserved-key-not-allowed",
              parse(service.addAccelerator(maximize, 0, XKB_KEY_Caps_Lock, MetaDown))
                      ["result"].toString());
    EXPECT_EQ("not-found",
              parse(service.addAccelerator("no-such", KEY_K, 0, MetaDown))
                      ["result"].toString());

    // Meta+Down belongs to minimize: the first request asks, the second takes.
    base::json::Value answer = parse(service.addAccelerator(maximize, KEY_DOWN, 0, MetaDown));
    EXPECT_EQ("conflict-can-override", answer["result"].toString());
    EXPECT_EQ("window-minimize", answer["conflictAction"].toString());
    answer = parse(service.addAccelerator(maximize, KEY_DOWN, 0, MetaDown));
    EXPECT_EQ("success", answer["result"].toString());
    EXPECT_EQ(AcceleratorAction::ToggleMaximized,
              *configuration->findAcceleratorAction(Accelerator(XKB_KEY_Down, MetaDown)));

    // Already this action's own: a plain conflict.
    answer = parse(service.addAccelerator(maximize, KEY_UP, 0, MetaDown));
    EXPECT_EQ("conflict", answer["result"].toString());

    // Without Meta the first request only warns; the second goes through.
    answer = parse(service.addAccelerator(maximize, KEY_K, 0, ControlDown | AltDown));
    EXPECT_EQ("non-meta-accelerator-warning",
              answer["result"].toString());
    answer = parse(service.addAccelerator(maximize, KEY_K, 0, ControlDown | AltDown));
    EXPECT_EQ("success", answer["result"].toString());

    // Starting the recorder again forgets any pending confirmation.
    answer = parse(service.addAccelerator(maximize, KEY_J, 0, ControlDown | AltDown));
    EXPECT_EQ("non-meta-accelerator-warning",
              answer["result"].toString());
    service.preventProcessingAccelerators(true);
    EXPECT_TRUE(controller->shouldPreventProcessingAccelerators());
    service.preventProcessingAccelerators(false);
    answer = parse(service.addAccelerator(maximize, KEY_J, 0, ControlDown | AltDown));
    EXPECT_EQ("non-meta-accelerator-warning",
              answer["result"].toString());
    answer = parse(service.addAccelerator(maximize, KEY_J, 0, ControlDown | AltDown));
    EXPECT_EQ("success", answer["result"].toString());

    // Under Command, Command+key is the application's Ctrl+key; only
    // Control+Command is clear of it.
    seat.setShortcutConvention(input::ShortcutConvention::Command);
    configuration->setConvention(input::ShortcutConvention::Command);
    const std::string launcher = "toggle-app-list";
    answer = parse(service.addAccelerator(launcher, KEY_K, 0, MetaDown));
    EXPECT_EQ("non-meta-accelerator-warning",
              answer["result"].toString());
    answer = parse(service.addAccelerator(launcher, KEY_K, 0, ControlDown | MetaDown));
    EXPECT_EQ("success", answer["result"].toString());
    seat.setShortcutConvention(input::ShortcutConvention::Pc);
    configuration->setConvention(input::ShortcutConvention::Pc);

    // Five accelerators per action at most.
    EXPECT_EQ(5u, configuration->acceleratorsForAction(
                          toActionId(AcceleratorAction::ToggleMaximized))->size());
    EXPECT_EQ("maximum-accelerators-reached",
              parse(service.addAccelerator(maximize, KEY_Z, 0, MetaDown))
                      ["result"].toString());
    EXPECT_EQ("success",
              parse(service.removeAccelerator(maximize, XKB_KEY_j, ControlDown | AltDown))
                      ["result"].toString());

    // The keysym the toolkit saw is the fallback when no keycode is known; a
    // known keycode wins because the compositor's keymap decides.
    EXPECT_EQ("success",
              parse(service.addAccelerator(maximize, 0, XKB_KEY_X, MetaDown))
                      ["result"].toString());
    EXPECT_TRUE(configuration->findAcceleratorAction(Accelerator(XKB_KEY_x, MetaDown)));
    answer = parse(service.describeKey(KEY_1, XKB_KEY_exclam));
    EXPECT_EQ("1", answer["keyText"].toString());

    // An action without a default starts empty and takes a binding like any
    // other.
    EXPECT_EQ("success",
              parse(service.addAccelerator("toggle-app-list", KEY_SPACE, 0,
                                           MetaDown | ShiftDown))
                      ["result"].toString());
    chord({KEY_LEFTMETA, KEY_LEFTSHIFT}, KEY_SPACE);
    EXPECT_EQ(1, delegate.appListToggles);
    EXPECT_EQ("success",
              parse(service.restoreDefault("toggle-app-list"))
                      ["result"].toString());
    chord({KEY_LEFTMETA}, KEY_SPACE);
    EXPECT_EQ(1, delegate.appListToggles);

    // The launcher's default is Ash's lone Meta tap: it fires on release, only
    // when nothing else was pressed meanwhile, and the client still sees both
    // halves of the key.
    const Accelerator metaTap(XKB_KEY_Super_L, NoModifier, Accelerator::KeyState::Released);
    EXPECT_TRUE(configuration->findAcceleratorAction(metaTap));
    tap(KEY_LEFTMETA);
    EXPECT_EQ(2, delegate.appListToggles);
    chord({KEY_LEFTMETA}, KEY_A);
    EXPECT_EQ(2, delegate.appListToggles);
    EXPECT_EQ("missing-modifier",
              parse(service.addAccelerator("toggle-app-list", KEY_LEFTSHIFT, 0,
                                           NoModifier))
                      ["result"].toString());
    EXPECT_EQ("success",
              parse(service.removeAccelerator("toggle-app-list", XKB_KEY_Super_L,
                                              NoModifier))
                      ["result"].toString());
    tap(KEY_LEFTMETA);
    EXPECT_EQ(2, delegate.appListToggles);
    // Recording a lone Meta binds the same tap again.
    EXPECT_EQ("success",
              parse(service.addAccelerator("toggle-app-list", KEY_LEFTMETA, 0,
                                           NoModifier))
                      ["result"].toString());
    EXPECT_TRUE(configuration->findAcceleratorAction(metaTap));
    tap(KEY_LEFTMETA);
    EXPECT_EQ(3, delegate.appListToggles);
    EXPECT_EQ("success",
              parse(service.restoreDefault("toggle-app-list"))
                      ["result"].toString());

    EXPECT_TRUE(service.hasCustomAccelerators());
    EXPECT_EQ("success",
              parse(service.restoreDefault(maximize))["result"].toString());
    // Meta+Down went back to minimize with maximize's defaults: nothing is
    // custom any more.
    EXPECT_FALSE(service.hasCustomAccelerators());
    EXPECT_EQ("success",
              parse(service.restoreAllDefaults())["result"].toString());
    EXPECT_FALSE(service.hasCustomAccelerators());
}

TEST_F(AcceleratorTest, PositionalLayoutsPinPunctuationAndStoreTheUsKey)
{
    if (!seat.xkbTracker()->updateKeyboardLayout("de"))
        GTEST_SKIP() << "no German keymap installed";
    controller->setUsePositionalLookup(true);
    wm::Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    manager.activation()->activate(window);

    // Alt+] is Alt+"+" on a German keyboard, at the same position.
    chord({KEY_LEFTALT}, KEY_RIGHTBRACE);
    EXPECT_EQ(wm::WindowStateType::SecondarySnapped, window->stateType());
    // Alt+= (maximize) sits under Alt+"´" there; letters keep their meaning:
    // Meta+"Y" is the German key where the US layout has Z.
    chord({KEY_LEFTALT}, KEY_EQUAL);
    EXPECT_EQ(wm::WindowStateType::Maximized, window->stateType());

    AcceleratorService service(configuration.get(), controller.get(), seat.xkbTracker());
    EXPECT_EQ("success",
              parse(service.addAccelerator("window-minimize", KEY_SLASH, 0,
                                           MetaDown))
                      ["result"].toString());
    // Stored as the US key at that position, so the physical key finds it.
    EXPECT_TRUE(configuration->findAcceleratorAction(Accelerator(XKB_KEY_slash, MetaDown)));
    chord({KEY_LEFTMETA}, KEY_SLASH);
    EXPECT_EQ(wm::WindowStateType::Minimized, window->stateType());
}

TEST_F(AcceleratorTest, ServiceListsActionsWithTheirDefaultsOverTheBus)
{
    base::EventLoop loop;
    AcceleratorService service(configuration.get(), controller.get(), seat.xkbTracker());
    ASSERT_TRUE(service.registerService());
    input_test::RemoteObject proxy(loop, "org.lingmo.Shortcuts1", "/org/lingmo/Shortcuts1",
                                   "org.lingmo.Shortcuts1");
    ASSERT_TRUE(proxy.isValid());
    int changes = 0;
    auto counted = service.acceleratorsChanged.connect([&changes] { ++changes; });

    const auto reply = proxy.call("accelerators");
    ASSERT_FALSE(reply.isError()) << reply.error().message;
    const base::json::Value actions = parse(reply.value<std::string>().value_or(""))["actions"];
    EXPECT_EQ(46u, actions.size());
    bool sawMaximize = false;
    bool sawOverview = false;
    for (const base::json::Value &action : actions.toArray()) {
        if (action["name"].toString() == "toggle-overview") {
            // Meta+Tab and the dedicated key.
            sawOverview = true;
            const base::json::Value &defaults = action["defaults"];
            ASSERT_EQ(2u, defaults.size());
            EXPECT_EQ("Tab", defaults[0]["keyName"].toString());
            EXPECT_EQ(static_cast<int>(MetaDown), defaults[0]["modifiers"].toInt());
            EXPECT_EQ(XKB_KEY_XF86LaunchA, defaults[1]["keysym"].toInt());
            continue;
        }
        if (action["name"].toString() == "toggle-app-list") {
            // The lone Meta tap, which triggers on release.
            const base::json::Value &defaults = action["defaults"];
            ASSERT_EQ(1u, defaults.size());
            const base::json::Value &tap = defaults[0];
            EXPECT_EQ(XKB_KEY_Super_L, tap["keysym"].toInt());
            EXPECT_EQ("released", tap["keyState"].toString());
            continue;
        }
        if (action["name"].toString() != "toggle-maximized")
            continue;
        sawMaximize = true;
        const base::json::Value &accelerators = action["accelerators"];
        ASSERT_EQ(2u, accelerators.size());
        const base::json::Value &metaUp = accelerators[1];
        EXPECT_EQ("Up", metaUp["keyName"].toString());
        EXPECT_EQ(static_cast<int>(MetaDown), metaUp["modifiers"].toInt());
        EXPECT_EQ("default", metaUp["type"].toString());
        EXPECT_EQ(accelerators, action["defaults"]);
        // What Settings has always read: the members in sorted order.
        EXPECT_EQ(R"({"keyName":"Up","keyState":"pressed","keyText":"","keysym":65362,)"
                  R"("modifiers":8,"type":"default"})",
                  metaUp.serialize());
    }
    EXPECT_TRUE(sawMaximize);
    EXPECT_TRUE(sawOverview);

    const auto added = proxy.call("addAccelerator", std::string("lock-screen"),
                                  uint32_t(KEY_ESC), uint32_t(0), uint32_t(MetaDown));
    ASSERT_FALSE(added.isError()) << added.error().message;
    EXPECT_EQ(R"({"result":"success"})", added.value<std::string>());
    EXPECT_EQ(1, changes);
}

TEST_F(AcceleratorTest, ARecorderThatLeavesTheBusLiftsTheAcceleratorBlock)
{
    base::EventLoop loop;
    AcceleratorService service(configuration.get(), controller.get(), seat.xkbTracker());
    ASSERT_TRUE(service.registerService());

    // A second connection stands in for Settings, which dies while recording.
    {
        input_test::RemoteObject recorder(loop, "org.lingmo.Shortcuts1",
                                          "/org/lingmo/Shortcuts1", "org.lingmo.Shortcuts1");
        ASSERT_TRUE(recorder.isValid());
        EXPECT_FALSE(recorder.call("preventProcessingAccelerators", true).isError());
        EXPECT_TRUE(controller->shouldPreventProcessingAccelerators());
    }
    EXPECT_TRUE(input_test::runUntil(
            loop, [&] { return !controller->shouldPreventProcessingAccelerators(); }));
}

TEST_F(AcceleratorTest, GlobalSearchUsesMetaSpaceWithoutOpeningLauncher)
{
    chord({KEY_LEFTMETA}, KEY_SPACE);
    EXPECT_EQ(1, delegate.searchToggles);
    EXPECT_EQ(0, delegate.appListToggles);
    chord({KEY_LEFTMETA}, KEY_L);
    ASSERT_TRUE(session.isBlocked());
    chord({KEY_LEFTMETA}, KEY_SPACE);
    EXPECT_EQ(1, delegate.searchToggles);
}

TEST_F(AcceleratorTest, ScreenKeyboardShortcutIsBlockedOnLockScreen)
{
    chord({KEY_LEFTMETA, KEY_LEFTCTRL}, KEY_K);
    EXPECT_EQ(1, delegate.screenKeyboardToggles);
    chord({KEY_LEFTMETA}, KEY_L);
    ASSERT_TRUE(session.isBlocked());
    chord({KEY_LEFTMETA, KEY_LEFTCTRL}, KEY_K);
    EXPECT_EQ(1, delegate.screenKeyboardToggles);
}
