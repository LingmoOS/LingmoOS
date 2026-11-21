// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "base/file.h"
#include "base/geometry.h"
#include "input/event_sink.h"
#include "input/input_manager.h"
#include "input/keyboard.h"
#include "input/pointer.h"
#include "input/pointer_delegate.h"
#include <chrono>
#include <linux/input-event-codes.h>
#include <unistd.h>

#include "clipboard/selection.h"
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "decoration/decoration.h"
#include "decoration/decoration_theme.h"
#include "fake_renderer.h"
#include "input/drag.h"
#include "input/hit_test.h"
#include "input/seat.h"
#include "input/xkb_tracker.h"
#include "loop_test_util.h"
#include "render/scanout.h"
#include "scene/node.h"
#include "scene/overlay_candidate.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "session/secure_container.h"
#include "session/session_controller.h"
#include "session/session_teardown.h"
#include "shm_test_util.h"
#include "wm/activation.h"
#include "wm/shell_surface.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/desks/desks_controller.h"
#include "wm/window_manager.h"

using namespace Lingmo;
using session::SessionState;

namespace {

const base::Rect kPrimary(0, 0, 1000, 800);
const base::Rect kSecondary(1000, 0, 800, 600);
const std::string kPrimaryName = "DP-1";
const std::string kSecondaryName = "HDMI-1";

class TestDataSource : public clipboard::DataSource
{
public:
    void send(const std::string &, int fd) override { ::close(fd); }
};

class TestTexture final : public render::Texture
{
public:
    explicit TestTexture(const base::Size &size)
        : m_size(size)
    {
    }

    base::Size size() const override { return m_size; }

private:
    base::Size m_size;
};

class TestScanoutBuffer final : public render::ScanoutBuffer
{
public:
    explicit TestScanoutBuffer(const base::Size &size)
    {
        m_attributes.size = size;
        m_attributes.format = 0x34325258; // DRM_FORMAT_XRGB8888
        m_attributes.modifier = 0;
        m_attributes.planes.push_back(render::DmaBufPlane {ScopedFd(), 0, 4096});
    }

    const render::DmaBufAttributes &scanoutAttributes() const override { return m_attributes; }

private:
    render::DmaBufAttributes m_attributes;
};

class ClientKeyboard : public input::KeyboardDelegate
{
public:
    int keys = 0;
    PressedKeys entered;

    bool canAcceptKeyboardEventsForSurface(compositor::Surface *) const override { return true; }

    void onKeyboardEnter(compositor::Surface *, const PressedKeys &value) override
    {
        entered = value;
    }

    void onKeyboardLeave(compositor::Surface *) override { }

    uint32_t onKeyboardKey(input::Timestamp, uint32_t, bool) override { return ++keys; }

    void onKeyboardModifiers(const input::KeyboardModifiers &) override { }

    void onKeyRepeatSettingsChanged(bool, std::chrono::milliseconds,
                                    std::chrono::milliseconds) override
    {
    }

    void onKeyboardLayoutUpdated(const std::string &) override { }
};

class ClientPointer : public input::PointerDelegate
{
public:
    int events = 0;
    std::vector<std::pair<input::ButtonFlags, bool>> buttons;

    void onPointerDestroying(input::Pointer *) override { }

    bool canAcceptPointerEventsForSurface(compositor::Surface *) const override { return true; }

    void onPointerEnter(compositor::Surface *, const base::PointF &, input::ButtonFlags) override
    {
        ++events;
    }

    void onPointerLeave(compositor::Surface *) override { }

    void onPointerMotion(input::Timestamp, const base::PointF &) override { ++events; }

    void onPointerButton(input::Timestamp, input::ButtonFlags button, bool pressed) override
    {
        ++events;
        buttons.emplace_back(button, pressed);
    }

    void onPointerScroll(input::Timestamp, const base::PointF &, bool) override { ++events; }

    void onFingerScrollStop(input::Timestamp) override { ++events; }

    void onPointerFrame() override { }
};

class SessionLockTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node *surfaceContainer =
            scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    scene::Node *secureNode =
            scene.root()->addChild(std::make_unique<scene::Node>("secure"));
    scene::Node *greeterNode =
            scene.root()->addChild(std::make_unique<scene::Node>("greeter"));
    compositor::Compositor compositor {surfaceContainer};
    testsupport::FakeRenderer renderer;
    input::Seat seat {&compositor};
    // Ahead of the manager: the frames of its windows hold them.
    decoration::Theme theme;
    decoration::DecorationProvider frames {&theme};
    wm::WindowManager manager {&compositor, &seat};

    session::SecureContainer secure {secureNode};
    session::SessionTeardown teardown {&seat, &manager};
    session::SessionController controller {&secure, &teardown};

    std::vector<std::unique_ptr<compositor::Surface>> surfaces;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(16 << 20);

    void SetUp() override
    {
        scene.setSize(base::Size(1800, 800));
        manager.setOutputBounds(kPrimary);
        seat.setSessionGate(&controller);
        manager.setSessionGate(&controller);
        secure.setOutputs({{kPrimaryName, kPrimary}});
    }

    void TearDown() override { seat.shutdown(); }

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

    void lockFully()
    {
        ASSERT_TRUE(controller.requestLock());
        controller.notifyFramePresented();
        ASSERT_EQ(SessionState::Locked, controller.state());
    }

    compositor::Surface *surfaceAt(const base::PointF &location) const
    {
        base::PointF local;
        return input::findTargetSurfaceIn(
                const_cast<compositor::Compositor *>(&compositor),
                input::inputRootFor(const_cast<compositor::Compositor *>(&compositor), &controller),
                location, &local);
    }
};

TEST_F(SessionLockTest, LockAndUnlockOnlyCompleteOnAPresentedFrame)
{
    EXPECT_EQ(SessionState::Active, controller.state());
    EXPECT_FALSE(controller.isBlocked());

    ASSERT_TRUE(controller.requestLock());
    EXPECT_EQ(SessionState::Locking, controller.state());
    EXPECT_TRUE(controller.isBlocked());

    controller.notifyFramePresented();
    EXPECT_EQ(SessionState::Locked, controller.state());

    ASSERT_TRUE(controller.requestUnlock());
    EXPECT_EQ(SessionState::Unlocking, controller.state());
    EXPECT_TRUE(controller.isBlocked());

    controller.notifyFramePresented();
    EXPECT_EQ(SessionState::Active, controller.state());
    EXPECT_FALSE(controller.isBlocked());
}

TEST_F(SessionLockTest, ASecondLockIsRefusedWhileBlocked)
{
    ASSERT_TRUE(controller.requestLock());
    EXPECT_FALSE(controller.requestLock());
    controller.notifyFramePresented();
    EXPECT_FALSE(controller.requestLock());
    EXPECT_EQ(SessionState::Locked, controller.state());
}

TEST_F(SessionLockTest, ALockDuringTheUnlockCancelsItAndCoversAtOnce)
{
    lockFully();
    ASSERT_TRUE(controller.requestUnlock());
    ASSERT_EQ(SessionState::Unlocking, controller.state());

    EXPECT_TRUE(controller.requestLock());
    EXPECT_EQ(SessionState::Locking, controller.state());
    EXPECT_TRUE(controller.isBlocked());
    EXPECT_TRUE(secure.isSettled(0));
    const session::CoverSample cover = secure.sample(0, 16);
    EXPECT_EQ(1.0f, cover.backdrop);
    EXPECT_EQ(1.0f, cover.panelOpacity);
    EXPECT_EQ(1.0, cover.panelScale);
    EXPECT_FALSE(controller.requestLock());

    controller.notifyFramePresented();
    EXPECT_EQ(SessionState::Locked, controller.state());
}

TEST_F(SessionLockTest, AnUnlockFromTheLoginScreenIsNotTurnedIntoALock)
{
    session::SessionController login {&secure, nullptr, SessionState::Login};
    ASSERT_TRUE(login.requestUnlock());
    EXPECT_FALSE(login.requestLock());
    EXPECT_EQ(SessionState::Unlocking, login.state());
}

TEST_F(SessionLockTest, UnlockIsRefusedBeforeTheSessionIsLocked)
{
    EXPECT_FALSE(controller.requestUnlock());
    ASSERT_TRUE(controller.requestLock());
    EXPECT_FALSE(controller.requestUnlock());
    EXPECT_EQ(SessionState::Locking, controller.state());
}

TEST_F(SessionLockTest, LoginIsBlockedButIsNotALockedSession)
{
    session::SecureContainer greeterScene(greeterNode);
    session::SessionController greeter(&greeterScene, nullptr, SessionState::Login);
    greeterScene.setOutputs({{kPrimaryName, kPrimary}});

    EXPECT_EQ(SessionState::Login, greeter.state());
    EXPECT_TRUE(greeter.isBlocked());
    EXPECT_FALSE(greeter.allowsActivation());
    EXPECT_FALSE(greeter.allowsCapture());
    EXPECT_FALSE(greeter.allowsDirectScanout());
    EXPECT_TRUE(greeter.restrictsInput());
    EXPECT_TRUE(greeterScene.node()->isVisible());
    EXPECT_EQ(nullptr, greeter.inputRoot());

    EXPECT_EQ(SessionState::Login, greeter.state());
}

TEST_F(SessionLockTest, TheLoginScreenIsReadyOnceEveryOutputHasItsShield)
{
    session::SecureContainer greeterScene(greeterNode);
    EXPECT_FALSE(greeterScene.isWallpaperReady());
    greeterScene.setOutputs({{kPrimaryName, kPrimary}});
    // The wallpaper decodes on a worker; the shield is ready once it lands.
    test::runUntil([&] { return greeterScene.isWallpaperReady(); }, std::chrono::seconds(30));
    EXPECT_TRUE(greeterScene.isWallpaperReady());
}

TEST_F(SessionLockTest, ThePickedAccountsWallpaperFadesInOverTheOneShown)
{
    base::file::TemporaryDirectory directory("/tmp", "session-lock-test");
    const std::string alice = directory.path() + "/alice.conf";
    const std::string bob = directory.path() + "/bob.conf";
    ASSERT_TRUE(base::file::writeAtomically(alice, "[Appearance]\nBackgroundType=1\nBackgroundColor=#ff0000\n"));
    ASSERT_TRUE(base::file::writeAtomically(bob, "[Appearance]\nBackgroundType=1\nBackgroundColor=#0000ff\n"));

    session::SecureContainer greeterScene(greeterNode);
    greeterScene.setSettingsFile(alice, false);
    greeterScene.setOutputs({{kPrimaryName, kPrimary}});
    ASSERT_TRUE(test::runUntil([&] { return greeterScene.isWallpaperReady(); }, std::chrono::seconds(30)));
    const scene::Node *shield = greeterScene.shieldContainer()->children().front();
    EXPECT_EQ(1u, shield->children().size());

    // Alice's stays up under Bob's until Bob's has faded in over it.
    greeterScene.setSettingsFile(bob, true);
    EXPECT_EQ(2u, shield->children().size());
    ASSERT_TRUE(test::runUntil([&] { return shield->children().size() == 1; }, std::chrono::seconds(30)));
    EXPECT_TRUE(greeterScene.isWallpaperReady());

    // Without the crossfade the old one goes at once.
    greeterScene.setSettingsFile(alice, false);
    EXPECT_EQ(1u, shield->children().size());
    EXPECT_FALSE(greeterScene.isWallpaperReady());
}

TEST_F(SessionLockTest, LoginUnlocksIntoAnActiveSessionTheSameWayALockDoes)
{
    session::SecureContainer greeterScene(greeterNode);
    session::SessionController greeter(&greeterScene, nullptr, SessionState::Login);
    greeterScene.setOutputs({{kPrimaryName, kPrimary}});

    EXPECT_FALSE(greeter.requestLock());
    EXPECT_EQ(SessionState::Login, greeter.state());

    ASSERT_TRUE(greeter.requestUnlock());
    EXPECT_EQ(SessionState::Unlocking, greeter.state());
    greeter.notifyFramePresented();
    EXPECT_EQ(SessionState::Active, greeter.state());

    ASSERT_TRUE(greeter.requestLock());
    greeter.notifyFramePresented();
    EXPECT_EQ(SessionState::Locked, greeter.state());
}

TEST_F(SessionLockTest, InputIsConfinedToTheSecureContainerWhileLocked)
{
    wm::Window *window = makeWindow(base::Rect(100, 100, 400, 300));
    const base::PointF inWindow(200, 200);
    ASSERT_EQ(window, manager.windowAt(inWindow));

    lockFully();

    EXPECT_EQ(nullptr, manager.windowAt(inWindow));
    EXPECT_EQ(nullptr, surfaceAt(inWindow));
}

TEST_F(SessionLockTest, NothingIsTargetableOnAnOutputWithNoLockSurface)
{
    makeWindow(base::Rect(100, 100, 400, 300));
    lockFully();

    EXPECT_EQ(nullptr, surfaceAt(base::PointF(200, 200)));
    EXPECT_EQ(nullptr, manager.windowAt(base::PointF(200, 200)));
}

TEST_F(SessionLockTest, ServerSideFramesTakeNoInputWhileLocked)
{
    wm::Window *window = manager.createWindow(
            makeSurface(base::Size(400, 300)), {.requestedSize = base::Size(400, 300), .decorated = true});
    window->setBounds(base::Rect(100, 100, 400, 300));
    lockFully();

    for (const base::PointF point : {base::PointF(100, 100), base::PointF(300, 105), base::PointF(500, 400)})
        EXPECT_EQ(nullptr, manager.frameAt(point)) << "frame took input at " << point.x();
}

TEST_F(SessionLockTest, LockingClearsFocusAndRefusesItAfterwards)
{
    compositor::Surface *application = makeSurface(base::Size(400, 300));
    seat.setFocusedSurface(application);
    ASSERT_EQ(application, seat.focusedSurface());

    lockFully();

    EXPECT_EQ(nullptr, seat.focusedSurface());
    EXPECT_FALSE(seat.allowsFocus(application));
    seat.setFocusedSurface(application);
    EXPECT_EQ(nullptr, seat.focusedSurface());
}

TEST_F(SessionLockTest, LockingDismissesPopupGrabs)
{
    wm::Window *window = makeWindow(base::Rect(100, 100, 400, 300));
    wm::ShellSurface popup(&manager, window->rootSurface());
    manager.pushPopupGrab(&popup);
    ASSERT_EQ(1u, manager.popupGrabs().size());

    ASSERT_TRUE(controller.requestLock());

    EXPECT_TRUE(manager.popupGrabs().empty());
}

TEST_F(SessionLockTest, LockingCancelsTheDragAndTakesItsIconOutOfTheScene)
{
    scene::Node *dragIcons =
            scene.root()->addChild(std::make_unique<scene::Node>("drag-icons"));
    seat.setDragIconContainer(dragIcons);

    compositor::Surface *origin = makeSurface({200, 150});
    compositor::Surface *icon = makeSurface({32, 32});
    auto source = std::make_shared<TestDataSource>();
    seat.drag()->start(origin, icon, source, 1, input::LeftButton);
    ASSERT_TRUE(seat.drag()->active());
    ASSERT_EQ(dragIcons, icon->node()->parent());

    ASSERT_TRUE(controller.requestLock());

    EXPECT_FALSE(seat.drag()->active());
    EXPECT_NE(dragIcons, icon->node()->parent());
    EXPECT_FALSE(seat.cursorOverride().has_value());
}

TEST_F(SessionLockTest, AWindowMoveTheLockInterruptedDoesNotResumeAfterUnlock)
{
    wm::Window *window = makeWindow(base::Rect(100, 100, 400, 300));
    input::InputManager input(&seat);
    input.setBounds(scene.size());
    input.setEventSink(&manager);
    input.warpPointer(base::PointF(150, 150));
    input.onPointerButton({}, BTN_LEFT, true);
    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(150, 150)));

    lockFully();
    input.onPointerButton({}, BTN_LEFT, false);
    ASSERT_TRUE(controller.requestUnlock());
    controller.notifyFramePresented();

    EXPECT_EQ(input::ButtonFlags(input::NoButton), seat.pressedButtons());
    const base::Rect bounds = window->bounds();
    input.onPointerMotion({}, base::PointF(40, 30));
    EXPECT_FALSE(manager.eventHandler()->isDragging());
    EXPECT_EQ(bounds, window->bounds());
    input.setEventSink(nullptr);
}

TEST_F(SessionLockTest, ACaptionButtonPressedAcrossTheLockIsLetGoOf)
{
    manager.setFrameProvider(&frames);
    wm::Window *window = manager.createWindow(
            makeSurface(base::Size(400, 300)), {.requestedSize = base::Size(400, 300), .decorated = true});
    window->setBounds(base::Rect(100, 100, 400, 300));
    auto *frame = static_cast<decoration::Decoration *>(window->frame());
    ASSERT_NE(nullptr, frame);
    ASSERT_FALSE(frame->buttons().empty());
    input::InputManager input(&seat);
    input.setBounds(scene.size());
    input.setEventSink(&manager);
    input.warpPointer(
            base::PointF(window->bounds().topLeft() + frame->buttons().front().geometry().center()));
    input.onPointerButton({}, BTN_LEFT, true);
    ASSERT_TRUE(frame->isTrackingPress());

    lockFully();
    input.onPointerButton({}, BTN_LEFT, false);
    ASSERT_TRUE(controller.requestUnlock());
    controller.notifyFramePresented();

    EXPECT_FALSE(frame->isTrackingPress());
    input.setEventSink(nullptr);
}

TEST_F(SessionLockTest, AControlClickTheLockInterruptedLeavesNoSecondaryClickBehind)
{
    ASSERT_TRUE(seat.xkbTracker()->updateKeyboardLayout("us"));
    seat.setShortcutConvention(input::ShortcutConvention::Command);
    makeSurface({400, 300});
    ClientPointer client;
    input::Pointer pointer(&client, &seat);
    input::InputManager input(&seat);
    input.setBounds(scene.size());
    input.onPointerMotionAbsolute({}, {20, 20});
    input.onKey({}, KEY_LEFTCTRL, true);
    input.onPointerButton({}, BTN_LEFT, true);
    ASSERT_FALSE(client.buttons.empty());
    ASSERT_EQ(std::make_pair(input::ButtonFlags(input::RightButton), true), client.buttons.back());

    lockFully();
    input.onPointerButton({}, BTN_LEFT, false);
    input.onKey({}, KEY_LEFTCTRL, false);
    ASSERT_TRUE(controller.requestUnlock());
    controller.notifyFramePresented();

    EXPECT_EQ(nullptr, seat.pointerGrabSurface());
    input.onPointerMotion({}, base::PointF(1, 1));
    client.buttons.clear();
    input.onPointerButton({}, BTN_LEFT, true);
    input.onPointerButton({}, BTN_LEFT, false);
    const std::vector<std::pair<input::ButtonFlags, bool>> expected = {
            {input::LeftButton, true}, {input::LeftButton, false}};
    EXPECT_EQ(expected, client.buttons);
}

TEST_F(SessionLockTest, ACaptureHoldingTheKeyboardAcrossTheLockDoesNotStrandIt)
{
    wm::Window *application = makeWindow(base::Rect(100, 100, 400, 300));
    wm::Window *capture = makeWindow(base::Rect(0, 0, 1000, 800));
    manager.activation()->activate(application);
    manager.setKeyboardGrab(capture);
    ASSERT_EQ(capture->rootSurface(), seat.focusedSurface());

    lockFully();
    ASSERT_TRUE(controller.requestUnlock());
    controller.notifyFramePresented();

    EXPECT_NE(nullptr, seat.focusedSurface());
}

TEST_F(SessionLockTest, NoWindowMayBeActivatedWhileLocked)
{
    wm::Window *window = makeWindow(base::Rect(100, 100, 400, 300));
    manager.activation()->activate(window);
    ASSERT_EQ(window, manager.activation()->activeWindow());

    lockFully();

    EXPECT_EQ(nullptr, manager.activation()->activeWindow());
    EXPECT_FALSE(manager.activation()->canActivate(window));
    manager.activation()->activate(window);
    EXPECT_EQ(nullptr, manager.activation()->activeWindow());

    wm::Window *late = makeWindow(base::Rect(200, 200, 300, 200));
    manager.activation()->activate(late);
    EXPECT_EQ(nullptr, manager.activation()->activeWindow());
}

TEST_F(SessionLockTest, ActivationComesBackOnlyAfterTheSessionIsActiveAgain)
{
    wm::Window *window = makeWindow(base::Rect(100, 100, 400, 300));
    lockFully();
    ASSERT_TRUE(controller.requestUnlock());

    manager.activation()->activate(window);
    EXPECT_EQ(nullptr, manager.activation()->activeWindow());

    controller.notifyFramePresented();
    manager.activation()->activate(window);
    EXPECT_EQ(window, manager.activation()->activeWindow());
}

TEST_F(SessionLockTest, UnlockingGivesTheKeyboardBackToTheWindowThatHadIt)
{
    wm::Window *first = makeWindow(base::Rect(100, 100, 400, 300));
    wm::Window *second = makeWindow(base::Rect(200, 200, 400, 300));
    manager.activation()->activate(second);
    manager.activation()->activate(first);

    lockFully();
    ASSERT_TRUE(controller.requestUnlock());
    EXPECT_EQ(nullptr, manager.activation()->activeWindow());
    controller.notifyFramePresented();
    EXPECT_EQ(first, manager.activation()->activeWindow());
    EXPECT_EQ(first->rootSurface(), seat.focusedSurface());

    // One that closed while locked leaves the keyboard to the topmost window.
    lockFully();
    manager.destroyWindow(first);
    ASSERT_TRUE(controller.requestUnlock());
    controller.notifyFramePresented();
    EXPECT_EQ(second, manager.activation()->activeWindow());

    // Nothing active before the lock, nothing active after it.
    manager.activation()->activate(nullptr);
    lockFully();
    ASSERT_TRUE(controller.requestUnlock());
    controller.notifyFramePresented();
    EXPECT_EQ(nullptr, manager.activation()->activeWindow());
}

TEST_F(SessionLockTest, UnlockingNeverTakesTheUserToAnotherDesk)
{
    wm::DesksController *desks = manager.desks();
    wm::Window *elsewhere = makeWindow(base::Rect(100, 100, 400, 300));
    manager.activation()->activate(elsewhere);
    wm::Desk *second = desks->newDesk(wm::DesksCreationRemovalSource::Keyboard);
    desks->activateDesk(second, wm::DesksSwitchSource::DeskSwitchShortcut);
    wm::Window *here = makeWindow(base::Rect(200, 200, 400, 300));
    manager.activation()->activate(here);

    // The window that had the keyboard closes while locked; the one left on
    // another desk is not the topmost window the keyboard falls back to.
    lockFully();
    manager.destroyWindow(here);
    ASSERT_TRUE(controller.requestUnlock());
    controller.notifyFramePresented();
    EXPECT_EQ(second, desks->activeDesk());
    EXPECT_EQ(nullptr, manager.activation()->activeWindow());
    EXPECT_FALSE(elsewhere->isVisible());
}

TEST_F(SessionLockTest, AnOutputRemovedMidTransitionCannotDeadlockTheBarrier)
{
    controller.setOutputs({{kPrimaryName, kPrimary}, {kSecondaryName, kSecondary}});
    ASSERT_TRUE(controller.requestLock());
    controller.notifyOutputPresented(kPrimaryName);
    ASSERT_EQ(SessionState::Locking, controller.state());

    controller.setOutputs({{kPrimaryName, kPrimary}});

    EXPECT_EQ(SessionState::Locked, controller.state());
}

TEST_F(SessionLockTest, LosingEveryOutputMidTransitionWaitsForOneToPresentAgain)
{
    ASSERT_TRUE(controller.requestLock());
    controller.setOutputs({});
    EXPECT_EQ(SessionState::Locking, controller.state());
    controller.notifyFramePresented();
    EXPECT_EQ(SessionState::Locking, controller.state());

    controller.setOutputs({{kSecondaryName, kSecondary}});
    EXPECT_EQ(SessionState::Locking, controller.state());
    controller.notifyOutputPresented(kSecondaryName);
    EXPECT_EQ(SessionState::Locked, controller.state());
}

TEST_F(SessionLockTest, AnOutputThatPresentedBeforeTheLayoutWasClearedMustPresentAgain)
{
    controller.setOutputs({{kPrimaryName, kPrimary}, {kSecondaryName, kSecondary}});
    ASSERT_TRUE(controller.requestLock());
    controller.notifyOutputPresented(kPrimaryName);
    controller.setOutputs({});
    controller.setOutputs({{kPrimaryName, kPrimary}, {kSecondaryName, kSecondary}});
    controller.notifyOutputPresented(kSecondaryName);
    EXPECT_EQ(SessionState::Locking, controller.state());
    controller.notifyOutputPresented(kPrimaryName);
    EXPECT_EQ(SessionState::Locked, controller.state());
}

TEST_F(SessionLockTest, UnlockingAlsoSurvivesLosingTheOutputItWaitsFor)
{
    controller.setOutputs({{kPrimaryName, kPrimary}, {kSecondaryName, kSecondary}});
    lockFully();
    ASSERT_TRUE(controller.requestUnlock());
    controller.notifyOutputPresented(kPrimaryName);
    ASSERT_EQ(SessionState::Unlocking, controller.state());

    controller.setOutputs({{kPrimaryName, kPrimary}});

    EXPECT_EQ(SessionState::Active, controller.state());
}

TEST_F(SessionLockTest, LockingIsolatesBothSelectionsAndUnlockingRestoresThem)
{
    auto copied = std::make_shared<TestDataSource>();
    copied->offer("text/plain");
    auto highlighted = std::make_shared<TestDataSource>();
    highlighted->offer("text/plain");
    ASSERT_TRUE(seat.selection()->setSource(copied));
    ASSERT_TRUE(seat.primarySelection()->setSource(highlighted));

    lockFully();

    EXPECT_TRUE(seat.selection()->isIsolated());
    EXPECT_TRUE(seat.primarySelection()->isIsolated());
    EXPECT_EQ(nullptr, seat.selection()->source());
    EXPECT_EQ(nullptr, seat.primarySelection()->source());

    auto typed = std::make_shared<TestDataSource>();
    typed->offer("text/plain");
    EXPECT_FALSE(seat.selection()->setSource(typed));

    ASSERT_TRUE(controller.requestUnlock());
    controller.notifyFramePresented();
    ASSERT_EQ(SessionState::Active, controller.state());

    EXPECT_FALSE(seat.selection()->isIsolated());
    EXPECT_EQ(copied, seat.selection()->source());
    EXPECT_EQ(highlighted, seat.primarySelection()->source());
}

TEST_F(SessionLockTest, CaptureAndDirectScanoutAreRefusedWhileBlocked)
{
    EXPECT_TRUE(controller.allowsCapture());
    EXPECT_TRUE(controller.allowsDirectScanout());

    ASSERT_TRUE(controller.requestLock());
    EXPECT_FALSE(controller.allowsCapture());
    EXPECT_FALSE(controller.allowsDirectScanout());

    controller.notifyFramePresented();
    EXPECT_FALSE(controller.allowsCapture());
    EXPECT_FALSE(controller.allowsDirectScanout());

    ASSERT_TRUE(controller.requestUnlock());
    EXPECT_FALSE(controller.allowsCapture());
    EXPECT_FALSE(controller.allowsDirectScanout());

    controller.notifyFramePresented();
    EXPECT_TRUE(controller.allowsCapture());
    EXPECT_TRUE(controller.allowsDirectScanout());
}

TEST_F(SessionLockTest, TheScanoutPlannerIsSkippedWhileBlocked)
{
    auto *node = static_cast<scene::SurfaceNode *>(surfaceContainer->addChild(
            std::make_unique<scene::SurfaceNode>("fullscreen")));
    node->setGeometry(kPrimary);
    node->setTexture(std::make_shared<TestTexture>(kPrimary.size()));
    node->setScanoutBuffer(std::make_shared<TestScanoutBuffer>(kPrimary.size()));
    node->setOpaque(true);

    const auto plan = [this] {
        if (!controller.allowsDirectScanout())
            return scene::OverlayPlan();
        return scene::proposeOverlays(*surfaceContainer, kPrimary.size(), 1);
    };

    ASSERT_TRUE(plan().hasDirectScanout());

    ASSERT_TRUE(controller.requestLock());
    EXPECT_FALSE(plan().hasDirectScanout());
    controller.notifyFramePresented();
    EXPECT_FALSE(plan().hasDirectScanout());

    ASSERT_TRUE(controller.requestUnlock());
    EXPECT_FALSE(plan().hasDirectScanout());
    controller.notifyFramePresented();
    EXPECT_TRUE(plan().hasDirectScanout());
}

TEST_F(SessionLockTest, EveryOutputGetsItsOwnShield)
{
    secure.setOutputs({{kPrimaryName, kPrimary}, {kSecondaryName, kSecondary}});
    lockFully();

    const auto &shields = secure.shieldContainer()->children();
    ASSERT_EQ(2u, shields.size());
    EXPECT_EQ(kPrimary, shields[0]->geometry());
    EXPECT_EQ(kSecondary, shields[1]->geometry());
}

TEST_F(SessionLockTest, LockedWaitsForEveryOutputToPresent)
{
    secure.setOutputs({{kPrimaryName, kPrimary}, {kSecondaryName, kSecondary}});
    ASSERT_TRUE(controller.requestLock());

    controller.notifyOutputPresented(kPrimaryName);
    EXPECT_EQ(SessionState::Locking, controller.state());

    controller.notifyOutputPresented(kSecondaryName);
    EXPECT_EQ(SessionState::Locked, controller.state());
}

TEST_F(SessionLockTest, AnOutputAppearingMidTransitionIsWaitedForToo)
{
    ASSERT_TRUE(controller.requestLock());
    controller.notifyOutputPresented(kPrimaryName);
    ASSERT_EQ(SessionState::Locked, controller.state());

    ASSERT_TRUE(controller.requestUnlock());
    secure.setOutputs({{kPrimaryName, kPrimary}, {kSecondaryName, kSecondary}});
    controller.notifyOutputPresented(kPrimaryName);
    EXPECT_EQ(SessionState::Unlocking, controller.state());

    controller.notifyOutputPresented(kSecondaryName);
    EXPECT_EQ(SessionState::Active, controller.state());
}

TEST_F(SessionLockTest, AnOutputConnectedWhileLockedStartsBlack)
{
    lockFully();

    secure.setOutputs({{kPrimaryName, kPrimary}, {kSecondaryName, kSecondary}});

    ASSERT_EQ(2u, secure.shieldContainer()->children().size());
    EXPECT_EQ(kSecondary, secure.shieldContainer()->children()[1]->geometry());
    EXPECT_EQ(nullptr, surfaceAt(kSecondary.center()));
}

TEST_F(SessionLockTest, TheSecureContainerIsInertWhileTheSessionIsActive)
{
    secure.setOutputs({{kPrimaryName, kPrimary}, {kSecondaryName, kSecondary}});
    EXPECT_FALSE(secure.node()->isVisible());
    EXPECT_EQ(surfaceContainer, input::inputRootFor(&compositor, &controller));

    lockFully();
    EXPECT_TRUE(secure.node()->isVisible());
    EXPECT_EQ(nullptr, input::inputRootFor(&compositor, &controller));
}

TEST_F(SessionLockTest, OrdinaryClientsReceiveNoInputAndHeldAuthKeysDoNotLeakAfterUnlock)
{
    auto *surface = makeSurface({400, 300});
    auto delegate = std::make_unique<ClientKeyboard>();
    auto *keys = delegate.get();
    input::Keyboard keyboard(std::move(delegate), &seat);
    ClientPointer pointerDelegate;
    input::Pointer pointer(&pointerDelegate, &seat);
    input::InputManager input(&seat);
    seat.setFocusedSurface(surface);
    input.onPointerMotionAbsolute({}, {20, 20});
    input.onKey({}, KEY_A, true);
    input.onKey({}, KEY_A, false);
    ASSERT_EQ(2, keys->keys);
    ASSERT_GT(pointerDelegate.events, 0);
    lockFully();
    pointerDelegate.events = 0;
    keys->keys = 0;
    input.onKey({}, KEY_B, true);
    input.onPointerMotion({}, {10, 10});
    input.onPointerButton({}, BTN_LEFT, true);
    input.onPointerButton({}, BTN_LEFT, false);
    input.onPointerAxis({}, {0, 10}, true);
    input.onPointerAxisStop({});
    EXPECT_EQ(0, keys->keys);
    EXPECT_EQ(0, pointerDelegate.events);
    EXPECT_TRUE(seat.clientPressedKeys().empty());
    ASSERT_TRUE(controller.requestUnlock());
    controller.notifyFramePresented();
    seat.setFocusedSurface(surface);
    EXPECT_TRUE(keys->entered.empty());
    input.onKey({}, KEY_B, false);
    EXPECT_EQ(0, keys->keys);
}

TEST_F(SessionLockTest, CapsLockToggledWhileLockedReachesClientsAndTheLights)
{
    input::InputManager input(&seat);
    lockFully();
    input.onKey({}, KEY_CAPSLOCK, true);
    input.onKey({}, KEY_CAPSLOCK, false);
    EXPECT_EQ(input::KeyboardLed::CapsLock, seat.clientXkbTracker()->leds());
    ASSERT_TRUE(controller.requestUnlock());
    controller.notifyFramePresented();
    // Toggled off again after the unlock, both states agree it is off.
    input.onKey({}, KEY_CAPSLOCK, true);
    input.onKey({}, KEY_CAPSLOCK, false);
    EXPECT_EQ(0u, seat.clientXkbTracker()->leds());
    EXPECT_EQ(0u, seat.xkbTracker()->leds());
}

TEST_F(SessionLockTest, ModifiersHeldForTheLockShortcutAreReleasedWhileLocked)
{
    seat.setShortcutConvention(input::ShortcutConvention::Command);
    input::InputManager input(&seat);
    input.onKey({}, KEY_LEFTCTRL, true);
    input.onKey({}, KEY_LEFTMETA, true);
    lockFully();
    input.onKey({}, KEY_LEFTCTRL, false);
    input.onKey({}, KEY_LEFTMETA, false);
    EXPECT_TRUE(seat.clientPressedKeys().empty());
    ASSERT_TRUE(controller.requestUnlock());
    controller.notifyFramePresented();
    // A later Command shortcut clears a stuck Command and would leave Control.
    input.onKey({}, KEY_LEFTMETA, true);
    input.onKey({}, KEY_LEFTMETA, false);
    EXPECT_EQ(uint32_t(BTN_LEFT), seat.keyTranslator()->translateButton(BTN_LEFT, true));
    EXPECT_EQ(uint32_t(BTN_LEFT), seat.keyTranslator()->translateButton(BTN_LEFT, false));
}

TEST_F(SessionLockTest, AMetaHeldForThePcLockShortcutIsReleasedWhileLocked)
{
    auto *surface = makeSurface({400, 300});
    auto delegate = std::make_unique<ClientKeyboard>();
    auto *keys = delegate.get();
    input::Keyboard keyboard(std::move(delegate), &seat);
    input::InputManager input(&seat);
    seat.setFocusedSurface(surface);
    // The PC convention sends Meta at once, so a client holds it at the lock.
    input.onKey({}, KEY_LEFTMETA, true);
    ASSERT_FALSE(seat.clientPressedKeys().empty());
    lockFully();
    input.onKey({}, KEY_LEFTMETA, false);
    EXPECT_TRUE(seat.clientPressedKeys().empty());
    ASSERT_TRUE(controller.requestUnlock());
    controller.notifyFramePresented();
    seat.setFocusedSurface(surface);
    EXPECT_TRUE(keys->entered.empty());
    keys->keys = 0;
    input.onKey({}, KEY_A, true);
    EXPECT_EQ(1, keys->keys);
    EXPECT_EQ(1u, seat.clientPressedKeys().size());
    EXPECT_TRUE(seat.clientPressedKeys().contains(KEY_A));
}

TEST_F(SessionLockTest, AKeyHeldAcrossTheLockIsPressedAgainAfterUnlock)
{
    auto *surface = makeSurface({400, 300});
    auto delegate = std::make_unique<ClientKeyboard>();
    auto *keys = delegate.get();
    input::Keyboard keyboard(std::move(delegate), &seat);
    input::InputManager input(&seat);
    seat.setFocusedSurface(surface);
    input.onKey({}, KEY_A, true);
    lockFully();
    input.onKey({}, KEY_A, false);
    ASSERT_TRUE(controller.requestUnlock());
    controller.notifyFramePresented();
    seat.setFocusedSurface(surface);
    keys->keys = 0;
    input.onKey({}, KEY_A, true);
    input.onKey({}, KEY_A, false);
    EXPECT_EQ(2, keys->keys);
}

TEST_F(SessionLockTest, ATranslatedKeyHeldAcrossTheLockLeavesNoOtherKeySuppressed)
{
    seat.setShortcutConvention(input::ShortcutConvention::Command);
    auto *surface = makeSurface({400, 300});
    auto delegate = std::make_unique<ClientKeyboard>();
    auto *keys = delegate.get();
    input::Keyboard keyboard(std::move(delegate), &seat);
    input::InputManager input(&seat);
    seat.setFocusedSurface(surface);
    // Command+Left reaches the client as Ctrl+Left.
    const auto apple = input::KeyboardModifierLayout::Apple;
    input.onKey({}, KEY_LEFTMETA, true, apple);
    input.onKey({}, KEY_LEFT, true, apple);
    ASSERT_TRUE(seat.clientPressedKeys().contains(KEY_LEFTCTRL));
    ASSERT_TRUE(seat.clientPressedKeys().contains(KEY_LEFT));
    lockFully();
    input.onKey({}, KEY_LEFT, false, apple);
    input.onKey({}, KEY_LEFTMETA, false, apple);
    EXPECT_TRUE(seat.clientPressedKeys().empty());
    ASSERT_TRUE(controller.requestUnlock());
    controller.notifyFramePresented();
    seat.setFocusedSurface(surface);
    // Neither the Ctrl it went out as nor the key stays suppressed.
    keys->keys = 0;
    input.onKey({}, KEY_LEFTCTRL, true);
    input.onKey({}, KEY_LEFT, true);
    input.onKey({}, KEY_LEFT, false);
    input.onKey({}, KEY_LEFTCTRL, false);
    EXPECT_EQ(4, keys->keys);
    EXPECT_TRUE(seat.clientPressedKeys().empty());
}

TEST_F(SessionLockTest, InitialLoginIsolatesBothSelections)
{
    session::SecureContainer greeterScene(greeterNode);
    session::SessionController greeter(&greeterScene, &teardown, SessionState::Login);
    EXPECT_TRUE(seat.selection()->isIsolated());
    EXPECT_TRUE(seat.primarySelection()->isIsolated());
    EXPECT_EQ(SessionState::Login, greeter.state());
}

TEST_F(SessionLockTest, ReusedOutputNameMustPresentAgainAfterGeometryChanges)
{
    controller.setOutputs({{kPrimaryName, kPrimary}, {kSecondaryName, kSecondary}});
    ASSERT_TRUE(controller.requestLock());
    controller.notifyOutputPresented(kPrimaryName);
    controller.setOutputs({{kPrimaryName, base::Rect(0, 0, 640, 480)},
                           {kSecondaryName, kSecondary}});
    controller.notifyOutputPresented(kSecondaryName);
    EXPECT_EQ(SessionState::Locking, controller.state());
    controller.notifyOutputPresented(kPrimaryName);
    EXPECT_EQ(SessionState::Locked, controller.state());
}

TEST_F(SessionLockTest, ObserversReceiveInitialLoginSecurityPolicy)
{
    class Policy : public session::SessionObserver
    {
    public:
        bool blocked = false;

        void onSessionStateChanged(SessionState state) override
        {
            blocked = session::isSessionBlocked(state);
        }
    } policy;

    session::SecureContainer greeterScene(greeterNode);
    session::SessionController greeter(&greeterScene, nullptr, SessionState::Login);
    greeter.addObserver(&policy);
    EXPECT_TRUE(policy.blocked);
    greeter.removeObserver(&policy);
}

double steadyMs()
{
    return std::chrono::duration<double, std::milli>(
                   std::chrono::steady_clock::now().time_since_epoch())
            .count();
}

TEST_F(SessionLockTest, TheLockFadesInOverTheDesktopAndIsSettledOnlyAtItsEnd)
{
    ASSERT_TRUE(controller.requestLock());
    const double first = steadyMs();
    // Moving in its first frame, not showing where it starts.
    const auto start = secure.sample(first, 16);
    EXPECT_GT(start.backdrop, 0);
    EXPECT_LT(start.backdrop, 0.1);
    EXPECT_LT(start.panelScale, 1);
    EXPECT_FALSE(secure.isSettled(first));

    const double middle = first + session::SecureContainer::kCoverDurationMs / 2.0;
    const auto halfway = secure.sample(middle, 16);
    EXPECT_GT(halfway.backdrop, start.backdrop);
    EXPECT_LT(halfway.backdrop, 1);
    EXPECT_FALSE(secure.isSettled(middle));

    const double end = first - 16 + session::SecureContainer::kCoverDurationMs;
    EXPECT_TRUE(secure.isSettled(end));
    const auto covered = secure.sample(end, 16);
    EXPECT_EQ(1, covered.backdrop);
    EXPECT_EQ(1, covered.panelOpacity);
    EXPECT_EQ(1, covered.panelScale);
}

TEST_F(SessionLockTest, TheUnlockLiftsThePanelAndClearsTheBackdrop)
{
    lockFully();
    ASSERT_TRUE(controller.requestUnlock());
    const double first = steadyMs();
    secure.sample(first, 16);
    const auto lifting = secure.sample(first + 100, 16);
    EXPECT_GT(lifting.panelScale, 1);
    EXPECT_LT(lifting.panelOpacity, lifting.backdrop);
    EXPECT_FALSE(secure.isSettled(first + 100));

    const double end = first - 16 + session::SecureContainer::kUncoverDurationMs;
    EXPECT_TRUE(secure.isSettled(end));
    EXPECT_EQ(0, secure.sample(end, 16).backdrop);
}

TEST_F(SessionLockTest, ALockNoScreenDrewInTimeIsAlreadyOver)
{
    ASSERT_TRUE(controller.requestLock());
    const double late = steadyMs() + 1000;
    EXPECT_EQ(1, secure.sample(late, 16).backdrop);
    EXPECT_TRUE(secure.isSettled(late));
}

TEST_F(SessionLockTest, AFinishedTransitionDrawsItsEndAtOnce)
{
    ASSERT_TRUE(controller.requestLock());
    EXPECT_FALSE(secure.isSettled(steadyMs()));
    secure.finishTransition();
    const double now = steadyMs();
    EXPECT_TRUE(secure.isSettled(now));
    EXPECT_EQ(1, secure.sample(now, 16).backdrop);
}

TEST_F(SessionLockTest, TheLoginScreenIsCoveredFromItsFirstFrame)
{
    session::SecureContainer greeterScene(greeterNode);
    session::SessionController greeter(&greeterScene, nullptr, SessionState::Login);
    const double now = steadyMs();
    EXPECT_TRUE(greeterScene.isSettled(now));
    EXPECT_EQ(1, greeterScene.sample(now, 16).backdrop);
}

} // namespace
