// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Window manager. Behaviour modelled on chromium/src/ash/wm
// (window_state_unittest.cc, window_resizer edge cases,
// workspace_controller_unittest.cc auto placement, window_positioner_unittest.cc,
// window_modality_controller_unittest.cc) and
// ui/wm/core/focus_controller_unittest.cc, rewritten against the Lingmo
// abstractions. No DRM, no GL, no Wayland client.

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <linux/input-event-codes.h>

#include "base/auto_reset.h"
#include "base/geometry.h"
#include "compositor/compositor.h"
#include "compositor/sub_surface.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "input/session_gate.h"
#include "input/touch.h"
#include "input/touch_delegate.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "scene/snap_preview_node.h"
#include "scene/surface_node.h"
#include "scene/window_animation.h"
#include "scene/window_visual_node.h"
#include "shm_test_util.h"
#include "wm/activation.h"
#include "wm/anchored_placement.h"
#include "wm/placement.h"
#include "wm/quit_all.h"
#include "wm/snap_drag_controller.h"
#include "wm/stacking.h"
#include "wm/toplevel_event_handler.h"
#include "wm/transient_manager.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_modality_controller.h"
#include "wm/window_positioner.h"
#include "wm/window_resizer.h"
#include "wm/window_snap.h"
#include "wm/window_state.h"
#include "wm/wm_event.h"

using namespace Lingmo;
using namespace Lingmo::Wm;

namespace {

input::Timestamp at(int ms)
{
    return input::Timestamp(std::chrono::milliseconds(ms));
}

const base::Rect kOutput(0, 0, 1000, 800);

class WmTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node *surfaceContainer =
            scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    compositor::Compositor compositor {surfaceContainer};
    testsupport::FakeRenderer renderer;
    input::Seat seat {&compositor};
    input::InputManager input {&seat};
    WindowManager manager {&compositor, &seat};

    std::vector<std::unique_ptr<compositor::Surface>> surfaces;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 20);

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
        input.setBounds(kOutput.size());
        input.setEventSink(&manager);
    }

    void TearDown() override { seat.shutdown(); }

    // A mapped surface of `size`, owned by the fixture.
    compositor::Surface *makeSurface(const base::Size &size)
    {
        surfaces.push_back(compositor.createSurface());
        compositor::Surface *surface = surfaces.back().get();
        auto buffer = pool->createBuffer(size, 0, size.width() * 4, render::PixelFormat::Bgrx8888,
                                         nullptr);
        surface->attach(buffer, {});
        surface->damage(base::Rect(base::Point(), size));
        surface->commit();
        compositor.updateTextures(renderer);
        return surface;
    }

    Window *makeWindow(const base::Rect &bounds)
    {
        Window *window =
                manager.createWindow(makeSurface(bounds.size()), {.requestedSize = bounds.size()});
        window->setBounds(bounds);
        return window;
    }

    Window *mapManagedWindow(const base::Size &size, const std::string &applicationId = "app")
    {
        return manager.createWindow(
                makeSurface(size),
                {.requestedSize = size, .applicationId = applicationId, .positionManaged = true});
    }
};

////////////////////////////////////////////////////////////////////////////////
// Applications: Quit, Hide and Hide Others

TEST_F(WmTest, QuitClosesTheApplicationsWindowsAndTerminatesWhatStaysBehind)
{
    std::vector<std::string> closed;
    const auto make = [&](const std::string &name, const std::string &applicationId, int64_t process,
                          Window *parent = nullptr) {
        return manager.createWindow(makeSurface({200, 100}),
                                    {.requestedSize = {200, 100},
                                     .title = name,
                                     .applicationId = applicationId,
                                     .processId = process,
                                     .transientParent = parent,
                                     .closeCallback = [&closed, name] {
                                         closed.push_back(name);
                                     }});
    };
    Window *browser = make("browser", "browser", 10);
    Window *browser2 = make("browser 2", "Browser", 11);
    Window *prompt = make("prompt", "browser", 10, browser);
    // An installed web app runs in the browser's process but is its own
    // application.
    make("web app", "web-app", 10);

    std::vector<int64_t> terminated;
    manager.setTerminateProcess([&terminated](int64_t pid) {
        terminated.push_back(pid);
    });
    manager.setQuitGracePeriod(std::chrono::milliseconds(0));

    // From a dialog, the application is its parent's. Dialogs are left alone,
    // so a prompt is never answered for the user.
    manager.quitApplication(prompt);
    EXPECT_EQ(std::vector<std::string>({"browser", "browser 2"}),
              closed);
    // The windows are still up after the grace period: nothing is terminated.
    test::processPendingEvents();
    EXPECT_TRUE(terminated.empty());

    // Once they are gone, a process with no window left is terminated; the one
    // still running the web app is not.
    for (Window *window : {prompt, browser2, browser})
        manager.destroyWindow(window);
    closed.clear();
    Window *web = manager.windows().back();
    ASSERT_EQ("web-app", web->applicationId());
    Window *other = make("other", "browser", 11);
    manager.quitApplication(other);
    manager.destroyWindow(other);
    test::processPendingEvents();
    EXPECT_EQ(std::vector<int64_t>({11}), terminated);
}

class QuitAllTest : public WmTest
{
protected:
    int64_t now = 0;
    QuitAll quitAll {&manager, [this] {
                         return now;
                     }};
    std::vector<std::string> closed;
    std::optional<QuitAll::Result> result;

    Window *make(const std::string &applicationId, int64_t process, Window *parent = nullptr)
    {
        return manager.createWindow(makeSurface({200, 100}),
                                    {.requestedSize = {200, 100},
                                     .applicationId = applicationId,
                                     .processId = process,
                                     .transientParent = parent,
                                     .closeCallback = [this, applicationId] {
                                         closed.push_back(applicationId);
                                     }});
    }

    void start()
    {
        ASSERT_TRUE(quitAll.start([this](const QuitAll::Result &done) {
            result = done;
        }));
    }

    void settle()
    {
        test::processPendingEvents();
        quitAll.update();
    }
};

TEST_F(QuitAllTest, EndsOnceTheLastApplicationWindowIsGone)
{
    Window *editor = make("kate", 20);
    Window *player = make("player", 21);
    // Shell windows are not the user's to quit.
    make("desktop", 22)->setRole(WindowRole::Desktop);
    start();
    EXPECT_EQ(std::vector<std::string>({"kate", "player"}), closed);
    EXPECT_FALSE(quitAll.start({})) << "one request at a time";

    manager.destroyWindow(player);
    settle();
    EXPECT_FALSE(result);
    manager.destroyWindow(editor);
    settle();
    ASSERT_TRUE(result);
    EXPECT_EQ(QuitAll::Outcome::Quit, result->outcome);
    EXPECT_FALSE(quitAll.isRunning());
}

TEST_F(QuitAllTest, APromptIsWaitedForHoweverLongItTakes)
{
    Window *editor = make("kate", 20);
    start();
    Window *prompt = make("kate", 20, editor);
    now = 10 * 60 * 1000;
    settle();
    EXPECT_FALSE(result);
    EXPECT_EQ(1u, closed.size()) << "the prompt is the user's to answer";

    // Saved: the prompt goes, and the window soon after.
    manager.destroyWindow(prompt);
    now += QuitAll::kCancelledAfterMs / 2;
    settle();
    EXPECT_FALSE(result);
    manager.destroyWindow(editor);
    settle();
    ASSERT_TRUE(result);
    EXPECT_EQ(QuitAll::Outcome::Quit, result->outcome);
}

TEST_F(QuitAllTest, AWindowKeptAfterItsPromptCancels)
{
    Window *editor = make("kate", 20);
    make("player", 21);
    start();
    Window *prompt = make("kate", 20, editor);
    now = 1000;
    settle();
    manager.destroyWindow(prompt);
    now += QuitAll::kCancelledAfterMs;
    settle();
    EXPECT_FALSE(result);
    now += 1;
    settle();
    ASSERT_TRUE(result);
    EXPECT_EQ(QuitAll::Outcome::Cancelled, result->outcome);
    EXPECT_EQ("kate", result->applicationId);
}

TEST_F(QuitAllTest, AWindowThatNeitherClosesNorAsksStoppedResponding)
{
    make("frozen", 20);
    start();
    now = QuitAll::kUnresponsiveAfterMs;
    settle();
    EXPECT_FALSE(result);
    now += 1;
    settle();
    ASSERT_TRUE(result);
    EXPECT_EQ(QuitAll::Outcome::Unresponsive, result->outcome);
    EXPECT_EQ("frozen", result->applicationId);
}

TEST_F(QuitAllTest, AWindowOpenedMeanwhileIsAskedOnce)
{
    Window *editor = make("kate", 20);
    start();
    Window *second = make("kate", 20);
    Window *other = make("player", 21);
    settle();
    settle();
    // The second editor window counts as asked with the first: Quit closed both.
    EXPECT_EQ(std::vector<std::string>({"kate", "kate",
                                    "player"}),
              closed);
    for (Window *window : {editor, second, other})
        manager.destroyWindow(window);
    settle();
    ASSERT_TRUE(result);
    EXPECT_EQ(QuitAll::Outcome::Quit, result->outcome);
}

TEST_F(WmTest, HideAndHideOthersActOnWholeApplications)
{
    const auto make = [this](int64_t process, const std::string &applicationId) {
        return manager.createWindow(makeSurface({200, 100}),
                                    {.requestedSize = {200, 100},
                                     .applicationId = applicationId,
                                     .processId = process});
    };
    Window *editor = make(10, "editor");
    Window *editor2 = make(11, "editor");
    // Same process, another application id: another application.
    Window *viewer = make(10, "viewer");
    Window *tool = make(0, "tool");
    Window *tool2 = make(0, "Tool");

    manager.hideOtherApplications(editor);
    EXPECT_FALSE(editor->isHidden());
    EXPECT_FALSE(editor2->isHidden());
    EXPECT_TRUE(viewer->isHidden());
    EXPECT_TRUE(tool->isHidden());

    for (Window *window : {viewer, tool})
        window->setHidden(false);
    manager.hideApplication(tool);
    EXPECT_TRUE(tool2->isHidden());
    EXPECT_FALSE(viewer->isHidden());
    manager.hideApplication(editor2);
    EXPECT_TRUE(editor->isHidden());

    // With no application to keep, every one is hidden.
    viewer->setHidden(false);
    manager.hideOtherApplications(nullptr);
    EXPECT_TRUE(viewer->isHidden());

    // Showing an application again shows only its windows.
    manager.unhideApplication(editor);
    EXPECT_FALSE(editor->isHidden());
    EXPECT_FALSE(editor2->isHidden());
    EXPECT_TRUE(viewer->isHidden());
}

////////////////////////////////////////////////////////////////////////////////
// Unresponsive windows

TEST_F(WmTest, CloseAsksAResponsiveWindowAndKillsAnUnresponsiveOne)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    int closes = 0;
    int pings = 0;
    int kills = 0;
    window->setCloseCallback([&] {
        ++closes;
    });
    window->setPingCallback([&] {
        ++pings;
    });
    window->setKillCallback([&] {
        ++kills;
    });

    window->requestClose();
    EXPECT_EQ(1, closes);
    EXPECT_EQ(1, pings);
    EXPECT_EQ(0, kills);

    window->setResponsive(false);
    window->requestClose();
    EXPECT_EQ(1, closes);
    EXPECT_EQ(1, kills);

    window->setResponsive(true);
    window->requestClose();
    EXPECT_EQ(2, closes);
    EXPECT_EQ(1, kills);
}

TEST_F(WmTest, WaitingOnAnUnresponsiveWindowOffersForceQuit)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    int closes = 0;
    int kills = 0;
    window->setCloseCallback([&] {
        ++closes;
    });
    window->setPingCallback([] {});
    window->setKillCallback([&] {
        ++kills;
    });
    int prompts = 0;
    manager.setNotRespondingPrompt([&](Window *prompted) {
        EXPECT_EQ(window, prompted);
        ++prompts;
        return true;
    });
    manager.activation()->activate(window);

    // Nothing is offered while the window answers.
    window->ping();
    input.onKey(at(1), KEY_A, true);
    input.onKey(at(2), KEY_A, false);
    window->requestClose();
    EXPECT_EQ(0, prompts);
    EXPECT_EQ(1, closes);

    // A press, a key or a close on a window that stopped answering is the user
    // waiting on it.
    window->setResponsive(false);
    window->ping();
    EXPECT_EQ(1, prompts);
    input.onKey(at(3), KEY_A, true);
    input.onKey(at(4), KEY_A, false);
    EXPECT_EQ(2, prompts);
    window->requestClose();
    EXPECT_EQ(3, prompts);
    EXPECT_EQ(1, closes);
    EXPECT_EQ(0, kills);

    // With nobody to offer it, closing ends the client.
    manager.setNotRespondingPrompt([](Window *) {
        return false;
    });
    window->requestClose();
    EXPECT_EQ(1, kills);
}

TEST_F(WmTest, ForceQuitEndsTheClientAndItsApplication)
{
    Window *window = manager.createWindow(makeSurface({300, 200}),
                                          {.requestedSize = {300, 200}, .processId = 4242});
    int kills = 0;
    window->setKillCallback([&] {
        ++kills;
    });
    std::vector<int64_t> ended;
    manager.setForceQuitProcess([&](int64_t pid) {
        ended.push_back(pid);
    });

    manager.forceQuit(window);
    EXPECT_EQ(1, kills);
    EXPECT_EQ(std::vector<int64_t>({4242}), ended);
}

TEST_F(WmTest, ASheetHangsFromItsParentWhereverItGoes)
{
    Window *parent = makeWindow(base::Rect(100, 100, 400, 300));
    Window *other = makeWindow(base::Rect(600, 100, 300, 200));
    compositor::Surface *surface = makeSurface({200, 80});
    ShellWindowMetadata metadata;
    metadata.role = WindowRole::Sheet;
    metadata.parentWindow = parent->id();
    manager.setShellMetadata(surface, metadata);
    Window *sheet = manager.createWindow(surface, {.requestedSize = {200, 80}});

    // Centred on the parent, and modal for it.
    EXPECT_EQ(parent, sheet->transientParent());
    EXPECT_EQ(sheet, modalTransient(parent));
    EXPECT_EQ(base::Rect(200, 210, 200, 80), sheet->bounds());
    EXPECT_EQ(StackingLayer::Normal, layerFor(sheet));

    // It follows what the parent shows: until the client draws its new size,
    // as a hung one never does, its old contents stay where they were.
    parent->setBounds(base::Rect(300, 200, 500, 300));
    EXPECT_EQ(base::Rect(400, 310, 200, 80), sheet->bounds());
    compositor::Surface *parentSurface = parent->rootSurface();
    parentSurface->attach(pool->createBuffer({500, 300}, 0, 500 * 4,
                                             render::PixelFormat::Bgrx8888, nullptr),
                          {});
    parentSurface->damage(base::Rect(0, 0, 500, 300));
    parentSurface->commit();
    compositor.updateTextures(renderer);
    EXPECT_EQ(base::Rect(450, 310, 200, 80), sheet->bounds());
    sheet->setBounds(base::Rect(sheet->bounds().topLeft(), base::Size(300, 120)));
    EXPECT_EQ(base::Rect(400, 290, 300, 120), sheet->bounds());

    // Right above the parent whichever window is raised.
    manager.raise(other);
    manager.raise(parent);
    ASSERT_EQ(3u, manager.windows().size());
    EXPECT_EQ(parent, manager.windows()[1]);
    EXPECT_EQ(sheet, manager.windows()[2]);

    // Minimized and restored with it.
    parent->minimize();
    EXPECT_FALSE(sheet->isVisible());
    parent->restore();
    EXPECT_TRUE(sheet->isVisible());
}

TEST_F(WmTest, ASheetNeverHangsFromAShellWindow)
{
    Window *panel = makeWindow(base::Rect(0, 0, 800, 30));
    panel->setRole(WindowRole::Panel);
    compositor::Surface *surface = makeSurface({200, 80});
    ShellWindowMetadata metadata;
    metadata.role = WindowRole::Sheet;
    metadata.parentWindow = panel->id();
    manager.setShellMetadata(surface, metadata);
    Window *sheet = manager.createWindow(surface, {.requestedSize = {200, 80}});

    EXPECT_EQ(nullptr, sheet->transientParent());
    EXPECT_FALSE(sheet->isModal());
}

TEST_F(WmTest, ShellWindowsAreNeverPinged)
{
    Window *panel = makeWindow(base::Rect(0, 0, 800, 30));
    panel->setRole(WindowRole::Panel);
    int pings = 0;
    panel->setPingCallback([&] {
        ++pings;
    });
    panel->ping();
    EXPECT_EQ(0, pings);
}

////////////////////////////////////////////////////////////////////////////////
// WindowState

TEST_F(WmTest, NormalToMaximizedUsesTheWorkArea)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));

    window->maximize();

    EXPECT_EQ(WindowStateType::Maximized, window->stateType());
    EXPECT_EQ(kOutput, window->bounds());
    EXPECT_TRUE(window->state()->hasRestoreBounds());
    EXPECT_EQ(base::Rect(100, 100, 300, 200), window->state()->restoreBounds());
}

TEST_F(WmTest, MinimizedGeometryFollowsDockMovementWithoutAnotherIconUpdate)
{
    Window *window = makeWindow({100, 100, 300, 200});
    Window *dock = makeWindow({400, 700, 300, 80});
    const base::Rect icon(30, 10, 48, 48);
    window->setMinimizedGeometry(dock, icon);
    EXPECT_EQ(window->minimizedGeometry(), icon.translated(dock->contentBounds().topLeft()));
    for (const base::Rect bounds :
         {base::Rect(700, 1100, 300, 80), base::Rect(200, 500, 300, 80), base::Rect(-800, 600, 300, 80)}) {
        dock->setBounds(bounds);
        EXPECT_EQ(window->minimizedGeometry(), icon.translated(dock->contentBounds().topLeft()));
        window->minimize();
        EXPECT_EQ(window->minimizedGeometry(), icon.translated(dock->contentBounds().topLeft()));
        window->restore();
    }
    manager.destroyWindow(dock);
    EXPECT_TRUE(window->minimizedGeometry().isEmpty());
    window->setMinimizedGeometry({20, 30, 40, 40});
    EXPECT_EQ(window->minimizedGeometry(), base::Rect(20, 30, 40, 40));
}

TEST_F(WmTest, MaximizedRestoresToItsPreviousBounds)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));

    window->maximize();
    window->restore();

    EXPECT_EQ(WindowStateType::Normal, window->stateType());
    EXPECT_EQ(base::Rect(100, 100, 300, 200), window->bounds());
    // Nothing left to restore to, so the bounds are dropped with the history.
    EXPECT_FALSE(window->state()->hasRestoreBounds());
}

TEST_F(WmTest, NormalToFullscreenUsesTheOutput)
{
    Window *window = makeWindow(base::Rect(10, 20, 300, 200));

    window->setFullscreen(true);

    EXPECT_EQ(WindowStateType::Fullscreen, window->stateType());
    EXPECT_EQ(kOutput, window->bounds());

    window->setFullscreen(false);
    EXPECT_EQ(WindowStateType::Normal, window->stateType());
    EXPECT_EQ(base::Rect(10, 20, 300, 200), window->bounds());
}

TEST_F(WmTest, MaximizeThenFullscreenRestoresBackThroughMaximized)
{
    Window *window = makeWindow(base::Rect(10, 20, 300, 200));

    window->maximize();
    window->setFullscreen(true);
    ASSERT_EQ(WindowStateType::Fullscreen, window->stateType());

    window->restore();
    EXPECT_EQ(WindowStateType::Maximized, window->stateType());
    EXPECT_EQ(kOutput, window->bounds());

    window->restore();
    EXPECT_EQ(WindowStateType::Normal, window->stateType());
    EXPECT_EQ(base::Rect(10, 20, 300, 200), window->bounds());
}

TEST_F(WmTest, FullscreenFromNormalRestoresStraightToNormal)
{
    Window *window = makeWindow(base::Rect(10, 20, 300, 200));

    window->setFullscreen(true);
    window->restore();

    EXPECT_EQ(WindowStateType::Normal, window->stateType());
    EXPECT_EQ(base::Rect(10, 20, 300, 200), window->bounds());
}

TEST_F(WmTest, MinimizeAndRestorePreserveGeometry)
{
    Window *window = makeWindow(base::Rect(40, 50, 300, 200));

    window->minimize();
    EXPECT_EQ(WindowStateType::Minimized, window->stateType());
    EXPECT_FALSE(window->isVisible());

    window->restore();
    EXPECT_EQ(WindowStateType::Normal, window->stateType());
    EXPECT_TRUE(window->isVisible());
    EXPECT_EQ(base::Rect(40, 50, 300, 200), window->bounds());
}

TEST_F(WmTest, RestoreUsesLatestDockGeometryOrOutputFallback)
{
    class SnapshotRenderer : public testsupport::FakeRenderer
    {
    public:
        bool renderToTexture(std::unique_ptr<render::Texture> &texture, const base::Rect &,
                             const std::function<void()> &paint) override
        {
            class Snapshot : public render::Texture
            {
            public:
                base::Size size() const override { return {1, 1}; }
            };

            paint();
            texture = std::make_unique<Snapshot>();
            return true;
        }
    } snapshotRenderer;

    const base::Rect bounds(100, 100, 300, 200);
    Window *window = makeWindow(bounds);
    auto *animation = dynamic_cast<scene::WindowAnimation *>(window->node());
    ASSERT_NE(animation, nullptr);
    animation->finish();
    animation->render(snapshotRenderer);
    window->setMinimizedGeometry({200, 700, 48, 48});

    for (const base::Rect dock : {base::Rect(600, 720, 40, 32), base::Rect()}) {
        window->minimize();
        animation->finish();
        window->setMinimizedGeometry(dock);
        window->restore();
        animation->render(snapshotRenderer);

        const base::Rect target =
                dock.isEmpty() ? base::Rect(centerPixel(kOutput).x(), kOutput.bottom() - 1, 1, 1) : dock;
        const double sx = double(target.width()) / bounds.width();
        const double sy = double(target.height()) / bounds.height();
        const base::Rect expected = base::RectF(target.x() - 64 * sx, target.y() - 64 * sy,
                                      (bounds.width() + 128) * sx, (bounds.height() + 128) * sy)
                                       .toEnclosingRect();
        EXPECT_EQ(snapshotRenderer.calls.back().rect, expected);
        EXPECT_EQ(window->bounds(), bounds);
        animation->finish();
    }
}

TEST_F(WmTest, MinimizingAMaximizedWindowRestoresBackToMaximized)
{
    Window *window = makeWindow(base::Rect(40, 50, 300, 200));

    window->maximize();
    window->minimize();
    window->restore();

    EXPECT_EQ(WindowStateType::Maximized, window->stateType());
    EXPECT_EQ(kOutput, window->bounds());

    window->restore();
    EXPECT_EQ(WindowStateType::Normal, window->stateType());
    EXPECT_EQ(base::Rect(40, 50, 300, 200), window->bounds());
}

TEST_F(WmTest, AMinimizedWindowIsNotRenderedAndIsNotHitTested)
{
    Window *window = makeWindow(base::Rect(0, 0, 300, 200));
    ASSERT_EQ(window, manager.windowAt(base::PointF(50, 50)));
    ASSERT_TRUE(window->node()->isEffectivelyVisible());

    window->minimize();

    EXPECT_FALSE(window->node()->isEffectivelyVisible());
    EXPECT_EQ(nullptr, manager.windowAt(base::PointF(50, 50)));
    // The surface and its buffer are untouched.
    EXPECT_TRUE(window->rootSurface()->isMapped());
    EXPECT_TRUE(window->rootSurface()->hasContents());
}

TEST_F(WmTest, AHiddenWindowLeavesTheScreenAndKeepsItsState)
{
    Window *behind = makeWindow(base::Rect(0, 0, 300, 200));
    Window *window = makeWindow(base::Rect(0, 0, 300, 200));
    window->maximize();
    window->activate();
    ASSERT_EQ(window, manager.activation()->activeWindow());

    window->setHidden(true);

    EXPECT_TRUE(window->isHidden());
    EXPECT_FALSE(window->isVisible());
    EXPECT_FALSE(window->node()->isEffectivelyVisible());
    EXPECT_EQ(behind, manager.windowAt(base::PointF(50, 50)));
    // The focus moves on, as it does for a closed window.
    EXPECT_EQ(behind, manager.activation()->activeWindow());
    EXPECT_EQ(WindowStateType::Maximized, window->stateType());
    EXPECT_TRUE(window->rootSurface()->isMapped());

    // Unhiding shows the window as it was, without activating it.
    window->setHidden(false);
    EXPECT_TRUE(window->isVisible());
    EXPECT_EQ(WindowStateType::Maximized, window->stateType());
    EXPECT_EQ(behind, manager.activation()->activeWindow());
}

TEST_F(WmTest, AHiddenWindowStaysMinimizedUnderneath)
{
    Window *window = makeWindow(base::Rect(0, 0, 300, 200));
    window->minimize();
    window->setHidden(true);
    ASSERT_FALSE(window->isVisible());

    // Leaving the minimized state while hidden does not show the window.
    window->restore();
    EXPECT_EQ(WindowStateType::Normal, window->stateType());
    EXPECT_FALSE(window->isVisible());
    window->minimize();

    // Unhiding a minimized window keeps it off screen for the dock.
    window->setHidden(false);
    EXPECT_EQ(WindowStateType::Minimized, window->stateType());
    EXPECT_FALSE(window->isVisible());

    window->restore();
    EXPECT_TRUE(window->isVisible());
}

TEST_F(WmTest, AMinimizedWindowSurvivesACommit)
{
    Window *window = makeWindow(base::Rect(0, 0, 300, 200));
    window->minimize();

    window->rootSurface()->damage(base::Rect(0, 0, 10, 10));
    window->rootSurface()->commit();

    EXPECT_FALSE(window->node()->isEffectivelyVisible());
}

TEST_F(WmTest, UnmaximizingToWorkAreaSizedRestoreBoundsInsetsThem)
{
    Window *window = makeWindow(kOutput);

    window->maximize();
    window->restore();

    EXPECT_EQ(kOutput.adjusted(10, 10, -10, -10), window->bounds());
}

TEST_F(WmTest, MinimizeAndFullscreenRoundTripsKeepWorkAreaSizedBounds)
{
    Window *window = makeWindow(kOutput);

    window->minimize();
    window->restore();
    EXPECT_EQ(kOutput, window->bounds());

    window->setFullscreen(true);
    window->setFullscreen(false);
    EXPECT_EQ(kOutput, window->bounds());
}

TEST_F(WmTest, SetBoundsIsHonouredOnlyInTheNormalState)
{
    Window *window = makeWindow(base::Rect(10, 10, 300, 200));

    window->requestBounds(base::Rect(50, 60, 400, 300));
    EXPECT_EQ(base::Rect(50, 60, 400, 300), window->bounds());

    window->maximize();
    window->requestBounds(base::Rect(0, 0, 100, 100));
    EXPECT_EQ(kOutput, window->bounds());
}

TEST_F(WmTest, AWorkAreaChangeReappliesTheCurrentState)
{
    Window *window = makeWindow(base::Rect(10, 10, 300, 200));
    window->maximize();

    manager.setOutputBounds(base::Rect(0, 0, 640, 480));

    EXPECT_EQ(base::Rect(0, 0, 640, 480), window->bounds());
}

TEST_F(WmTest, ANonResizableWindowDoesNotMaximize)
{
    Window *window = makeWindow(base::Rect(10, 10, 300, 200));
    window->setResizable(false);

    window->maximize();

    EXPECT_EQ(WindowStateType::Normal, window->stateType());
    EXPECT_EQ(base::Rect(10, 10, 300, 200), window->bounds());
}

////////////////////////////////////////////////////////////////////////////////
// Activation

TEST_F(WmTest, TheFirstWindowActivatesOnRequestAndTakesKeyboardFocus)
{
    Window *window = makeWindow(base::Rect(0, 0, 200, 200));
    EXPECT_EQ(nullptr, manager.activation()->activeWindow());

    window->activate();

    EXPECT_EQ(window, manager.activation()->activeWindow());
    EXPECT_TRUE(window->isActive());
    EXPECT_EQ(window->rootSurface(), seat.focusedSurface());
}

TEST_F(WmTest, ActivatingASecondWindowDeactivatesTheFirst)
{
    Window *first = makeWindow(base::Rect(0, 0, 200, 200));
    Window *second = makeWindow(base::Rect(300, 0, 200, 200));

    first->activate();
    second->activate();

    EXPECT_FALSE(first->isActive());
    EXPECT_TRUE(second->isActive());
    EXPECT_EQ(second->rootSurface(), seat.focusedSurface());
    // Activation raises.
    EXPECT_EQ(second, manager.windows().back());
}

TEST_F(WmTest, ActivationChangesAreOrderedLossBeforeGain)
{
    struct Recorder : ActivationObserver {
        std::vector<std::pair<Window *, Window *>> changes;

        void onWindowActivated(Window *gained, Window *lost) override
        {
            changes.emplace_back(gained, lost);
        }
    } recorder;

    Window *first = makeWindow(base::Rect(0, 0, 200, 200));
    Window *second = makeWindow(base::Rect(300, 0, 200, 200));
    manager.activation()->addObserver(&recorder);

    first->activate();
    second->activate();

    ASSERT_EQ(2u, recorder.changes.size());
    EXPECT_EQ(std::make_pair(first, static_cast<Window *>(nullptr)), recorder.changes[0]);
    EXPECT_EQ(std::make_pair(second, first), recorder.changes[1]);
    manager.activation()->removeObserver(&recorder);
}

TEST_F(WmTest, DestroyingTheActiveWindowActivatesTheNextOne)
{
    Window *first = makeWindow(base::Rect(0, 0, 200, 200));
    Window *second = makeWindow(base::Rect(300, 0, 200, 200));

    first->activate();
    second->activate();
    manager.destroyWindow(second);

    EXPECT_EQ(first, manager.activation()->activeWindow());
    EXPECT_EQ(first->rootSurface(), seat.focusedSurface());
}

TEST_F(WmTest, DestroyingTheOnlyWindowClearsActivationAndFocus)
{
    Window *window = makeWindow(base::Rect(0, 0, 200, 200));
    window->activate();

    manager.destroyWindow(window);

    EXPECT_EQ(nullptr, manager.activation()->activeWindow());
    EXPECT_EQ(nullptr, seat.focusedSurface());
}

TEST_F(WmTest, DestroyingTheRootSurfaceDestroysTheWindow)
{
    Window *window = makeWindow(base::Rect(0, 0, 200, 200));
    window->activate();

    surfaces.back().reset();

    EXPECT_TRUE(manager.windows().empty());
    EXPECT_EQ(nullptr, manager.activation()->activeWindow());
    EXPECT_EQ(nullptr, seat.focusedSurface());
}

TEST_F(WmTest, MinimizingTheActiveWindowMovesActivationOn)
{
    Window *first = makeWindow(base::Rect(0, 0, 200, 200));
    Window *second = makeWindow(base::Rect(300, 0, 200, 200));

    first->activate();
    second->activate();
    second->minimize();

    EXPECT_FALSE(second->isActive());
    EXPECT_EQ(first, manager.activation()->activeWindow());
    EXPECT_EQ(first->rootSurface(), seat.focusedSurface());
}

TEST_F(WmTest, MinimizePassesOverTheDockAndLeavesItsSlotAfterwards)
{
    Window *first = makeWindow(base::Rect(0, 0, 200, 200));
    Window *second = makeWindow(base::Rect(0, 0, 200, 200));
    Window *front = makeWindow(base::Rect(0, 0, 200, 200));
    Window *dock = makeWindow(base::Rect(0, 700, 1000, 100));
    dock->setRole(WindowRole::Dock);
    dock->setFocusPolicy(FocusPolicy::NoFocus);
    // The dock's tooltip: it stays readable while a window lands under it.
    Window *tip = manager.createWindow(makeSurface(base::Size(100, 40)),
                                       {.type = WindowType::Popup, .transientParent = dock});
    second->activate();
    front->activate();

    front->minimize();

    const auto &order = surfaceContainer->children();
    auto index = [&](Window *window) {
        return std::find(order.begin(), order.end(), window->node()) - order.begin();
    };
    EXPECT_EQ(second, manager.activation()->activeWindow());
    EXPECT_FALSE(front->node()->isEffectivelyVisible());
    EXPECT_LT(index(dock), index(front));
    EXPECT_LT(index(front), index(tip));
    auto *animation = dynamic_cast<scene::WindowAnimation *>(front->node());
    ASSERT_NE(animation, nullptr);
    animation->finish();
    first->activate();
    EXPECT_LT(index(second), index(first));
    EXPECT_LT(index(first), index(dock));

    front->restore();
    // On its way out of the dock the window is still above it, under its tooltip.
    EXPECT_LT(index(dock), index(front));
    EXPECT_LT(index(front), index(tip));
    EXPECT_EQ(first, manager.activation()->activeWindow());
    animation->finish();
    manager.restackScene();
    EXPECT_LT(index(front), index(first));
    front->activate();
    EXPECT_LT(index(first), index(front));
    EXPECT_LT(index(front), index(dock));
}

TEST_F(WmTest, ASystemDialogStacksAboveApplicationsAndBelowTheDock)
{
    manager.setSystemDialogPolicy([](int64_t pid) {
        return pid == 42;
    });
    Window *app = makeWindow(base::Rect(0, 0, 400, 300));
    Window *dock = makeWindow(base::Rect(0, 700, 1000, 100));
    dock->setRole(WindowRole::Dock);
    Window *prompt = manager.createWindow(
            makeSurface(base::Size(300, 200)),
            {.requestedSize = {300, 200}, .processId = 42, .transientParent = app, .modal = true});
    Window *other = makeWindow(base::Rect(0, 0, 400, 300));
    // Its menus follow it, and an application raised afterwards stays under it.
    Window *menu = manager.createWindow(makeSurface(base::Size(100, 40)),
                                        {.type = WindowType::Popup, .transientParent = prompt});
    other->activate();
    app->activate();

    EXPECT_TRUE(prompt->isSystemDialog());
    EXPECT_FALSE(other->isSystemDialog());
    EXPECT_EQ(layerFor(prompt), StackingLayer::SystemDialog);
    EXPECT_EQ(layerFor(menu), StackingLayer::SystemDialog);
    EXPECT_TRUE(prompt->isDecorated() == app->isDecorated());
    const auto &order = manager.windows();
    auto index = [&](Window *window) {
        return std::find(order.begin(), order.end(), window) - order.begin();
    };
    EXPECT_LT(index(other), index(prompt));
    EXPECT_LT(index(app), index(prompt));
    EXPECT_LT(index(prompt), index(menu));
    EXPECT_LT(index(menu), index(dock));
}

TEST_F(WmTest, AWindowAskingToStayOnTopStacksAboveApplicationsAndBelowTheDock)
{
    Window *app = makeWindow(base::Rect(0, 0, 400, 300));
    Window *dock = makeWindow(base::Rect(0, 700, 1000, 100));
    dock->setRole(WindowRole::Dock);
    // The hint arrives before the first commit, so before the window exists.
    compositor::Surface *surface = makeSurface(base::Size(300, 200));
    surface->setAlwaysOnTop(true);
    Window *dialog = manager.createWindow(surface, {.requestedSize = {300, 200}});
    auto index = [&](Window *window) {
        const auto order = manager.windows();
        return std::find(order.begin(), order.end(), window) - order.begin();
    };

    app->activate();
    EXPECT_EQ(layerFor(dialog), StackingLayer::AlwaysOnTop);
    EXPECT_LT(index(app), index(dialog));
    EXPECT_LT(index(dialog), index(dock));

    surface->setAlwaysOnTop(false);
    manager.raise(app);
    EXPECT_EQ(layerFor(dialog), StackingLayer::Normal);
    EXPECT_LT(index(dialog), index(app));

    surface->setAlwaysOnTop(true);
    EXPECT_LT(index(app), index(dialog));
    EXPECT_LT(index(dialog), index(dock));
}

TEST_F(WmTest, MinimizingBackgroundWindowKeepsTheActiveWindow)
{
    Window *back = makeWindow(base::Rect(0, 0, 200, 200));
    Window *front = makeWindow(base::Rect(0, 0, 200, 200));
    front->activate();
    back->minimize();
    manager.raise(front);
    EXPECT_EQ(front, manager.activation()->activeWindow());
    EXPECT_FALSE(back->node()->isEffectivelyVisible());
}

TEST_F(WmTest, AMinimizedWindowCannotBeActivated)
{
    Window *window = makeWindow(base::Rect(0, 0, 200, 200));
    window->minimize();

    window->activate();

    EXPECT_FALSE(window->isActive());
    EXPECT_EQ(nullptr, manager.activation()->activeWindow());
}

TEST_F(WmTest, ActivatingAMinimizedTransientChildActivatesItsParent)
{
    Window *parent = makeWindow(base::Rect(0, 0, 200, 200));
    Window *child = makeWindow(base::Rect(50, 50, 100, 100));
    ASSERT_TRUE(child->setTransientParent(parent));
    child->minimize();

    child->activate();

    EXPECT_EQ(parent, manager.activation()->activeWindow());
}

TEST_F(WmTest, OnlyAWindowMovingUnderOrAcrossAStillPointerRetargetsIt)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    input.onPointerMotionAbsolute(at(1), base::PointF(150, 150));
    int changes = 0;
    const base::Subscription counting = manager.pointerTargetsChanged.connect([&changes] {
        ++changes;
    });

    // A resize in place leaves the pointer where it was in the window.
    window->setBounds(base::Rect(100, 100, 400, 300));
    EXPECT_EQ(0, changes);

    window->setBounds(base::Rect(120, 100, 400, 300));
    EXPECT_LT(0, changes);

    changes = 0;
    window->setBounds(base::Rect(500, 400, 400, 300));
    EXPECT_LT(0, changes);

    // Nothing under the pointer changes.
    changes = 0;
    window->setBounds(base::Rect(520, 400, 400, 300));
    EXPECT_EQ(0, changes);
}

TEST_F(WmTest, ClickingAWindowActivatesItAndFocusesItsSurface)
{
    Window *first = makeWindow(base::Rect(0, 0, 200, 200));
    Window *second = makeWindow(base::Rect(300, 0, 200, 200));

    input.onPointerMotionAbsolute(at(1), base::PointF(50, 50));
    EXPECT_EQ(nullptr, manager.activation()->activeWindow());

    input.onPointerButton(at(2), BTN_LEFT, true);
    EXPECT_EQ(first, manager.activation()->activeWindow());
    EXPECT_EQ(first->rootSurface(), seat.focusedSurface());

    input.onPointerButton(at(3), BTN_LEFT, false);
    input.onPointerMotionAbsolute(at(4), base::PointF(350, 50));
    input.onPointerButton(at(5), BTN_LEFT, true);
    EXPECT_EQ(second, manager.activation()->activeWindow());
    EXPECT_EQ(second->rootSurface(), seat.focusedSurface());

    // Clicking the background clears activation.
    input.onPointerButton(at(6), BTN_LEFT, false);
    input.onPointerMotionAbsolute(at(7), base::PointF(800, 700));
    input.onPointerButton(at(8), BTN_LEFT, true);
    EXPECT_EQ(nullptr, manager.activation()->activeWindow());
    EXPECT_EQ(nullptr, seat.focusedSurface());
}

TEST_F(WmTest, PickingACandidateLeavesTheWindowTypedIntoActive)
{
    scene::Node *inputMethod =
            scene.root()->addChild(std::make_unique<scene::Node>("input-method"));
    compositor.setInputMethodContainer(inputMethod);
    Window *window = makeWindow(base::Rect(0, 0, 300, 200));
    window->activate();
    compositor::Surface *candidates = makeSurface(base::Size(100, 50));
    candidates->reparentNode(inputMethod);
    candidates->setPosition(base::Point(20, 20));

    compositor::Surface *hit = nullptr;
    manager.windowAt(base::PointF(30, 30), &hit);
    EXPECT_EQ(candidates, hit);

    input.onPointerMotionAbsolute(at(1), base::PointF(30, 30));
    input.onPointerButton(at(2), BTN_LEFT, true);
    input.onPointerButton(at(3), BTN_LEFT, false);
    EXPECT_EQ(window, manager.activation()->activeWindow());
    EXPECT_EQ(window->rootSurface(), seat.focusedSurface());

    // Beside the popup the window is pressed as before.
    input.onPointerMotionAbsolute(at(4), base::PointF(200, 150));
    manager.windowAt(base::PointF(200, 150), &hit);
    EXPECT_EQ(window->rootSurface(), hit);
}

namespace {

// Hears which points reach a client.
struct TouchRecorder : input::TouchDelegate {
    std::vector<int32_t> downs;
    void onTouchDestroying(input::Touch *) override { }
    bool canAcceptTouchEventsForSurface(compositor::Surface *) const override { return true; }
    void onTouchDown(compositor::Surface *, input::Timestamp, int32_t id,
                     const base::PointF &) override
    {
        downs.push_back(id);
    }
    void onTouchUp(input::Timestamp, int32_t) override { }
    void onTouchMotion(input::Timestamp, int32_t, const base::PointF &) override { }
    void onTouchFrame() override { }
    void onTouchCancel() override { }
};

} // namespace

// wm::FocusController activates the window a touch press lands on, as it does
// for a click, and the point itself goes on to the client.
TEST_F(WmTest, TouchingAWindowActivatesItAndTheClientGetsThePoint)
{
    Window *first = makeWindow(base::Rect(0, 0, 200, 200));
    Window *second = makeWindow(base::Rect(300, 0, 200, 200));
    TouchRecorder recorder;
    input::Touch touch(&recorder, &seat);
    const auto normalized = [this](base::PointF point) {
        return base::PointF(point.x() / kOutput.width(), point.y() / kOutput.height());
    };

    input.onTouchDown(at(1), 1, normalized({50, 50}));
    input.onTouchFrame(at(1));
    EXPECT_EQ(first, manager.activation()->activeWindow());
    EXPECT_EQ(first->rootSurface(), seat.focusedSurface());
    input.onTouchUp(at(2), 1);
    input.onTouchDown(at(3), 2, normalized({350, 50}));
    input.onTouchFrame(at(3));
    EXPECT_EQ(second, manager.activation()->activeWindow());
    EXPECT_EQ((std::vector<int32_t> {1, 2}), recorder.downs);
    input.onTouchUp(at(4), 2);
    input.onTouchFrame(at(4));
}

TEST_F(WmTest, ClickingASubsurfaceActivatesTheOwningRootWindow)
{
    Window *window = makeWindow(base::Rect(0, 0, 200, 200));
    compositor::Surface *child = makeSurface(base::Size(60, 60));
    auto sub = compositor::SubSurface::create(child, window->rootSurface());
    sub->setPosition(base::Point(10, 10));
    window->rootSurface()->commit();

    compositor::Surface *hit = nullptr;
    ASSERT_EQ(window, manager.windowAt(base::PointF(20, 20), &hit));
    ASSERT_EQ(child, hit);

    input.onPointerMotionAbsolute(at(1), base::PointF(20, 20));
    input.onPointerButton(at(2), BTN_LEFT, true);

    EXPECT_EQ(window, manager.activation()->activeWindow());
    // Keyboard focus lands on the root, never on the subsurface.
    EXPECT_EQ(window->rootSurface(), seat.focusedSurface());
}

////////////////////////////////////////////////////////////////////////////////
// Placement

TEST_F(WmTest, TheFirstWindowIsCenteredInTheWorkArea)
{
    Window *window = manager.createWindow(makeSurface(base::Size(300, 200)));
    EXPECT_EQ(base::Rect(350, 300, 300, 200), window->bounds());
}

TEST_F(WmTest, SuccessiveWindowsAreCentered)
{
    Window *first = manager.createWindow(makeSurface(base::Size(300, 200)));
    Window *second = manager.createWindow(makeSurface(base::Size(300, 200)));
    Window *third = manager.createWindow(makeSurface(base::Size(300, 200)));

    EXPECT_EQ(base::Rect(350, 300, 300, 200), first->bounds());
    EXPECT_EQ(first->bounds(), second->bounds());
    EXPECT_EQ(first->bounds(), third->bounds());
}

TEST(PlacementTest, CentersInAnOffsetWorkAreaAndClampsEachDimension)
{
    Placement placement;
    const base::Rect workArea(-1000, 30, 1000, 721);
    EXPECT_EQ(base::Rect(-650, 290, 300, 200), placement.place(base::Size(300, 200), workArea));
    EXPECT_EQ(base::Rect(-1000, 290, 1000, 200), placement.place(base::Size(1200, 200), workArea));
    EXPECT_EQ(base::Rect(-650, 30, 300, 721), placement.place(base::Size(300, 900), workArea));
}

TEST(PlacementTest, AnAnchoredWindowGoesNextToItsRectangleAndStaysInTheWorkArea)
{
    using compositor::PlacementAnchor;
    using compositor::PlacementEdge;
    const base::Rect workArea(0, 30, 1000, 770);
    const PlacementAnchor below {base::Rect(700, 12, 0, 0), PlacementEdge::Bottom, PlacementEdge::Bottom};
    EXPECT_EQ(base::Rect(550, 30, 300, 200), placeAtAnchor(base::Size(300, 200), below, workArea));

    // Slid back from the right edge.
    const PlacementAnchor corner {base::Rect(950, 40, 0, 0), PlacementEdge::Bottom, PlacementEdge::Bottom};
    EXPECT_EQ(base::Rect(700, 40, 300, 200), placeAtAnchor(base::Size(300, 200), corner, workArea));

    // A point exactly where the window's top left goes.
    const PlacementAnchor topLeft {base::Rect(100, 300, 0, 0), PlacementEdge::TopLeft,
                                   PlacementEdge::BottomRight};
    EXPECT_EQ(base::Rect(100, 300, 300, 200), placeAtAnchor(base::Size(300, 200), topLeft, workArea));

    // Larger than the work area: shrunk into it.
    EXPECT_EQ(base::Rect(0, 30, 1000, 770), placeAtAnchor(base::Size(1200, 900), below, workArea));
}

TEST(PlacementTest, AnAnchoredWindowFlipsWhenOnlyTheOtherSideHasRoom)
{
    using compositor::PlacementAnchor;
    using compositor::PlacementEdge;
    // A bar along the bottom of the screen.
    const base::Rect workArea(0, 0, 1000, 770);
    const PlacementAnchor item {base::Rect(500, 770, 40, 30), PlacementEdge::Bottom, PlacementEdge::Bottom};
    EXPECT_EQ(base::Rect(370, 570, 300, 200), placeAtAnchor(base::Size(300, 200), item, workArea));
    // Room on neither side: this side, slid in.
    EXPECT_EQ(base::Rect(370, 0, 300, 770), placeAtAnchor(base::Size(300, 770), item, workArea));
}

TEST_F(WmTest, AnAnchoredWindowMapsNextToItsAnchorBeforeItsRestoreId)
{
    const auto map = [&](bool anchored) {
        compositor::Surface *surface = makeSurface(base::Size(200, 100));
        surface->setRestoreId("calendar");
        if (anchored) {
            surface->setPlacementAnchor(compositor::PlacementAnchor {
                    base::Rect(600, 0, 0, 0), compositor::PlacementEdge::Bottom,
                    compositor::PlacementEdge::Bottom});
        }
        return manager.createWindow(surface, {.requestedSize = base::Size(200, 100),
                                              .applicationId = "app",
                                              .positionManaged = true});
    };
    Window *first = map(false);
    first->requestBounds(base::Rect(100, 120, 200, 100));
    manager.destroyWindow(first);

    Window *anchored = map(true);
    EXPECT_EQ(base::Rect(500, 0, 200, 100), anchored->bounds());
    manager.destroyWindow(anchored);

    // Where the anchored window was left is what the restore id remembers.
    Window *restored = map(false);
    EXPECT_EQ(base::Rect(500, 0, 200, 100), restored->bounds());
}

TEST_F(WmTest, AnOversizedWindowIsClampedToTheWorkArea)
{
    Window *window =
            manager.createWindow(makeSurface(base::Size(64, 64)), {.requestedSize = base::Size(2000, 1600)});

    EXPECT_EQ(kOutput, window->bounds());
}

TEST_F(WmTest, MovingAWindowOffScreenLeavesAGraspableStrip)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));

    window->requestBounds(base::Rect(1500, 100, 300, 200));
    EXPECT_LT(window->bounds().x(), kOutput.width());
    EXPECT_GT(window->bounds().x() + window->bounds().width(), 0);

    // Never above the work area: the titlebar must stay reachable.
    window->requestBounds(base::Rect(100, -400, 300, 200));
    EXPECT_EQ(0, window->bounds().y());
}

TEST_F(WmTest, MovingAWindowUnderTheDockKeepsItsStripAboveTheDock)
{
    manager.setExclusiveZone(this, ScreenEdge::Bottom, 80);
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));

    // The body may slide under the dock, but a strip stays above it: a
    // window whose only visible part is behind the dock cannot be grabbed.
    window->requestBounds(base::Rect(100, 790, 300, 200));
    EXPECT_EQ(kOutput.height() - 80 - kMinimumOnScreenArea, window->bounds().y());
    EXPECT_GT(window->bounds().y() + window->bounds().height(), kOutput.height() - 80);

    manager.clearExclusiveZone(this);
}

////////////////////////////////////////////////////////////////////////////////
// Transients

TEST_F(WmTest, SettingAndChangingATransientParent)
{
    Window *a = makeWindow(base::Rect(0, 0, 100, 100));
    Window *b = makeWindow(base::Rect(0, 0, 100, 100));
    Window *child = makeWindow(base::Rect(0, 0, 100, 100));

    ASSERT_TRUE(child->setTransientParent(a));
    EXPECT_EQ(a, child->transientParent());
    EXPECT_EQ(std::vector<Window *> {child}, a->transientChildren());

    ASSERT_TRUE(child->setTransientParent(b));
    EXPECT_EQ(b, child->transientParent());
    EXPECT_TRUE(a->transientChildren().empty());
    EXPECT_EQ(std::vector<Window *> {child}, b->transientChildren());

    ASSERT_TRUE(child->setTransientParent(nullptr));
    EXPECT_EQ(nullptr, child->transientParent());
    EXPECT_TRUE(b->transientChildren().empty());
}

TEST_F(WmTest, ATransientParentCannotFormACycle)
{
    Window *a = makeWindow(base::Rect(0, 0, 100, 100));
    Window *b = makeWindow(base::Rect(0, 0, 100, 100));
    Window *c = makeWindow(base::Rect(0, 0, 100, 100));

    ASSERT_TRUE(b->setTransientParent(a));
    ASSERT_TRUE(c->setTransientParent(b));

    EXPECT_FALSE(a->setTransientParent(a));
    EXPECT_FALSE(a->setTransientParent(c));
    EXPECT_EQ(nullptr, a->transientParent());
}

TEST_F(WmTest, DestroyingATransientParentOrphansItsChildren)
{
    Window *parent = makeWindow(base::Rect(0, 0, 100, 100));
    Window *child = makeWindow(base::Rect(0, 0, 100, 100));
    ASSERT_TRUE(child->setTransientParent(parent));

    manager.destroyWindow(parent);

    EXPECT_EQ(nullptr, child->transientParent());
    EXPECT_EQ(1u, manager.windows().size());
}

TEST_F(WmTest, DestroyingATransientChildDetachesItFromTheParent)
{
    Window *parent = makeWindow(base::Rect(0, 0, 100, 100));
    Window *child = makeWindow(base::Rect(0, 0, 100, 100));
    ASSERT_TRUE(child->setTransientParent(parent));

    manager.destroyWindow(child);

    EXPECT_TRUE(parent->transientChildren().empty());
}

////////////////////////////////////////////////////////////////////////////////
// Interactive move

TEST_F(WmTest, AMoveFollowsThePointerDelta)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));

    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(150, 120)));
    EXPECT_TRUE(manager.eventHandler()->isDragging());
    EXPECT_FALSE(window->isResizing()); // A move is not a resize.

    manager.eventHandler()->onPointerMotion(base::PointF(200, 170));
    EXPECT_EQ(base::Rect(150, 150, 300, 200), window->bounds());

    manager.eventHandler()->onPointerButtonReleased();
    EXPECT_FALSE(manager.eventHandler()->isDragging());
    EXPECT_EQ(base::Rect(150, 150, 300, 200), window->bounds());
}

TEST_F(WmTest, AMoveIsDrivenThroughTheInputManagerAndConsumesEvents)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));

    input.onPointerMotionAbsolute(at(1), base::PointF(150, 120));
    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(150, 120)));

    input.onPointerMotionAbsolute(at(2), base::PointF(250, 220));
    EXPECT_EQ(base::Rect(200, 200, 300, 200), window->bounds());

    input.onPointerButton(at(3), BTN_LEFT, false);
    EXPECT_FALSE(manager.eventHandler()->isDragging());
}

TEST_F(WmTest, EscapeRevertsAMove)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));

    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(150, 120)));
    input.onPointerMotionAbsolute(at(1), base::PointF(400, 400));
    ASSERT_NE(base::Rect(100, 100, 300, 200), window->bounds());

    input.onKey(at(2), KEY_ESC, true);

    EXPECT_FALSE(manager.eventHandler()->isDragging());
    EXPECT_EQ(base::Rect(100, 100, 300, 200), window->bounds());
}

TEST_F(WmTest, ASecondDragIsRefusedWhileOneIsRunning)
{
    Window *first = makeWindow(base::Rect(100, 100, 300, 200));
    Window *second = makeWindow(base::Rect(500, 100, 300, 200));

    ASSERT_TRUE(manager.eventHandler()->beginMove(first, base::PointF(150, 120)));
    EXPECT_FALSE(manager.eventHandler()->beginMove(second, base::PointF(550, 120)));
    EXPECT_EQ(first, manager.eventHandler()->draggedWindow());
}

TEST_F(WmTest, AMaximizedWindowMovesButDoesNotResize)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));

    window->maximize();
    EXPECT_FALSE(manager.eventHandler()->beginResize(window, ResizeEdge::Right, base::PointF(999, 400)));
    EXPECT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(150, 20)));
    manager.eventHandler()->completeDrag(ToplevelEventHandler::DragResult::Revert);
}

TEST_F(WmTest, MinimizedAndFullscreenWindowsRefuseInteractiveDrags)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));

    window->setFullscreen(true);
    EXPECT_FALSE(manager.eventHandler()->beginMove(window, base::PointF(150, 120)));
    EXPECT_FALSE(manager.eventHandler()->beginResize(window, ResizeEdge::Right, base::PointF(400, 200)));

    window->setFullscreen(false);
    window->minimize();
    EXPECT_FALSE(manager.eventHandler()->beginMove(window, base::PointF(150, 120)));
}

TEST_F(WmTest, DestroyingTheDraggedWindowEndsTheDrag)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(150, 120)));

    manager.destroyWindow(window);

    EXPECT_FALSE(manager.eventHandler()->isDragging());
    EXPECT_EQ(nullptr, manager.eventHandler()->draggedWindow());
}

TEST_F(WmTest, MovingUpdatesTheSceneNodePosition)
{
    Window *window = makeWindow(base::Rect(100, 100, 300, 200));
    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(150, 120)));

    manager.eventHandler()->onPointerMotion(base::PointF(200, 170));

    EXPECT_EQ(base::Point(150, 150), window->node()->position());
    // The node keeps the committed buffer's size; resizing the client is
    // xdg_toplevel's job, not the window manager's.
    EXPECT_EQ(base::Size(300, 200), window->node()->geometry().size());
}

////////////////////////////////////////////////////////////////////////////////
// Dragging out of a tiled state, and dragging into one

TEST_F(WmTest, TheEdgeTolerancesFollowTheWorkArea)
{
    // Every edge of the work area is an edge of the output, so all three
    // tolerances apply.
    EXPECT_EQ(base::Rect(32, 8, 936, 792), nonSnapArea(kOutput, kOutput));

    // Behind a dock the tolerance goes away: the pointer has to cross the dock
    // to reach the edge.
    const base::Rect withLeftDock(60, 0, 940, 800);
    EXPECT_EQ(base::Rect(60, 8, 908, 792), nonSnapArea(withLeftDock, kOutput));

    EXPECT_EQ(SnapType::Primary, snapTypeAt(base::PointF(20, 400), kOutput, kOutput));
    EXPECT_EQ(SnapType::Secondary, snapTypeAt(base::PointF(980, 400), kOutput, kOutput));
    EXPECT_EQ(SnapType::Maximize, snapTypeAt(base::PointF(500, 4), kOutput, kOutput));
    EXPECT_EQ(SnapType::None, snapTypeAt(base::PointF(500, 400), kOutput, kOutput));
    // A corner belongs to the side, as it does in Ash.
    EXPECT_EQ(SnapType::Primary, snapTypeAt(base::PointF(4, 2), kOutput, kOutput));
}

TEST_F(WmTest, DraggingToTheLeftEdgeSnapsTheWindowToThatHalf)
{
    Window *window = makeWindow(base::Rect(300, 300, 300, 200));

    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(350, 320)));
    manager.eventHandler()->onPointerMotion(base::PointF(20, 400));
    EXPECT_EQ(SnapType::Primary, manager.eventHandler()->snapController()->snapType());

    manager.eventHandler()->onPointerButtonReleased();

    EXPECT_EQ(WindowStateType::PrimarySnapped, window->stateType());
    EXPECT_EQ(base::Rect(0, 0, 500, 800), window->bounds());

    window->restore();
    EXPECT_EQ(WindowStateType::Normal, window->stateType());
    EXPECT_EQ(base::Size(300, 200), window->bounds().size());
}

TEST_F(WmTest, DraggingToTheRightEdgeSnapsTheWindowToTheOtherHalf)
{
    Window *window = makeWindow(base::Rect(300, 300, 300, 200));

    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(350, 320)));
    manager.eventHandler()->onPointerMotion(base::PointF(990, 400));
    manager.eventHandler()->onPointerButtonReleased();

    EXPECT_EQ(WindowStateType::SecondarySnapped, window->stateType());
    EXPECT_EQ(base::Rect(500, 0, 500, 800), window->bounds());
}

TEST_F(WmTest, TheSnapPreviewGrowsOutOfItsCentreAndFadesIn)
{
    const base::RectF target(8, 8, 484, 784);

    const base::RectF start = scene::SnapPreviewNode::entranceBounds(target, 0);
    EXPECT_EQ(target.center(), start.center());
    EXPECT_DOUBLE_EQ(target.width() * 0.75, start.width());
    EXPECT_DOUBLE_EQ(target.height() * 0.75, start.height());
    EXPECT_EQ(0.f, scene::SnapPreviewNode::entranceOpacity(0));

    const base::RectF middle = scene::SnapPreviewNode::entranceBounds(target, 75);
    EXPECT_GT(middle.width(), start.width());
    EXPECT_LT(middle.width(), target.width());
    EXPECT_EQ(1.f, scene::SnapPreviewNode::entranceOpacity(75));

    EXPECT_EQ(target, scene::SnapPreviewNode::entranceBounds(target, 150));
}

TEST_F(WmTest, AWindowTooWideForHalfTheWorkAreaIgnoresTheSideEdges)
{
    Window *window = makeWindow(base::Rect(300, 300, 300, 200));
    window->setMinimumSize(base::Size(800, 100));

    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(350, 320)));
    manager.eventHandler()->onPointerMotion(base::PointF(20, 400));

    EXPECT_EQ(SnapType::None, manager.eventHandler()->snapController()->snapType());
    manager.eventHandler()->onPointerButtonReleased();
    EXPECT_EQ(WindowStateType::Normal, window->stateType());
}

TEST_F(WmTest, TheTopEdgeMaximizesOnlyAfterTheDwellRunsOut)
{
    Window *window = makeWindow(base::Rect(300, 300, 300, 200));

    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(350, 320)));
    manager.eventHandler()->onPointerMotion(base::PointF(350, 4));
    // The countdown is running, but nothing has been decided yet.
    EXPECT_EQ(SnapType::None, manager.eventHandler()->snapController()->snapType());

    manager.eventHandler()->snapController()->triggerDwellForTesting();
    EXPECT_EQ(SnapType::Maximize, manager.eventHandler()->snapController()->snapType());

    manager.eventHandler()->onPointerButtonReleased();
    EXPECT_EQ(WindowStateType::Maximized, window->stateType());
    EXPECT_EQ(manager.workAreaFor(window), window->bounds());
}

TEST_F(WmTest, AWindowJustBelowTheTopMaximizesWithoutAFullDragDown)
{
    // Grabbed a few pixels under the top edge: far less than Ash's 64px of
    // vertical travel, which is what made the top so hard to hit.
    Window *window = makeWindow(base::Rect(300, 10, 300, 200));

    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(350, 20)));
    manager.eventHandler()->onPointerMotion(base::PointF(360, 2));
    manager.eventHandler()->snapController()->triggerDwellForTesting();

    EXPECT_EQ(SnapType::Maximize, manager.eventHandler()->snapController()->snapType());
    manager.eventHandler()->onPointerButtonReleased();
    EXPECT_EQ(WindowStateType::Maximized, window->stateType());
}

TEST_F(WmTest, DraggingAlongTheTopEdgeDoesNotMaximizeUntilThePointerLeavesIt)
{
    Window *window = makeWindow(base::Rect(300, 0, 300, 200));

    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(350, 4)));
    manager.eventHandler()->onPointerMotion(base::PointF(600, 4));
    manager.eventHandler()->snapController()->triggerDwellForTesting();
    EXPECT_EQ(SnapType::None, manager.eventHandler()->snapController()->snapType());

    manager.eventHandler()->onPointerMotion(base::PointF(600, 40));
    manager.eventHandler()->onPointerMotion(base::PointF(600, 4));
    manager.eventHandler()->snapController()->triggerDwellForTesting();
    EXPECT_EQ(SnapType::Maximize, manager.eventHandler()->snapController()->snapType());
    manager.eventHandler()->onPointerButtonReleased();
}

TEST_F(WmTest, AMaximizedWindowShrinksUnderThePointerAndEndsUpNormal)
{
    Window *window = makeWindow(base::Rect(200, 100, 400, 300));
    window->maximize();
    ASSERT_EQ(base::Rect(0, 0, 1000, 800), window->bounds());

    // Grabbed halfway along the title bar.
    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(500, 10)));
    manager.eventHandler()->onPointerMotion(base::PointF(503, 12));
    EXPECT_EQ(base::Rect(0, 0, 1000, 800), window->bounds()) << "below the unmaximize threshold";

    manager.eventHandler()->onPointerMotion(base::PointF(500, 300));
    // Back to the restore size, still grabbed halfway along it.
    EXPECT_EQ(base::Rect(300, 290, 400, 300), window->bounds());
    // The state machine only follows at the end of the drag, but everything
    // the client and the decoration see is already the restored window.
    EXPECT_EQ(WindowStateType::Maximized, window->stateType());
    EXPECT_EQ(WindowStateType::Normal, window->presentedStateType());

    manager.eventHandler()->onPointerButtonReleased();

    EXPECT_EQ(WindowStateType::Normal, window->stateType());
    EXPECT_EQ(base::Rect(300, 290, 400, 300), window->bounds());
    EXPECT_FALSE(window->state()->hasRestoreBounds());
    EXPECT_FALSE(window->isRestoringFromDrag());
}

TEST_F(WmTest, EscapeDuringAnUnmaximizeDragLeavesTheWindowMaximized)
{
    Window *window = makeWindow(base::Rect(200, 100, 400, 300));
    window->maximize();

    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(500, 10)));
    manager.eventHandler()->onPointerMotion(base::PointF(500, 300));
    ASSERT_NE(base::Rect(0, 0, 1000, 800), window->bounds());

    input.onKey(at(1), KEY_ESC, true);

    EXPECT_EQ(WindowStateType::Maximized, window->stateType());
    EXPECT_EQ(base::Rect(0, 0, 1000, 800), window->bounds());
    EXPECT_FALSE(window->isRestoringFromDrag());
    // The bounds it would have gone back to are back as well.
    window->restore();
    EXPECT_EQ(base::Rect(200, 100, 400, 300), window->bounds());
}

TEST_F(WmTest, AMaximizedWindowDraggedBackToTheTopStaysMaximized)
{
    Window *window = makeWindow(base::Rect(200, 100, 400, 300));
    window->maximize();

    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(500, 10)));
    manager.eventHandler()->onPointerMotion(base::PointF(500, 300));
    manager.eventHandler()->onPointerMotion(base::PointF(500, 4));
    manager.eventHandler()->snapController()->triggerDwellForTesting();
    manager.eventHandler()->onPointerButtonReleased();

    EXPECT_EQ(WindowStateType::Maximized, window->stateType());
    EXPECT_EQ(base::Rect(0, 0, 1000, 800), window->bounds());
    // And it still knows where to go back to.
    window->restore();
    EXPECT_EQ(base::Rect(200, 100, 400, 300), window->bounds());
}

TEST_F(WmTest, ASnappedWindowDraggedIntoTheMiddleBecomesNormal)
{
    Window *window = makeWindow(base::Rect(200, 100, 400, 300));
    window->snapPrimary();
    ASSERT_EQ(base::Rect(0, 0, 500, 800), window->bounds());

    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(250, 10)));
    manager.eventHandler()->onPointerMotion(base::PointF(600, 400));
    manager.eventHandler()->onPointerButtonReleased();

    EXPECT_EQ(WindowStateType::Normal, window->stateType());
    EXPECT_EQ(base::Size(400, 300), window->bounds().size());
}

TEST_F(WmTest, ASnappedWindowKeepsItsStateWhileItsSharedEdgeIsResized)
{
    Window *window = makeWindow(base::Rect(200, 100, 400, 300));
    window->snapPrimary();

    ASSERT_TRUE(manager.eventHandler()->beginResize(window, ResizeEdge::Right, base::PointF(500, 400)));
    manager.eventHandler()->onPointerMotion(base::PointF(650, 400));
    manager.eventHandler()->onPointerButtonReleased();

    EXPECT_EQ(WindowStateType::PrimarySnapped, window->stateType());
    EXPECT_EQ(base::Rect(0, 0, 650, 800), window->bounds());
}

TEST_F(WmTest, ASnappedWindowResizedOffItsEdgeBecomesNormal)
{
    Window *window = makeWindow(base::Rect(200, 100, 400, 300));
    window->snapPrimary();

    ASSERT_TRUE(manager.eventHandler()->beginResize(window, ResizeEdge::Bottom, base::PointF(250, 800)));
    manager.eventHandler()->onPointerMotion(base::PointF(250, 600));
    manager.eventHandler()->onPointerButtonReleased();

    EXPECT_EQ(WindowStateType::Normal, window->stateType());
    EXPECT_EQ(base::Rect(0, 0, 500, 600), window->bounds());
}

TEST_F(WmTest, MaximizingASnappedWindowRestoresBackToSnapped)
{
    Window *window = makeWindow(base::Rect(200, 100, 400, 300));

    window->snapSecondary();
    window->maximize();
    ASSERT_EQ(base::Rect(0, 0, 1000, 800), window->bounds());

    window->restore();
    EXPECT_EQ(WindowStateType::SecondarySnapped, window->stateType());
    EXPECT_EQ(base::Rect(500, 0, 500, 800), window->bounds());

    window->restore();
    EXPECT_EQ(WindowStateType::Normal, window->stateType());
    EXPECT_EQ(base::Rect(200, 100, 400, 300), window->bounds());
}

TEST_F(WmTest, TheSnapPreviewIsStackedDirectlyUnderTheDraggedWindow)
{
    Window *below = makeWindow(base::Rect(100, 100, 300, 200));
    Window *window = makeWindow(base::Rect(300, 300, 300, 200));

    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(350, 320)));
    manager.eventHandler()->onPointerMotion(base::PointF(20, 400));

    scene::Node *preview = manager.dragOverlay();
    ASSERT_NE(nullptr, preview);
    // Ash's phantom sits inset from where the window will land.
    EXPECT_EQ(base::Rect(8, 8, 484, 784), preview->geometry());
    const auto &order = surfaceContainer->children();
    const auto previewSlot = std::find(order.begin(), order.end(), preview);
    const auto windowSlot = std::find(order.begin(), order.end(), window->node());
    ASSERT_NE(order.end(), previewSlot);
    ASSERT_NE(order.end(), windowSlot);
    EXPECT_LT(std::find(order.begin(), order.end(), below->node()), previewSlot);
    EXPECT_LT(previewSlot, windowSlot);

    manager.eventHandler()->onPointerButtonReleased();
    EXPECT_EQ(nullptr, manager.dragOverlay());
}

TEST_F(WmTest, LeavingATiledStateByHandCrossFades)
{
    Window *window = makeWindow(base::Rect(200, 100, 400, 300));
    window->maximize();
    auto *animation = dynamic_cast<scene::WindowAnimation *>(window->node());
    ASSERT_NE(nullptr, animation);
    animation->finish(); // The maximize animation is not what this is about.

    ASSERT_TRUE(manager.eventHandler()->beginMove(window, base::PointF(500, 10)));
    manager.eventHandler()->onPointerMotion(base::PointF(503, 12));
    EXPECT_FALSE(animation->isRunning()) << "nothing has moved yet";

    manager.eventHandler()->onPointerMotion(base::PointF(500, 300));
    EXPECT_TRUE(animation->isRunning());
    EXPECT_EQ(scene::WindowAnimation::Type::DragBounds, animation->type());
}

////////////////////////////////////////////////////////////////////////////////
// Interactive resize

class ResizeTest : public WmTest, public testing::WithParamInterface<std::pair<ResizeEdge, base::Rect>>
{
};

TEST_P(ResizeTest, DraggingAnEdgeKeepsTheOppositeOneFixed)
{
    Window *window = makeWindow(base::Rect(200, 200, 300, 200));
    const base::PointF start(350, 300);
    // The same drag for every edge; only the edge under test may respond.
    const base::PointF end = start + base::PointF(-40, -30);

    ASSERT_TRUE(manager.eventHandler()->beginResize(window, GetParam().first, start));
    EXPECT_TRUE(window->isResizing());
    manager.eventHandler()->onPointerMotion(end);
    EXPECT_EQ(GetParam().second, window->bounds());

    manager.eventHandler()->onPointerButtonReleased();
    EXPECT_FALSE(window->isResizing());
}

INSTANTIATE_TEST_SUITE_P(AllEdges, ResizeTest,
                         testing::Values(
                                 // dx = -40, dy = -30 from a 300x200 window at (200, 200).
                                 std::make_pair(ResizeEdge::Left, base::Rect(160, 200, 340, 200)),
                                 std::make_pair(ResizeEdge::Right, base::Rect(200, 200, 260, 200)),
                                 std::make_pair(ResizeEdge::Top, base::Rect(200, 170, 300, 230)),
                                 std::make_pair(ResizeEdge::Bottom, base::Rect(200, 200, 300, 170)),
                                 std::make_pair(ResizeEdge::TopLeft, base::Rect(160, 170, 340, 230)),
                                 std::make_pair(ResizeEdge::TopRight, base::Rect(200, 170, 260, 230)),
                                 std::make_pair(ResizeEdge::BottomLeft, base::Rect(160, 200, 340, 170)),
                                 std::make_pair(ResizeEdge::BottomRight,
                                                base::Rect(200, 200, 260, 170))));

TEST_F(WmTest, ResizingLeftPastTheMinimumWidthPinsTheRightEdge)
{
    Window *window = makeWindow(base::Rect(200, 200, 300, 200));
    window->setMinimumSize(base::Size(100, 80));

    ASSERT_TRUE(manager.eventHandler()->beginResize(window, ResizeEdge::Left, base::PointF(200, 300)));
    manager.eventHandler()->onPointerMotion(base::PointF(1000, 300));

    EXPECT_EQ(base::Size(100, 200), window->bounds().size());
    // Right edge unmoved at 500.
    EXPECT_EQ(500, window->bounds().x() + window->bounds().width());
}

TEST_F(WmTest, ResizingTopPastTheMinimumHeightPinsTheBottomEdge)
{
    Window *window = makeWindow(base::Rect(200, 200, 300, 200));
    window->setMinimumSize(base::Size(100, 80));

    ASSERT_TRUE(manager.eventHandler()->beginResize(window, ResizeEdge::Top, base::PointF(350, 200)));
    manager.eventHandler()->onPointerMotion(base::PointF(350, 700));

    EXPECT_EQ(base::Size(300, 80), window->bounds().size());
    EXPECT_EQ(400, window->bounds().y() + window->bounds().height());
}

TEST_F(WmTest, ResizingLeftPastTheMaximumWidthPinsTheRightEdge)
{
    Window *window = makeWindow(base::Rect(200, 200, 300, 200));
    window->setMaximumSize(base::Size(400, 400));

    ASSERT_TRUE(manager.eventHandler()->beginResize(window, ResizeEdge::Left, base::PointF(200, 300)));
    manager.eventHandler()->onPointerMotion(base::PointF(0, 300));

    EXPECT_EQ(base::Size(400, 200), window->bounds().size());
    EXPECT_EQ(500, window->bounds().x() + window->bounds().width());
}

TEST_F(WmTest, ANonResizableWindowDoesNotResize)
{
    Window *window = makeWindow(base::Rect(200, 200, 300, 200));
    window->setResizable(false);

    EXPECT_FALSE(manager.eventHandler()->beginResize(window, ResizeEdge::Right, base::PointF(500, 300)));
    EXPECT_EQ(base::Rect(200, 200, 300, 200), window->bounds());
}

TEST_F(WmTest, EscapeRevertsAResize)
{
    Window *window = makeWindow(base::Rect(200, 200, 300, 200));

    ASSERT_TRUE(manager.eventHandler()->beginResize(window, ResizeEdge::BottomRight,
                                                    base::PointF(500, 400)));
    input.onPointerMotionAbsolute(at(1), base::PointF(600, 500));
    ASSERT_EQ(base::Size(400, 300), window->bounds().size());

    input.onKey(at(2), KEY_ESC, true);

    EXPECT_EQ(base::Rect(200, 200, 300, 200), window->bounds());
    EXPECT_FALSE(window->isResizing());
}

TEST_F(WmTest, AResizeTakesOneStepPerFrame)
{
    Window *window = makeWindow(base::Rect(200, 200, 300, 200));
    ASSERT_TRUE(manager.eventHandler()->beginResize(window, ResizeEdge::BottomRight,
                                                    base::PointF(500, 400)));
    // The first move lands at once; the rest of the frame's wait for it.
    input.onPointerMotionAbsolute(at(1), base::PointF(510, 410));
    EXPECT_EQ(base::Size(310, 210), window->bounds().size());
    input.onPointerMotionAbsolute(at(2), base::PointF(520, 420));
    input.onPointerMotionAbsolute(at(3), base::PointF(530, 430));
    EXPECT_EQ(base::Size(310, 210), window->bounds().size());
    // The next frame takes only the newest.
    compositor.frameStarted(16);
    EXPECT_EQ(base::Size(330, 230), window->bounds().size());
    compositor.frameStarted(32);
    input.onPointerMotionAbsolute(at(4), base::PointF(540, 440));
    EXPECT_EQ(base::Size(340, 240), window->bounds().size());
    input.onPointerMotionAbsolute(at(5), base::PointF(560, 460));
    // The drag ends where the pointer last was.
    input.onPointerButton(at(6), BTN_LEFT, false);
    EXPECT_EQ(base::Rect(200, 200, 360, 260), window->bounds());
    EXPECT_FALSE(window->isResizing());
}

TEST_F(WmTest, TheResizerMathIsIndependentOfTheWindow)
{
    Window *window = makeWindow(base::Rect(200, 200, 300, 200));
    WindowResizer resizer(window, base::PointF(200, 200), ResizeEdge::TopLeft, kOutput);

    EXPECT_EQ(base::Rect(180, 190, 320, 210), resizer.calculateBoundsForDrag(base::PointF(180, 190)));
    // Nothing was applied.
    EXPECT_EQ(base::Rect(200, 200, 300, 200), window->bounds());
}

////////////////////////////////////////////////////////////////////////////////
// Surface to window mapping and stacking

TEST_F(WmTest, ASubsurfaceResolvesToItsToplevelWindow)
{
    Window *window = makeWindow(base::Rect(0, 0, 200, 200));
    compositor::Surface *child = makeSurface(base::Size(60, 60));
    auto sub = compositor::SubSurface::create(child, window->rootSurface());
    window->rootSurface()->commit();

    EXPECT_EQ(window, manager.windowForSurface(child));
    EXPECT_EQ(window, manager.windowForSurface(window->rootSurface()));
}

TEST_F(WmTest, ASurfaceWithNoWindowResolvesToNothing)
{
    compositor::Surface *orphan = makeSurface(base::Size(60, 60));
    EXPECT_EQ(nullptr, manager.windowForSurface(orphan));
    EXPECT_EQ(nullptr, manager.windowForSurface(nullptr));
}

TEST_F(WmTest, RaisingReordersBothTheModelAndTheScene)
{
    Window *first = makeWindow(base::Rect(0, 0, 200, 200));
    Window *second = makeWindow(base::Rect(0, 0, 200, 200));
    ASSERT_EQ(second, manager.windows().back());

    manager.raise(first);

    EXPECT_EQ(first, manager.windows().back());
    EXPECT_EQ(first->node(), surfaceContainer->children().back());
    // Overlapping windows: the topmost one wins the hit test.
    EXPECT_EQ(first, manager.windowAt(base::PointF(50, 50)));
}

TEST_F(WmTest, RaisingSettingsKeepsItsDialogAboveIt)
{
    Window *settings = makeWindow(base::Rect(0, 0, 200, 200));
    Window *dialog = makeWindow(base::Rect(0, 0, 200, 200));
    ASSERT_TRUE(dialog->setTransientParent(settings));
    Window *other = makeWindow(base::Rect(0, 0, 200, 200));

    manager.raise(settings);

    EXPECT_EQ((std::vector<Window *> {other, settings, dialog}), manager.windows());
    EXPECT_EQ(dialog->node(), surfaceContainer->children().back());
    EXPECT_EQ(dialog, manager.windowAt(base::PointF(50, 50)));

    manager.raise(other);
    EXPECT_EQ(other, manager.windowAt(base::PointF(50, 50)));
    manager.raise(dialog);
    EXPECT_EQ((std::vector<Window *> {other, settings, dialog}), manager.windows());
    EXPECT_EQ(dialog, manager.windowAt(base::PointF(50, 50)));
}

TEST_F(WmTest, RaisingTransientSiblingPreservesNestedFamilyOrder)
{
    Window *parent = makeWindow(base::Rect(0, 0, 200, 200));
    Window *first = makeWindow(base::Rect(0, 0, 200, 200));
    Window *nested = makeWindow(base::Rect(0, 0, 200, 200));
    Window *second = makeWindow(base::Rect(0, 0, 200, 200));
    ASSERT_TRUE(first->setTransientParent(parent));
    ASSERT_TRUE(nested->setTransientParent(first));
    ASSERT_TRUE(second->setTransientParent(parent));

    manager.raise(first);
    EXPECT_EQ((std::vector<Window *> {parent, second, first, nested}), manager.windows());
    manager.raise(parent);
    EXPECT_EQ((std::vector<Window *> {parent, second, first, nested}), manager.windows());
    manager.raise(second);
    EXPECT_EQ((std::vector<Window *> {parent, first, nested, second}), manager.windows());
}

TEST_F(WmTest, ChangingDialogParentRestacksImmediately)
{
    Window *dialog = makeWindow(base::Rect(0, 0, 200, 200));
    Window *first = makeWindow(base::Rect(0, 0, 200, 200));
    Window *second = makeWindow(base::Rect(0, 0, 200, 200));

    ASSERT_TRUE(dialog->setTransientParent(second));
    EXPECT_EQ((std::vector<Window *> {first, second, dialog}), manager.windows());
    EXPECT_EQ(dialog->node(), surfaceContainer->children().back());
    ASSERT_TRUE(dialog->setTransientParent(first));
    EXPECT_EQ((std::vector<Window *> {first, dialog, second}), manager.windows());
    ASSERT_TRUE(dialog->setTransientParent(nullptr));
    manager.raise(first);
    EXPECT_EQ((std::vector<Window *> {dialog, second, first}), manager.windows());
}

TEST_F(WmTest, PanelsAndTheDockAppearWithoutAnimation)
{
    for (WindowRole role : {WindowRole::Panel, WindowRole::Dock}) {
        Window *window = makeWindow(base::Rect(0, 700, 1000, 100));
        window->setRole(role);
        auto *animation = dynamic_cast<scene::WindowAnimation *>(window->node());
        ASSERT_NE(nullptr, animation);
        EXPECT_EQ(scene::WindowAnimation::Style::None, animation->style());
    }
}

TEST_F(WmTest, PopupsGetAFadeAnimationNode)
{
    Window *parent = makeWindow(base::Rect(0, 0, 400, 300));
    Window *popup = manager.createWindow(makeSurface(base::Size(100, 40)),
                                         {.type = WindowType::Popup, .transientParent = parent});
    auto *animation = dynamic_cast<scene::WindowAnimation *>(popup->node());
    ASSERT_NE(nullptr, animation);
    EXPECT_EQ(scene::WindowAnimation::Style::Popup, animation->style());
    popup->setRole(WindowRole::Dock);
    EXPECT_EQ(scene::WindowAnimation::Style::Popup, animation->style());

    Window *dock = makeWindow(base::Rect(0, 700, 1000, 100));
    dock->setRole(WindowRole::Dock);
    Window *tip = manager.createWindow(makeSurface(base::Size(100, 40)),
                                       {.type = WindowType::Popup, .transientParent = dock});
    Window *submenu = manager.createWindow(makeSurface(base::Size(100, 40)),
                                           {.type = WindowType::Popup, .transientParent = tip});
    for (Window *window : {tip, submenu}) {
        EXPECT_EQ(scene::WindowAnimation::Style::None,
                  static_cast<scene::WindowAnimation *>(window->node())->style());
    }
}

TEST_F(WmTest, AnAnchoredWindowFadesInPlaceLikeAPopup)
{
    compositor::Surface *surface = makeSurface(base::Size(200, 100));
    surface->setPlacementAnchor(compositor::PlacementAnchor {
            base::Rect(600, 0, 0, 0), compositor::PlacementEdge::Bottom, compositor::PlacementEdge::Bottom});
    Window *anchored = manager.createWindow(surface, {.requestedSize = base::Size(200, 100)});
    auto *animation = dynamic_cast<scene::WindowAnimation *>(anchored->node());
    ASSERT_NE(nullptr, animation);
    EXPECT_EQ(scene::WindowAnimation::Style::Popup, animation->style());
    anchored->setRole(WindowRole::Panel);
    EXPECT_EQ(scene::WindowAnimation::Style::None, animation->style());
    anchored->setRole(WindowRole::Normal);
    EXPECT_EQ(scene::WindowAnimation::Style::Popup, animation->style());

    Window *plain = makeWindow(base::Rect(0, 0, 200, 100));
    EXPECT_EQ(scene::WindowAnimation::Style::Window,
              static_cast<scene::WindowAnimation *>(plain->node())->style());
}

TEST_F(WmTest, DesktopPopupTreeStaysAboveApplicationsAndBelowDock)
{
    Window *desktop = makeWindow(base::Rect(0, 0, 1000, 800));
    desktop->setRole(WindowRole::Desktop);
    Window *application = makeWindow(base::Rect(0, 0, 200, 200));
    Window *dock = makeWindow(base::Rect(0, 700, 200, 100));
    dock->setRole(WindowRole::Dock);
    auto makePopup = [&](Window *parent) {
        return manager.createWindow(makeSurface(base::Size(100, 100)),
                                    {.type = WindowType::Popup, .transientParent = parent});
    };
    Window *popup = makePopup(desktop);
    Window *submenu = makePopup(popup);
    Window *dockPopup = makePopup(dock);

    auto verifyOrder = [&] {
        const std::vector<Window *> expected {desktop, application, popup,
                                              submenu, dock,        dockPopup};
        EXPECT_EQ(expected, manager.windows());
        const auto &nodes = surfaceContainer->children();
        for (size_t i = 1; i < expected.size(); ++i) {
            EXPECT_LT(std::find(nodes.begin(), nodes.end(), expected[i - 1]->node()),
                      std::find(nodes.begin(), nodes.end(), expected[i]->node()));
        }
    };
    verifyOrder();
    manager.raise(application);
    verifyOrder();
}

////////////////////////////////////////////////////////////////////////////////
// Teardown re-entrancy
//
// Destroying a window notifies activation, which schedules a repaint. Anything
// listening to that - the demo controller's window adoption does - runs while the surface's
// destructor is on the stack, so neither the compositor's surface list nor the
// manager's window list may be in a half-updated state there.

TEST_F(WmTest, ADyingSurfaceIsNoLongerListedByTheCompositor)
{
    Window *window = makeWindow(base::Rect(0, 0, 200, 200));
    compositor::Surface *surface = window->rootSurface();

    bool sawSurface = true;

    struct Watcher : compositor::SurfaceObserver {
        compositor::Compositor *compositor = nullptr;
        compositor::Surface *surface = nullptr;
        bool *sawSurface = nullptr;

        void onSurfaceDestroying(compositor::Surface *) override
        {
            const auto &list = compositor->surfaces();
            *sawSurface = std::find(list.begin(), list.end(), surface) != list.end();
        }
    } watcher;

    watcher.compositor = &compositor;
    watcher.surface = surface;
    watcher.sawSurface = &sawSurface;
    surface->addSurfaceObserver(&watcher);

    surfaces.back().reset();

    EXPECT_FALSE(sawSurface);
}

TEST_F(WmTest, AWindowAddedWhileTheActiveOneIsBeingDestroyedDoesNotCorruptTheList)
{
    Window *first = makeWindow(base::Rect(0, 0, 200, 200));
    first->activate();
    ASSERT_EQ(first, manager.activation()->activeWindow());

    // Adopt a window from the activation change that destroying `first`
    // causes - the earliest point teardown hands control back out, and what
    // the demo controller effectively does.
    struct Adopter : ActivationObserver {
        WindowManager *manager = nullptr;
        compositor::Surface *surface = nullptr;
        Window *created = nullptr;

        void onWindowActivated(Window *, Window *) override
        {
            if (!created)
                created = manager->createWindow(surface);
        }
    } adopter;

    adopter.manager = &manager;
    adopter.surface = makeSurface(base::Size(100, 100));
    manager.activation()->addObserver(&adopter);

    manager.destroyWindow(first);
    manager.activation()->removeObserver(&adopter);

    ASSERT_NE(nullptr, adopter.created);
    ASSERT_EQ(1u, manager.windows().size());
    EXPECT_EQ(adopter.created, manager.windows().front());
    EXPECT_EQ(adopter.surface, manager.windows().front()->rootSurface());
}

TEST_F(WmTest, OutputScaleChangesLogicalStatesAndPreservesRestoreBounds)
{
    auto *normal = makeWindow({100, 50, 300, 200});
    auto *maximized = makeWindow({150, 100, 300, 200});
    auto *fullscreen = makeWindow({200, 150, 300, 200});
    maximized->maximize();
    fullscreen->setFullscreen(true);
    const auto scaleChanged = compositor.outputScaleChanged.connect([&] {
        manager.setOutputBounds(
                base::Rect(base::Point(), compositor.outputScale().logicalSize({1920, 1080})));
    });
    for (const auto &value : {"1.5", "1.75", "2", "1"}) {
        compositor.setOutputScale(*compositor::OutputScale::parse(value));
        EXPECT_EQ(normal->bounds(), base::Rect(100, 50, 300, 200));
        EXPECT_EQ(maximized->bounds(), manager.workAreaFor(maximized));
        EXPECT_EQ(fullscreen->bounds(), manager.outputBounds());
    }
    maximized->restore();
    fullscreen->setFullscreen(false);
    EXPECT_EQ(maximized->bounds(), base::Rect(150, 100, 300, 200));
    EXPECT_EQ(fullscreen->bounds(), base::Rect(200, 150, 300, 200));
}

} // namespace

TEST_F(WmTest, ClosingWindowKeepsItsSceneSlotWhenFocusRaisesAnotherWindow)
{
    Window *background = makeWindow(kOutput);
    Window *next = makeWindow(base::Rect(100, 100, 300, 200));
    Window *closing = makeWindow(base::Rect(100, 100, 300, 200));
    manager.activation()->activate(closing);
    scene::Node *snapshot = closing->node();

    manager.destroyWindow(closing);

    EXPECT_EQ(next, manager.activation()->activeWindow());
    auto order = surfaceContainer->children();
    const auto position = [&](scene::Node *node) {
        return std::find(order.begin(), order.end(), node) - order.begin();
    };
    EXPECT_LT(position(background->node()), position(next->node()));
    EXPECT_LT(position(next->node()), position(snapshot));

    manager.raise(background);
    order = surfaceContainer->children();
    EXPECT_LT(position(next->node()), position(background->node()));
    EXPECT_LT(position(background->node()), position(snapshot));
}

////////////////////////////////////////////////////////////////////////////////
// Window positioner

TEST_F(WmTest, WindowsOfOneApplicationCascadeFromTheCenter)
{
    Window *first = mapManagedWindow(base::Size(300, 200));
    Window *second = mapManagedWindow(base::Size(300, 200));
    manager.activation()->activate(second);
    Window *third = mapManagedWindow(base::Size(200, 100));

    EXPECT_EQ(base::Rect(350, 300, 300, 200), first->bounds());
    EXPECT_EQ(base::Rect(382, 332, 300, 200), second->bounds());
    EXPECT_EQ(base::Rect(414, 364, 200, 100), third->bounds());
}

TEST_F(WmTest, AnotherApplicationsWindowIsCentered)
{
    mapManagedWindow(base::Size(300, 200), "files");
    Window *other = mapManagedWindow(base::Size(300, 200), "terminal");
    Window *unnamed = mapManagedWindow(base::Size(300, 200), std::string());
    Window *unnamedAgain = mapManagedWindow(base::Size(300, 200), std::string());

    EXPECT_EQ(base::Rect(350, 300, 300, 200), other->bounds());
    EXPECT_EQ(base::Rect(350, 300, 300, 200), unnamed->bounds());
    EXPECT_EQ(base::Rect(350, 300, 300, 200), unnamedAgain->bounds());
}

TEST_F(WmTest, FixedSizeWindowCentersOverActiveApplicationWithoutParenting)
{
    Window *first = mapManagedWindow(base::Size(400, 300));
    first->requestBounds(base::Rect(100, 100, 400, 300));
    manager.activation()->activate(first);
    Window *dialog = manager.createWindow(makeSurface(base::Size(200, 100)),
                                          {.requestedSize = base::Size(200, 100),
                                           .applicationId = "app",
                                           .minimumSize = base::Size(200, 100),
                                           .maximumSize = base::Size(200, 100),
                                           .positionManaged = true});

    EXPECT_EQ(base::Rect(200, 200, 200, 100), dialog->bounds());
    EXPECT_EQ(nullptr, dialog->transientParent());
    first->requestBounds(base::Rect(300, 300, 400, 300));
    EXPECT_EQ(base::Rect(200, 200, 200, 100), dialog->bounds());
}

TEST_F(WmTest, FixedSizeWindowWithoutActiveApplicationReferenceUsesWorkArea)
{
    Window *first = mapManagedWindow(base::Size(400, 300));
    first->requestBounds(base::Rect(100, 100, 400, 300));
    Window *other = mapManagedWindow(base::Size(300, 200), "other");
    manager.activation()->activate(other);
    Window *dialog = manager.createWindow(makeSurface(base::Size(200, 100)),
                                          {.requestedSize = base::Size(200, 100),
                                           .applicationId = "app",
                                           .minimumSize = base::Size(200, 100),
                                           .maximumSize = base::Size(200, 100),
                                           .positionManaged = true});

    EXPECT_EQ(base::Rect(400, 350, 200, 100), dialog->bounds());
    EXPECT_EQ(nullptr, dialog->transientParent());
}

TEST_F(WmTest, AWindowWithARestoreIdOpensWhereTheLastOneWasLeft)
{
    const auto map = [&](const base::Size &size, compositor::ResizeAnchor anchor) {
        compositor::Surface *surface = makeSurface(size);
        surface->setRestoreId("preview");
        surface->setResizeAnchor(anchor);
        return manager.createWindow(surface, {.requestedSize = size,
                                              .applicationId = "app",
                                              .positionManaged = true});
    };
    Window *first = map(base::Size(200, 100), compositor::ResizeAnchor::TopLeft);
    EXPECT_EQ(base::Rect(400, 350, 200, 100), first->bounds());
    first->requestBounds(base::Rect(100, 120, 200, 100));
    manager.destroyWindow(first);

    Window *second = map(base::Size(300, 200), compositor::ResizeAnchor::TopLeft);
    EXPECT_EQ(base::Rect(100, 120, 300, 200), second->bounds());
    second->requestBounds(base::Rect(600, 500, 300, 200));
    manager.destroyWindow(second);

    // The centre comes back, and the window stays inside the work area.
    Window *third = map(base::Size(600, 500), compositor::ResizeAnchor::Center);
    EXPECT_EQ(base::Rect(400, 300, 600, 500), third->bounds());
}

TEST_F(WmTest, AMaximizedWindowWithARestoreIdIsRememberedByItsRestoreBounds)
{
    const auto map = [&] {
        compositor::Surface *surface = makeSurface(base::Size(200, 100));
        surface->setRestoreId("preview");
        return manager.createWindow(surface, {.requestedSize = base::Size(200, 100),
                                              .applicationId = "app",
                                              .positionManaged = true});
    };
    Window *first = map();
    first->requestBounds(base::Rect(100, 120, 200, 100));
    first->maximize();
    manager.destroyWindow(first);

    Window *second = map();
    EXPECT_EQ(base::Rect(100, 120, 200, 100), second->bounds());
}

TEST_F(WmTest, ARestoreIdIsKeptPerApplicationAndOutput)
{
    const auto map = [&](const std::string &applicationId, const std::string &restoreId) {
        compositor::Surface *surface = makeSurface(base::Size(200, 100));
        surface->setRestoreId(restoreId);
        return manager.createWindow(surface, {.requestedSize = base::Size(200, 100),
                                              .applicationId = applicationId,
                                              .positionManaged = true});
    };
    Window *first = map("app", "preview");
    first->requestBounds(base::Rect(100, 120, 200, 100));
    manager.destroyWindow(first);

    Window *other = map("other", "preview");
    EXPECT_EQ(base::Rect(400, 350, 200, 100), other->bounds());
    manager.destroyWindow(other);
    Window *otherId = map("app", "inspector");
    EXPECT_EQ(base::Rect(400, 350, 200, 100), otherId->bounds());
    manager.destroyWindow(otherId);

    // The output it was left on is gone.
    manager.setOutputBounds(base::Rect(0, 0, 1200, 900));
    Window *moved = map("app", "preview");
    EXPECT_EQ(base::Rect(500, 400, 200, 100), moved->bounds());
}

// Restore ids are the client's to choose, so only so many are remembered.
TEST_F(WmTest, OnlyTheMostRecentRestoreIdsAreRemembered)
{
    const auto map = [&](const std::string &restoreId) {
        compositor::Surface *surface = makeSurface(base::Size(200, 100));
        surface->setRestoreId(restoreId);
        return manager.createWindow(surface, {.requestedSize = base::Size(200, 100),
                                              .applicationId = "app",
                                              .positionManaged = true});
    };
    const auto leave = [&](const std::string &restoreId) {
        Window *window = map(restoreId);
        window->requestBounds(base::Rect(100, 120, 200, 100));
        manager.destroyWindow(window);
    };
    leave("first");
    leave("kept");
    for (int i = 0; i < 255; ++i) {
        leave("id" + std::to_string(i));
        // Used again, so it stays while the others push the first one out.
        if (i == 200)
            manager.destroyWindow(map("kept"));
    }

    Window *first = map("first");
    EXPECT_EQ(base::Rect(400, 350, 200, 100), first->bounds());
    manager.destroyWindow(first);
    Window *kept = map("kept");
    EXPECT_EQ(base::Rect(100, 120, 200, 100), kept->bounds());
}

TEST_F(WmTest, TheCascadeFollowsTheMostRecentWindowWhereverItIs)
{
    Window *first = mapManagedWindow(base::Size(300, 200));
    first->requestBounds(base::Rect(10, 20, 300, 200));
    Window *second = mapManagedWindow(base::Size(300, 200));
    EXPECT_EQ(base::Rect(42, 52, 300, 200), second->bounds());

    // Minimized windows and dialogs are not followed.
    second->minimize();
    Window *dialog = makeWindow(base::Rect(600, 500, 100, 100));
    dialog->setApplicationId("app");
    ASSERT_TRUE(dialog->setTransientParent(first));
    Window *third = mapManagedWindow(base::Size(300, 200));
    EXPECT_EQ(base::Rect(42, 52, 300, 200), third->bounds());
}

TEST_F(WmTest, AnAlwaysOnTopWindowIsNotACascadeReferenceForNormalWindows)
{
    Window *floating = mapManagedWindow(base::Size(200, 100));
    floating->setAlwaysOnTop(true);
    floating->requestBounds(base::Rect(10, 20, 200, 100));
    Window *normal = mapManagedWindow(base::Size(300, 200));
    EXPECT_EQ(base::Rect(350, 300, 300, 200), normal->bounds());

    Window *second = mapManagedWindow(base::Size(300, 200));
    EXPECT_EQ(base::Rect(382, 332, 300, 200), second->bounds());
}

TEST_F(WmTest, ACascadeRunningOffTheWorkAreaRestartsAtItsCorner)
{
    Window *first = mapManagedWindow(base::Size(300, 200));
    first->requestBounds(base::Rect(690, 400, 300, 200));
    Window *second = mapManagedWindow(base::Size(300, 200));
    EXPECT_EQ(base::Rect(0, 0, 300, 200), second->bounds());
}

TEST_F(WmTest, ANewWindowCascadesFromAMaximizedOnesRestoreBounds)
{
    Window *first = mapManagedWindow(base::Size(300, 200));
    first->maximize();
    ASSERT_EQ(base::Rect(350, 300, 300, 200), first->state()->restoreBounds());

    Window *second = mapManagedWindow(base::Size(200, 100));
    EXPECT_EQ(base::Rect(382, 332, 200, 100), second->bounds());
    EXPECT_EQ(kOutput, first->bounds());
}

TEST_F(WmTest, OnlyTheSameApplicationsMaximizedStateIsInherited)
{
    mapManagedWindow(base::Size(300, 200))->maximize();

    EXPECT_EQ(WindowStateType::Maximized,
              window_positioner::showStateForNewWindow(&manager, kOutput, "app",
                                                       WindowStateType::Normal));
    EXPECT_EQ(WindowStateType::Fullscreen,
              window_positioner::showStateForNewWindow(&manager, kOutput, "app",
                                                       WindowStateType::Fullscreen));
    EXPECT_EQ(WindowStateType::Normal,
              window_positioner::showStateForNewWindow(&manager, kOutput, "other",
                                                       WindowStateType::Normal));
}

////////////////////////////////////////////////////////////////////////////////
// Transient placement

TEST_F(WmTest, ATransientWindowIsCenteredOverItsParent)
{
    Window *parent = makeWindow(base::Rect(100, 100, 600, 400));
    Window *dialog =
            manager.createWindow(makeSurface(base::Size(200, 100)),
                                 {.requestedSize = base::Size(200, 100), .transientParent = parent});

    EXPECT_EQ(base::Rect(300, 250, 200, 100), dialog->bounds());
}

TEST_F(WmTest, ATransientLargerThanItsParentIsCenteredInTheWorkArea)
{
    Window *parent = makeWindow(base::Rect(0, 0, 100, 100));
    Window *dialog =
            manager.createWindow(makeSurface(base::Size(400, 300)),
                                 {.requestedSize = base::Size(400, 300), .transientParent = parent});

    EXPECT_EQ(base::Rect(300, 250, 400, 300), dialog->bounds());
}

TEST_F(WmTest, ATransientStaysInsideTheWorkArea)
{
    Window *parent = makeWindow(base::Rect(800, 700, 600, 400));
    Window *dialog =
            manager.createWindow(makeSurface(base::Size(300, 200)),
                                 {.requestedSize = base::Size(300, 200), .transientParent = parent});

    EXPECT_EQ(base::Rect(700, 600, 300, 200), dialog->bounds());
}

TEST(PlacementTest, TransientPlacementSkipsTheParentsTopInset)
{
    Placement placement;
    EXPECT_EQ(base::Rect(150, 130, 100, 100),
              placement.placeTransient(base::Size(100, 100), base::Rect(0, 0, 400, 300), 60,
                                       base::Rect(0, 0, 1000, 800)));
}

////////////////////////////////////////////////////////////////////////////////
// Window modality

TEST_F(WmTest, AModalTransientTakesActivationFromItsParent)
{
    Window *w1 = makeWindow(base::Rect(0, 0, 100, 100));
    Window *w11 = makeWindow(base::Rect(0, 0, 100, 100));
    Window *w12 = makeWindow(base::Rect(0, 0, 100, 100));

    ASSERT_TRUE(w11->setTransientParent(w1));
    manager.activation()->activate(w1);
    EXPECT_TRUE(w1->isActive());
    manager.activation()->activate(w11);
    EXPECT_TRUE(w11->isActive());

    ASSERT_TRUE(w12->setTransientParent(w1));
    w12->setModal(true);
    EXPECT_TRUE(w12->isActive());

    manager.activation()->activate(w11);
    EXPECT_TRUE(w11->isActive());
    EXPECT_EQ((std::vector<Window *> {w1, w12, w11}), manager.windows());

    manager.activation()->activate(w1);
    EXPECT_TRUE(w12->isActive());
    EXPECT_EQ((std::vector<Window *> {w1, w11, w12}), manager.windows());

    manager.destroyWindow(w12);
    EXPECT_TRUE(w11->isActive());
    manager.destroyWindow(w11);
    EXPECT_TRUE(w1->isActive());
}

TEST_F(WmTest, AModalTransientDimsItsParentWhileInEffect)
{
    Window *w1 = makeWindow(base::Rect(0, 0, 300, 200));
    Window *w2 = makeWindow(base::Rect(0, 0, 300, 200));
    Window *w11 = makeWindow(base::Rect(0, 0, 100, 100));
    const auto dim = [](Window *window) {
        return dynamic_cast<scene::WindowVisualNode *>(window->visualNode())->state().dimOpacity;
    };
    const float expected = manager.visualStyle().dimOpacity;
    ASSERT_GT(expected, 0);

    ASSERT_TRUE(w11->setTransientParent(w1));
    EXPECT_FLOAT_EQ(dim(w1), 0);
    w11->setModal(true);
    EXPECT_FLOAT_EQ(dim(w1), expected);
    EXPECT_FLOAT_EQ(dim(w11), 0);

    w11->minimize();
    EXPECT_FLOAT_EQ(dim(w1), 0);
    w11->restore();
    EXPECT_FLOAT_EQ(dim(w1), expected);

    ASSERT_TRUE(w11->setTransientParent(w2));
    EXPECT_FLOAT_EQ(dim(w1), 0);
    EXPECT_FLOAT_EQ(dim(w2), expected);

    w11->setModal(false);
    EXPECT_FLOAT_EQ(dim(w2), 0);
    w11->setModal(true);
    EXPECT_FLOAT_EQ(dim(w2), expected);

    manager.destroyWindow(w11);
    EXPECT_FLOAT_EQ(dim(w2), 0);
}

TEST_F(WmTest, AWindowCreatedModalDimsItsParentAtOnce)
{
    Window *parent = makeWindow(base::Rect(0, 0, 300, 200));
    Window *dialog = manager.createWindow(
            makeSurface({100, 100}),
            {.requestedSize = {100, 100}, .transientParent = parent, .modal = true});
    const auto *node = dynamic_cast<scene::WindowVisualNode *>(parent->visualNode());
    EXPECT_FLOAT_EQ(node->state().dimOpacity, manager.visualStyle().dimOpacity);
    manager.destroyWindow(dialog);
    EXPECT_FLOAT_EQ(node->state().dimOpacity, 0);
}

TEST_F(WmTest, ActivatingAParentReachesTheInnermostNestedModal)
{
    Window *w1 = makeWindow(base::Rect(0, 0, 100, 100));
    Window *w11 = makeWindow(base::Rect(0, 0, 100, 100));
    Window *w111 = makeWindow(base::Rect(0, 0, 100, 100));
    Window *w2 = makeWindow(base::Rect(0, 0, 100, 100));
    ASSERT_TRUE(w11->setTransientParent(w1));
    ASSERT_TRUE(w111->setTransientParent(w11));

    manager.activation()->activate(w2);
    w11->setModal(true);
    w111->setModal(true);

    manager.activation()->activate(w1);
    EXPECT_TRUE(w111->isActive());
    EXPECT_EQ((std::vector<Window *> {w2, w1, w11, w111}), manager.windows());
    manager.activation()->activate(w11);
    EXPECT_TRUE(w111->isActive());

    manager.activation()->activate(w2);
    EXPECT_TRUE(w2->isActive());
    EXPECT_EQ((std::vector<Window *> {w1, w11, w111, w2}), manager.windows());

    manager.destroyWindow(w2);
    EXPECT_TRUE(w111->isActive());
    manager.destroyWindow(w111);
    EXPECT_TRUE(w11->isActive());
    manager.destroyWindow(w11);
    EXPECT_TRUE(w1->isActive());
}

TEST_F(WmTest, HidingTheInnerModalHandsActivationToTheOuterOne)
{
    Window *w1 = makeWindow(base::Rect(0, 0, 100, 100));
    Window *w11 = makeWindow(base::Rect(0, 0, 100, 100));
    Window *w111 = makeWindow(base::Rect(0, 0, 100, 100));
    ASSERT_TRUE(w11->setTransientParent(w1));
    ASSERT_TRUE(w111->setTransientParent(w11));
    w11->setModal(true);
    w111->setModal(true);

    manager.activation()->activate(w1);
    EXPECT_TRUE(w111->isActive());

    w111->minimize();
    EXPECT_TRUE(w11->isActive());
    EXPECT_EQ(w11, modalTransient(w1));

    manager.destroyWindow(w11);
    EXPECT_TRUE(w1->isActive());
}

TEST_F(WmTest, APressOnABlockedParentActivatesTheModalAndIsNotDelivered)
{
    Window *w1 = makeWindow(base::Rect(0, 0, 100, 100));
    Window *w11 = makeWindow(base::Rect(20, 20, 50, 50));
    Window *w111 = makeWindow(base::Rect(20, 20, 50, 50));
    ASSERT_TRUE(w11->setTransientParent(w1));
    // A non-modal child of the modal does not lift the block.
    ASSERT_TRUE(w111->setTransientParent(w11));

    input.onPointerMotionAbsolute(at(1), base::PointF(10, 10));
    input.onPointerButton(at(2), BTN_LEFT, true);
    EXPECT_TRUE(w1->isActive());
    EXPECT_EQ(w1->rootSurface(), seat.pointerGrabSurface());
    input.onPointerButton(at(3), BTN_LEFT, false);

    w11->setModal(true);
    manager.activation()->activate(nullptr);

    input.onPointerMotionAbsolute(at(4), base::PointF(12, 12));
    input.onPointerButton(at(5), BTN_LEFT, true);
    EXPECT_TRUE(w11->isActive());
    EXPECT_EQ(nullptr, seat.pointerGrabSurface());
    input.onPointerButton(at(6), BTN_LEFT, false);
}

TEST_F(WmTest, APressOnTheUncoveredPartOfABlockedParentActivatesTheModal)
{
    Window *w1 = makeWindow(base::Rect(0, 0, 100, 100));
    Window *w11 = makeWindow(base::Rect(20, 20, 50, 50));
    ASSERT_TRUE(w11->setTransientParent(w1));
    Window *w2 = makeWindow(base::Rect(0, 0, 50, 50));
    w11->setModal(true);

    manager.activation()->activate(w2);
    input.onPointerMotionAbsolute(at(1), base::PointF(90, 90));
    input.onPointerButton(at(2), BTN_LEFT, true);
    EXPECT_TRUE(w11->isActive());
}

TEST_F(WmTest, OnlyAVisibleModalChildBlocksItsParent)
{
    Window *w1 = makeWindow(base::Rect(0, 0, 100, 100));
    Window *w2 = makeWindow(base::Rect(0, 0, 100, 100));
    w2->setModal(true);
    EXPECT_EQ(nullptr, modalTransient(w1));

    ASSERT_TRUE(w2->setTransientParent(w1));
    EXPECT_EQ(w2, modalTransient(w1));
    EXPECT_FALSE(manager.activation()->canActivate(w1));
    EXPECT_EQ(nullptr, modalTransient(w2));

    w2->minimize();
    EXPECT_EQ(nullptr, modalTransient(w1));
    EXPECT_TRUE(manager.activation()->canActivate(w1));
}

TEST_F(WmTest, ABlockedParentGetsNoPointerFocus)
{
    Window *parent = makeWindow(base::Rect(0, 0, 400, 400));
    Window *dialog = makeWindow(base::Rect(100, 100, 100, 100));
    ASSERT_TRUE(dialog->setTransientParent(parent));
    dialog->setModal(true);

    input.onPointerMotionAbsolute(at(1), base::PointF(150, 150));
    input.onPointerButton(at(2), BTN_LEFT, true);
    EXPECT_EQ(dialog->rootSurface(), seat.pointerGrabSurface());
    input.onPointerButton(at(3), BTN_LEFT, false);
    seat.setCompositorCursorShape(input::CursorShape::Text);

    // Over the parent, neither the press nor the cursor shape changes hands.
    input.onPointerMotionAbsolute(at(4), base::PointF(20, 20));
    EXPECT_EQ(input::CursorShape::Text, seat.compositorCursorShape());
    input.onPointerButton(at(5), BTN_LEFT, true);
    EXPECT_EQ(nullptr, seat.pointerGrabSurface());
    EXPECT_EQ(input::CursorShape::Text, seat.compositorCursorShape());
    input.onPointerButton(at(6), BTN_LEFT, false);

    dialog->setModal(false);
    input.onPointerMotionAbsolute(at(7), base::PointF(21, 21));
    input.onPointerButton(at(8), BTN_LEFT, true);
    EXPECT_EQ(parent->rootSurface(), seat.pointerGrabSurface());
    EXPECT_TRUE(parent->isActive());
}

TEST_F(WmTest, AParentlessModalDoesNotStealActivation)
{
    Window *other = makeWindow(base::Rect(0, 0, 100, 100));
    manager.activation()->activate(other);
    Window *modal = makeWindow(base::Rect(0, 0, 100, 100));

    modal->setModal(true);
    EXPECT_TRUE(other->isActive());
    EXPECT_FALSE(isModalInEffect(modal));

    Window *mapped = manager.createWindow(makeSurface(base::Size(50, 50)),
                                          {.requestedSize = base::Size(50, 50), .modal = true});
    EXPECT_TRUE(mapped->isModal());
    EXPECT_TRUE(other->isActive());
}

TEST_F(WmTest, AModalThatGainsAParentBlocksIt)
{
    Window *parent = makeWindow(base::Rect(0, 0, 100, 100));
    Window *modal = makeWindow(base::Rect(0, 0, 100, 100));
    modal->setModal(true);
    manager.activation()->activate(parent);
    ASSERT_TRUE(parent->isActive());

    ASSERT_TRUE(modal->setTransientParent(parent));
    EXPECT_TRUE(isModalInEffect(modal));
    EXPECT_TRUE(modal->isActive());

    ASSERT_TRUE(modal->setTransientParent(nullptr));
    EXPECT_FALSE(isModalInEffect(modal));
    manager.activation()->activate(parent);
    EXPECT_TRUE(parent->isActive());
}

TEST_F(WmTest, RestoringAMinimizedModalBlocksItsParentAgain)
{
    Window *parent = makeWindow(base::Rect(0, 0, 100, 100));
    Window *modal = makeWindow(base::Rect(0, 0, 100, 100));
    ASSERT_TRUE(modal->setTransientParent(parent));
    modal->setModal(true);
    ASSERT_TRUE(modal->isActive());

    modal->minimize();
    EXPECT_FALSE(isModalInEffect(modal));
    manager.activation()->activate(parent);
    EXPECT_TRUE(parent->isActive());

    modal->restore();
    EXPECT_EQ(modal, modalTransient(parent));
    EXPECT_TRUE(modal->isActive());
}

TEST_F(WmTest, AModalLosesItsEffectWithItsParent)
{
    Window *parent = makeWindow(base::Rect(0, 0, 100, 100));
    Window *modal = makeWindow(base::Rect(0, 0, 100, 100));
    ASSERT_TRUE(modal->setTransientParent(parent));
    modal->setModal(true);

    manager.destroyWindow(parent);
    EXPECT_FALSE(isModalInEffect(modal));
    EXPECT_TRUE(modal->isActive());
}

namespace {

class LockedGate : public input::SessionGate
{
public:
    bool restrictsInput() const override { return false; }

    scene::Node *inputRoot() const override { return nullptr; }

    bool allowsFocus(const compositor::Surface *) const override { return !locked; }

    bool allowsActivation() const override { return !locked; }

    bool locked = false;
};

} // namespace

TEST_F(WmTest, ModalReevaluationNeverActivatesThroughALockedSession)
{
    LockedGate gate;
    manager.setSessionGate(&gate);
    Window *parent = makeWindow(base::Rect(0, 0, 100, 100));
    Window *modal = makeWindow(base::Rect(0, 0, 100, 100));
    ASSERT_TRUE(modal->setTransientParent(parent));
    manager.activation()->activate(parent);
    ASSERT_TRUE(parent->isActive());

    gate.locked = true;
    modal->setModal(true);
    EXPECT_FALSE(modal->isActive());
    modal->minimize();
    modal->restore();
    EXPECT_FALSE(modal->isActive());
    manager.activation()->activate(parent);
    EXPECT_FALSE(modal->isActive());

    gate.locked = false;
    manager.activation()->activate(parent);
    EXPECT_TRUE(modal->isActive());
    manager.setSessionGate(nullptr);
}

TEST_F(WmTest, ScrollingOverABlockedParentIsDroppedWithoutActivatingTheModal)
{
    Window *parent = makeWindow(base::Rect(0, 0, 400, 400));
    Window *modal = makeWindow(base::Rect(100, 100, 100, 100));
    Window *other = makeWindow(base::Rect(500, 0, 100, 100));
    ASSERT_TRUE(modal->setTransientParent(parent));
    modal->setModal(true);
    manager.activation()->activate(other);

    EXPECT_TRUE(manager.onPointerAxis(at(1), base::PointF(20, 20), base::PointF(0, 10), false));
    EXPECT_TRUE(manager.onPointerAxisStop(at(2), base::PointF(20, 20)));
    EXPECT_TRUE(other->isActive());
    EXPECT_FALSE(manager.onPointerAxis(at(3), base::PointF(150, 150), base::PointF(0, 10), false));
    EXPECT_FALSE(manager.onPointerAxis(at(4), base::PointF(550, 50), base::PointF(0, 10), false));
}

TEST_F(WmTest, AConsumedPressKeepsItsReleaseForEachButton)
{
    Window *parent = makeWindow(base::Rect(0, 0, 400, 400));
    Window *modal = makeWindow(base::Rect(100, 100, 100, 100));
    ASSERT_TRUE(modal->setTransientParent(parent));
    modal->setModal(true);

    EXPECT_TRUE(manager.onPointerButton(at(1), base::PointF(20, 20), BTN_LEFT, true));
    EXPECT_TRUE(manager.onPointerButton(at(2), base::PointF(20, 20), BTN_RIGHT, true));
    // The modal is gone before the releases; they stay consumed all the same.
    modal->setModal(false);
    EXPECT_TRUE(manager.onPointerButton(at(3), base::PointF(20, 20), BTN_LEFT, false));
    EXPECT_TRUE(manager.onPointerButton(at(4), base::PointF(20, 20), BTN_RIGHT, false));

    EXPECT_FALSE(manager.onPointerButton(at(5), base::PointF(20, 20), BTN_LEFT, true));
    EXPECT_FALSE(manager.onPointerButton(at(6), base::PointF(20, 20), BTN_LEFT, false));
}
