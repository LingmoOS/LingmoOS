// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <filesystem>
#include <linux/input-event-codes.h>
#include <wayland-server-core.h>

#include "base/file.h"
#include "base/geometry.h"
#include "base/json.h"
#include "base/strings.h"
#include "compositor/compositor.h"
#include "compositor/output.h"
#include "compositor/surface.h"
#include "core/main_loop.h"
#include "cursor/cursor.h"
#include "display/callback_native_display_delegate.h"
#include "display/display_configuration_controller.h"
#include "display/display_runtime.h"
#include "display/display_topology_coordinator.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/pointer.h"
#include "input/pointer_delegate.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/scene.h"
#include "session/session_controller.h"
#include "shm_test_util.h"
#include "wayland/protocols/wl_output.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;

namespace {

// Records a signal's emissions.
template<typename... Args>
class Spy
{
public:
    explicit Spy(base::Signal<Args...> &signal)
        : m_subscription(signal.connect([this](Args... values) { calls.emplace_back(values...); }))
    {
    }

    // Waits for the next emission.
    bool wait(int milliseconds)
    {
        const size_t count = calls.size();
        return test::runUntil([&] { return calls.size() > count; },
                              std::chrono::milliseconds(milliseconds));
    }

    size_t size() const { return calls.size(); }
    bool isEmpty() const { return calls.empty(); }
    template<size_t I>
    const auto &first() const { return std::get<I>(calls.front()); }
    template<size_t I>
    const auto &last() const { return std::get<I>(calls.back()); }

    std::vector<std::tuple<std::decay_t<Args>...>> calls;

private:
    base::Subscription m_subscription;
};

class PointerRecorder : public input::PointerDelegate
{
public:
    compositor::Surface *target = nullptr;
    compositor::Surface *entered = nullptr;
    base::PointF local;
    std::vector<bool> buttons;

    void onPointerDestroying(input::Pointer *) override { }

    bool canAcceptPointerEventsForSurface(compositor::Surface *surface) const override
    {
        return surface == target;
    }

    void onPointerEnter(compositor::Surface *surface, const base::PointF &point,
                        input::ButtonFlags) override
    {
        entered = surface;
        local = point;
    }

    void onPointerLeave(compositor::Surface *) override { }

    void onPointerMotion(input::Timestamp, const base::PointF &point) override { local = point; }

    void onPointerButton(input::Timestamp, input::ButtonFlags, bool pressed) override
    {
        buttons.push_back(pressed);
    }

    void onPointerScroll(input::Timestamp, const base::PointF &, bool) override { }

    void onFingerScrollStop(input::Timestamp) override { }

    void onPointerFrame() override { }
};

class DisplayRuntimeTest : public testing::Test
{
protected:
    scene::Scene scene;
    compositor::Compositor compositor {scene.root()};
    input::Seat seat {&compositor};
    input::InputManager input {&seat};
    wm::WindowManager wm {&compositor, &seat};
    cursor::Cursor cursor {seat, *scene.root()};
    display::DisplayManager manager;
    compositor::Output first;
    compositor::Output second;
    wl_display *server = wl_display_create();
    std::vector<wl_global *> globals {nullptr, nullptr};
    wl_global *primaryGlobal = nullptr;
    bool updating = false;
    bool failCommit = false;
    base::file::TemporaryDirectory directory {std::filesystem::temp_directory_path().string()};

    std::string filePath(const std::string &name) const { return directory.path() + '/' + name; }
    std::unique_ptr<display::DisplayRuntime> runtime;
    std::unique_ptr<display::CallbackNativeDisplayDelegate> delegate;
    std::unique_ptr<display::DisplayConfigurationController> controller;
    testsupport::FakeRenderer renderer;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 20);
    std::vector<std::unique_ptr<compositor::Surface>> surfaces;

    void SetUp() override
    {
        first.name = "Virtual-1";
        second.name = "Virtual-2";
        first.model = first.name;
        second.model = second.name;
        first.physicalSizeMm = second.physicalSizeMm = {0, 0};
        first.pixelSize = second.pixelSize = {1920, 1080};
        first.refreshMilliHz = second.refreshMilliHz = 60000;
        second.position = {1920, 0};
        wm.setDisplayManager(&manager);
        display::DisplayRuntime::initialize(manager, nullptr, {&first, &second}, "runtime-test",
                                            compositor, input);
        ASSERT_TRUE(manager.applied());
        runtime = std::make_unique<display::DisplayRuntime>(
                manager, nullptr, std::vector<compositor::Output *> {&first, &second}, globals,
                primaryGlobal, server, compositor, input, wm, scene, cursor, nullptr, updating);
        delegate = std::make_unique<display::CallbackNativeDisplayDelegate>(
                *manager.hardware(),
                [this](const auto &request, auto mode) {
                    if (failCommit && mode == display::NativeDisplayDelegate::ConfigureMode::Commit)
                        return display::ConfigurationResult {
                                false,
                                false,
                                {display::DisplayErrorCode::CommitFailed,
                                 "Injected commit failure",
                                 {}},
                                std::nullopt};
                    return runtime->configure(request, mode);
                },
                [this](const auto &ids, auto state) {
                    return runtime->setPower(ids, state);
                });
        makeController(filePath("display.json"));
    }

    void makeController(const std::string &path)
    {
        controller = std::make_unique<display::DisplayConfigurationController>(
                &manager, delegate.get(), display::DisplayPreferences(path));
    }

    bool replaceTestHardware(display::HardwareSnapshot hardware)
    {
        auto applied = *manager.applied();
        ++hardware.topologyGeneration;
        applied.topologyGeneration = hardware.topologyGeneration;
        applied.physicalDisplays = hardware.displays;
        if (!manager.installHardware(hardware) || !manager.installApplied(applied))
            return false;
        delegate->setSnapshot(hardware);
        return true;
    }

    void TearDown() override
    {
        controller.reset();
        delegate.reset();
        runtime.reset();
        seat.shutdown();
        for (auto *global : globals)
            wayland::removeOutputGlobal(global);
        wl_display_destroy(server);
    }

    display::DisplayConfiguration
    request(int modeIndex = 0, int numerator = 120,
            display::DisplayRotation rotation = display::DisplayRotation::Normal)
    {
        display::DisplayConfiguration result;
        result.base = {manager.hardware()->compositorInstanceId,
                       manager.hardware()->topologyGeneration, manager.applied()->revision};
        for (const auto &physical : manager.hardware()->displays)
            result.displays.push_back({physical.sessionId, true, physical.modes.at(modeIndex),
                                    *compositor::OutputScale::fromNumerator(numerator), rotation});
        result.primary = result.displays.front().display;
        result.placements = {
                {result.displays.back().display, result.primary, display::LayoutEdge::Left, -40}};
        return result;
    }

    display::ConfigurationResult apply(const display::DisplayConfiguration &request)
    {
        Spy completed(controller->operationFinished);
        const auto token = controller->apply(request, "internal", 0);
        EXPECT_TRUE(completed.isEmpty());
        if (!completed.wait(2000)) {
            ADD_FAILURE() << "No asynchronous operation completion";
            return {};
        }
        EXPECT_EQ(completed.size(), 1);
        EXPECT_EQ(completed.first<0>(), token);
        return completed.first<1>();
    }

    void expectPublished()
    {
        ASSERT_TRUE(manager.applied());
        const std::vector<compositor::Output *> models {&first, &second};
        std::vector<base::Rect> topology;
        for (size_t i = 0; i < models.size(); ++i) {
            const auto id = manager.hardware()->displays.at(i).sessionId;
            const auto *logical = manager.display(id);
            ASSERT_NE(logical, nullptr);
            EXPECT_EQ(models[i]->position, logical->bounds.topLeft());
            EXPECT_EQ(models[i]->logicalSize(), logical->bounds.size());
            const auto transform = manager.transformFor(id);
            ASSERT_TRUE(transform);
            EXPECT_EQ(transform->nativeSize(), models[i]->pixelSize);
            EXPECT_EQ(transform->logicalBounds(), logical->bounds);
            topology.push_back(logical->bounds);
            EXPECT_NE(globals[i], nullptr);
        }
        EXPECT_EQ(input.topology(), topology);
        EXPECT_EQ(wm.outputBounds(), manager.desktopBounds());
        EXPECT_EQ(scene.size(), manager.desktopBounds().size());
        EXPECT_FALSE(updating);
    }

    wm::Window *window(base::Rect bounds)
    {
        auto surface = compositor.createSurface();
        auto buffer = pool->createBuffer(bounds.size(), 0, bounds.width() * 4,
                                         render::PixelFormat::Bgrx8888, nullptr);
        surface->attach(buffer, {});
        surface->damage(base::Rect(base::Point(), bounds.size()));
        surface->commit();
        compositor.updateTextures(renderer);
        auto *window = wm.createWindow(surface.get(), {.requestedSize = bounds.size()});
        window->setBounds(bounds);
        surfaces.push_back(std::move(surface));
        return window;
    }
};

TEST_F(DisplayRuntimeTest, ConsecutiveChangesPublishBeforeObserversAndDeliverAbsoluteClicks)
{
    int notifications = 0;
    const auto published =
            manager.displaysChanged.connect([this, &notifications](uint64_t, uint64_t) {
        ++notifications;
        expectPublished();
    });
    int step = 0;
    for (auto rotation : {display::DisplayRotation::Normal, display::DisplayRotation::Rotate90,
                          display::DisplayRotation::Rotate180, display::DisplayRotation::Rotate270,
                          display::DisplayRotation::Normal}) {
        ASSERT_TRUE(apply(request(step % 3, step % 2 ? 180 : 120, rotation)).succeeded);
        expectPublished();
        const auto id = manager.hardware()->displays.back().sessionId;
        const auto *logical = manager.display(id);
        ASSERT_LT(logical->bounds.x(), 0);
        const base::Point centre = logical->bounds.center();
        const base::PointF target = centre;
        const base::Rect desktop = manager.desktopBounds();
        auto *client = window(base::Rect(centre - base::Point(10, 10), base::Size(20, 20)));
        PointerRecorder recorder;
        recorder.target = client->rootSurface();
        input::Pointer pointer(&recorder, &seat);
        input.onPointerMotionNormalized(input::Timestamp {},
                                        {(target.x() - desktop.x()) / desktop.width(),
                                         (target.y() - desktop.y()) / desktop.height()});
        EXPECT_EQ(input.pointerPosition(), target);
        EXPECT_EQ(recorder.entered, recorder.target);
        EXPECT_EQ(recorder.local, base::PointF(10, 10));
        input.onPointerButton(input::Timestamp {}, BTN_LEFT, true);
        input.onPointerButton(input::Timestamp {}, BTN_LEFT, false);
        EXPECT_EQ(recorder.buttons, (std::vector<bool> {true, false}));
        const auto native = manager.transformFor(id)->globalToNative(target);
        EXPECT_EQ(manager.transformFor(id)->nativeToGlobal(native), target);
        EXPECT_EQ(manager.outputLocalToGlobal(
                          id, *manager.globalToOutputLocal(id, target)),
                  target);
        ++step;
    }
    EXPECT_EQ(notifications, 5);
    EXPECT_EQ(manager.hardware()->displays.front().currentMode->pixelSize, base::Size(1920, 1080));
}

TEST_F(DisplayRuntimeTest, PublicationReportsTheLargestScaleForXwayland)
{
    std::vector<double> reported;
    runtime->maximumScaleChanged = [&](double scale) {
        reported.push_back(scale);
    };
    auto mixed = request();
    ASSERT_GE(mixed.displays.size(), 2);
    mixed.displays.back().scale = *compositor::OutputScale::fromNumerator(150);
    ASSERT_TRUE(apply(mixed).succeeded);
    ASSERT_FALSE(reported.empty());
    EXPECT_DOUBLE_EQ(reported.back(), 1.25);
    ASSERT_TRUE(apply(request()).succeeded);
    EXPECT_DOUBLE_EQ(reported.back(), 1.0);
}

TEST_F(DisplayRuntimeTest, FirstMinimizeResolvesDockAfterDisplayPublication)
{
    auto *client = window({100, 100, 300, 200});
    auto *dock = window({400, 900, 300, 80});
    const base::Rect icon(30, 10, 48, 48);
    client->setMinimizedGeometry(dock, icon);
    ASSERT_TRUE(apply(request(2, 180, display::DisplayRotation::Rotate90)).succeeded);
    const base::Rect bounds = manager.applied()->logicalDisplays.back().bounds;
    dock->setBounds(base::Rect(bounds.bottomLeft() - base::Point(0, 80), base::Size(300, 80)));
    client->minimize();
    EXPECT_EQ(client->minimizedGeometry(), icon.translated(dock->contentBounds().topLeft()));
    client->restore();
    wm.destroyWindow(dock);
    EXPECT_TRUE(client->minimizedGeometry().isEmpty());
}

TEST_F(DisplayRuntimeTest, CommitFailurePreservesPublishedRevisionAndPreferences)
{
    ASSERT_TRUE(apply(request(1)).succeeded);
    const auto before = *manager.applied();
    const std::string savedPath = filePath("display.json");
    const auto bytes = base::file::read(savedPath);
    ASSERT_TRUE(bytes);
    failCommit = true;
    const auto result = apply(request(2, 180));
    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(result.error.code, display::DisplayErrorCode::CommitFailed);
    EXPECT_EQ(manager.applied()->revision, before.revision);
    expectPublished();
    EXPECT_EQ(base::file::read(savedPath), bytes);
}

TEST_F(DisplayRuntimeTest, PersistenceFailureCompletesOnlyAfterRuntimeRollback)
{
    const auto before = *manager.applied();
    const std::string blockerPath = filePath("blocker");
    ASSERT_TRUE(base::file::writeAtomically(blockerPath, ""));
    makeController(blockerPath + "/display.json");
    const auto result = apply(request(2, 180));
    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(manager.applied()->revision, before.revision + 2);
    EXPECT_EQ(manager.applied()->physicalDisplays.front().currentMode->pixelSize,
              before.physicalDisplays.front().currentMode->pixelSize);
    EXPECT_EQ(manager.desktopBounds(), base::Rect(0, 0, 3840, 1080));
    expectPublished();
}

TEST_F(DisplayRuntimeTest, TemporaryPowerPreservesDisabledOutputsAndSavedModeIntent)
{
    auto candidate = request(1);
    candidate.displays.back().enabled = false;
    candidate.placements.clear();
    ASSERT_TRUE(apply(candidate).succeeded);
    const auto power = [this](display::NativeDisplayDelegate::PowerState state) {
        const auto revision = manager.applied()->revision;
        Spy completed(controller->operationFinished);
        controller->setPower({}, state);
        EXPECT_TRUE(completed.isEmpty());
        ASSERT_TRUE(completed.wait(2000));
        ASSERT_TRUE(completed.first<1>().succeeded);
        EXPECT_EQ(manager.applied()->revision, revision);
        EXPECT_EQ(manager.applied()->logicalDisplays.size(), 1);
        EXPECT_EQ(globals.back(), nullptr);
    };
    power(display::NativeDisplayDelegate::PowerState::Off);
    candidate = request(2, 180);
    candidate.displays.back().enabled = false;
    candidate.placements.clear();
    ASSERT_TRUE(apply(candidate).succeeded);
    power(display::NativeDisplayDelegate::PowerState::On);
    display::DisplayPreferences preferences(filePath("display.json"));
    const auto saved = preferences.restore(*manager.hardware(), manager.applied()->revision);
    ASSERT_TRUE(saved);
    EXPECT_FALSE(saved->displays.back().enabled);
    EXPECT_EQ(saved->displays.front().mode.pixelSize, base::Size(1280, 720));
    EXPECT_EQ(first.pixelSize, base::Size(1280, 720));
    EXPECT_FALSE(manager.applied()->physicalDisplays.back().currentMode);
}

TEST_F(DisplayRuntimeTest, LastDisplayPowerOffRequestsLockBeforePowerChange)
{
    const auto firstId = manager.applied()->physicalDisplays.front().sessionId;
    const auto secondId = manager.applied()->physicalDisplays.back().sessionId;
    int requests = 0;
    runtime->lockRequested = [&] {
        ++requests;
        EXPECT_FALSE(runtime->isPoweredOff(secondId));
        runtime->setSecureScanout(true);
    };
    ASSERT_TRUE(runtime->setPower({firstId}, display::NativeDisplayDelegate::PowerState::Off)
                        .succeeded);
    EXPECT_EQ(requests, 0);
    ASSERT_TRUE(runtime->setPower({secondId}, display::NativeDisplayDelegate::PowerState::Off)
                        .succeeded);
    EXPECT_EQ(requests, 1);
    EXPECT_TRUE(runtime->isSecureScanout());
    ASSERT_TRUE(runtime->setPower({}, display::NativeDisplayDelegate::PowerState::On).succeeded);
    EXPECT_EQ(requests, 1);
    EXPECT_TRUE(runtime->isSecureScanout());
    ASSERT_TRUE(runtime->setPower({}, display::NativeDisplayDelegate::PowerState::Off).succeeded);
    EXPECT_EQ(requests, 2);
    input.onPointerMotion({}, base::PointF(1, 0));
    EXPECT_FALSE(runtime->isPoweredOff(firstId));
    EXPECT_FALSE(runtime->isPoweredOff(secondId));
    EXPECT_TRUE(runtime->isSecureScanout());
    EXPECT_EQ(requests, 2);
}

TEST_F(DisplayRuntimeTest, GlobalPowerOffWakesOnPhysicalInputWithoutChangingLayout)
{
    const auto revision = manager.applied()->revision;
    const auto bounds = manager.desktopBounds();
    const auto firstId = manager.applied()->physicalDisplays.front().sessionId;
    const auto secondId = manager.applied()->physicalDisplays.back().sessionId;
    ASSERT_TRUE(runtime->setPower({}, display::NativeDisplayDelegate::PowerState::Off).succeeded);
    EXPECT_TRUE(runtime->isPoweredOff(firstId));
    EXPECT_TRUE(runtime->isPoweredOff(secondId));
    input.warpPointer(base::PointF(20, 20));
    EXPECT_TRUE(runtime->isPoweredOff(firstId));
    input.onPointerMotion(input::Timestamp {}, base::PointF(1, 0));
    EXPECT_FALSE(runtime->isPoweredOff(firstId));
    EXPECT_FALSE(runtime->isPoweredOff(secondId));
    EXPECT_EQ(manager.applied()->revision, revision);
    EXPECT_EQ(manager.desktopBounds(), bounds);
    ASSERT_TRUE(runtime->setPower({}, display::NativeDisplayDelegate::PowerState::Off).succeeded);
    input.onKey(input::Timestamp {}, 30, true);
    EXPECT_FALSE(runtime->isPoweredOff(firstId));
}

TEST_F(DisplayRuntimeTest, WakeDoesNotUndoAnExplicitlyPoweredOffLidDisplay)
{
    const auto firstId = manager.applied()->physicalDisplays.front().sessionId;
    const auto secondId = manager.applied()->physicalDisplays.back().sessionId;
    ASSERT_TRUE(runtime->setPower({firstId}, display::NativeDisplayDelegate::PowerState::Off)
                        .succeeded);
    ASSERT_TRUE(runtime->setPower({}, display::NativeDisplayDelegate::PowerState::Off).succeeded);
    input.onPointerMotion(input::Timestamp {}, base::PointF(1, 0));
    EXPECT_TRUE(runtime->isPoweredOff(firstId));
    EXPECT_FALSE(runtime->isPoweredOff(secondId));
}

TEST_F(DisplayRuntimeTest, PublicationRejectsReentrantConfigurationWithoutReplacingState)
{
    bool rejected = false;
    const auto started = manager.displayChangeStarted.connect([this, &rejected] {
        auto nested = *manager.applied();
        nested.revision += 100;
        display::DisplayError error;
        rejected = !manager.installApplied(nested, &error);
        EXPECT_EQ(error.code, display::DisplayErrorCode::Busy);
    });
    ASSERT_TRUE(apply(request(1)).succeeded);
    EXPECT_TRUE(rejected);
    EXPECT_EQ(manager.applied()->revision, 2u);
    expectPublished();
}

TEST_F(DisplayRuntimeTest, HotplugRestoresSavedMirrorAfterPrimaryRemovalAndZeroOutputs)
{
    auto mirror = request(1, 180);
    mirror.primary = mirror.displays.back().display;
    mirror.placements.clear();
    mirror.mirrorGroups = {{mirror.primary, mirror.displays.front().display}};
    ASSERT_TRUE(apply(mirror).succeeded);
    const auto original = *manager.hardware();
    const std::string savedPath = filePath("display.json");
    const auto bytes = base::file::read(savedPath);
    ASSERT_TRUE(bytes);
    std::vector<display::DisplaySnapshot> source = original.displays;
    display::DisplayTopologyCoordinator coordinator(
            &manager, delegate.get(), controller.get(),
            [&] {
                std::vector<compositor::Output *> models;
                for (const auto &physical : source) {
                    auto *model = physical.connectorName == first.name ? &first
                                                                                         : &second;
                    model->displayId = physical.sessionId.value();
                    models.push_back(model);
                }
                runtime->replaceOutputs(models);
                return std::optional(display::HardwareSnapshot {
                        original.compositorInstanceId, original.seatId,
                        manager.hardware()->topologyGeneration + 1, source});
            },
            display::DisplayPreferences(savedPath));
    const auto change = [&] {
        Spy done(coordinator.reconciled);
        coordinator.invalidate();
        ASSERT_TRUE(done.wait(2000));
        ASSERT_TRUE(done.last<0>());
    };
    source.pop_back();
    change();
    ASSERT_EQ(manager.applied()->logicalDisplays.size(), 1);
    EXPECT_EQ(manager.primaryDisplay()->id, original.displays.front().sessionId);
    EXPECT_EQ(input.topology().size(), 1);
    source.clear();
    change();
    EXPECT_TRUE(manager.applied()->logicalDisplays.empty());
    EXPECT_TRUE(input.topology().empty());
    EXPECT_EQ(primaryGlobal, nullptr);
    source = original.displays;
    std::reverse(source.begin(), source.end());
    change();
    ASSERT_EQ(manager.applied()->logicalDisplays.size(), 1);
    EXPECT_EQ(manager.primaryDisplay()->id, mirror.primary);
    EXPECT_EQ(manager.primaryDisplay()->destinations, mirror.mirrorGroups.front());
    EXPECT_EQ(base::file::read(savedPath), bytes);
}

TEST_F(DisplayRuntimeTest, UnpluggingADisplayMovesItsWindowsWholeToThePrimary)
{
    ASSERT_TRUE(manager.applied());
    const auto original = *manager.hardware();
    const auto *unplugged = manager.display(original.displays.back().sessionId);
    ASSERT_NE(unplugged, nullptr);
    ASSERT_FALSE(unplugged->primary);
    const base::Rect primary = manager.primaryDisplay()->bounds;
    auto *client = window(base::Rect(unplugged->bounds.topLeft() + base::Point(600, 285),
                                     base::Size(760, 450)));
    ASSERT_EQ(manager.displayForRect(client->bounds())->id, unplugged->id);

    std::vector<display::DisplaySnapshot> source = original.displays;
    display::DisplayTopologyCoordinator coordinator(
            &manager, delegate.get(), controller.get(),
            [&] {
                std::vector<compositor::Output *> models;
                for (const auto &physical : source) {
                    auto *model = physical.connectorName == first.name ? &first : &second;
                    model->displayId = physical.sessionId.value();
                    models.push_back(model);
                }
                runtime->replaceOutputs(models);
                return std::optional(display::HardwareSnapshot {
                        original.compositorInstanceId, original.seatId,
                        manager.hardware()->topologyGeneration + 1, source});
            },
            display::DisplayPreferences(filePath("display.json")));
    Spy done(coordinator.reconciled);
    source.pop_back();
    coordinator.invalidate();
    ASSERT_TRUE(done.wait(2000));
    ASSERT_EQ(manager.applied()->logicalDisplays.size(), 1);
    EXPECT_EQ(client->bounds(),
              base::Rect(primary.topLeft() + base::Point(600, 285), base::Size(760, 450)));
}

// The seat activation at startup reprobes, and SDL2 cannot survive its only
// output being withdrawn and re-announced while it recreates its window.
TEST_F(DisplayRuntimeTest, AReprobeKeepsTheOutputGlobalOfAConnectorThatStays)
{
    ASSERT_TRUE(apply(request()).succeeded);
    ASSERT_EQ(globals.size(), 2u);
    wl_global *const firstGlobal = globals[0];
    wl_global *const secondGlobal = globals[1];
    ASSERT_NE(firstGlobal, nullptr);
    ASSERT_NE(secondGlobal, nullptr);

    runtime->replaceOutputs({&second, &first});
    ASSERT_EQ(globals.size(), 2u);
    EXPECT_EQ(globals[0], secondGlobal);
    EXPECT_EQ(globals[1], firstGlobal);

    runtime->replaceOutputs({&first});
    ASSERT_EQ(globals.size(), 1u);
    EXPECT_EQ(globals[0], firstGlobal);

    compositor::Output other;
    other.name = "Virtual-3";
    other.model = other.name;
    runtime->replaceOutputs({&other, &first});
    ASSERT_EQ(globals.size(), 2u);
    EXPECT_EQ(globals[0], nullptr);
    EXPECT_EQ(globals[1], firstGlobal);
    runtime->replaceOutputs({&first, &second});
}

// Between a reprobe and the configuration it reconciles, the coordinator
// installs a baseline with no logical display. That disables nothing, so no
// client sees its output go away and come back.
TEST_F(DisplayRuntimeTest, TheRecoveryBaselineKeepsTheOutputGlobals)
{
    ASSERT_TRUE(apply(request()).succeeded);
    wl_global *const firstGlobal = globals[0];
    wl_global *const secondGlobal = globals[1];
    ASSERT_NE(firstGlobal, nullptr);
    ASSERT_NE(secondGlobal, nullptr);

    const auto hardware = *manager.hardware();
    manager.clearApplied(hardware.topologyGeneration);
    EXPECT_FALSE(manager.applied());
    EXPECT_EQ(globals[0], firstGlobal);
    EXPECT_EQ(globals[1], secondGlobal);
    EXPECT_EQ(primaryGlobal, nullptr);

    display::AppliedConfiguration empty;
    empty.compositorInstanceId = hardware.compositorInstanceId;
    empty.topologyGeneration = hardware.topologyGeneration;
    empty.revision = 1000;
    empty.physicalDisplays = hardware.displays;
    for (auto &physical : empty.physicalDisplays)
        physical.currentMode.reset();
    ASSERT_TRUE(manager.installApplied(empty));
    EXPECT_EQ(globals[0], firstGlobal);
    EXPECT_EQ(globals[1], secondGlobal);

    ASSERT_TRUE(apply(request()).succeeded);
    EXPECT_EQ(globals[0], firstGlobal);
    EXPECT_EQ(globals[1], secondGlobal);
    EXPECT_EQ(primaryGlobal, firstGlobal);
}

TEST_F(DisplayRuntimeTest, InvalidationCancelsQueuedCommitBeforeNativeExecution)
{
    int commits = 0;
    controller.reset();
    delegate = std::make_unique<display::CallbackNativeDisplayDelegate>(
            *manager.hardware(),
            [&](const auto &configuration, auto mode) {
                if (mode == display::NativeDisplayDelegate::ConfigureMode::Commit)
                    ++commits;
                else
                    core::mainLoop()->post([&] {
                        delegate->invalidateSnapshots();
                    });
                return runtime->configure(configuration, mode);
            },
            nullptr);
    makeController(filePath("display.json"));
    const auto revision = manager.applied()->revision;
    const auto result = apply(request(1));
    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(result.error.code, display::DisplayErrorCode::StaleConfiguration);
    EXPECT_EQ(commits, 0);
    EXPECT_EQ(manager.applied()->revision, revision);
}

TEST_F(DisplayRuntimeTest, TopologyWaitsForActiveOperationAndRecoversAfterProbeFailure)
{
    const auto original = *manager.hardware();
    int calls = 0;
    display::DisplayTopologyCoordinator coordinator(
            &manager, delegate.get(), controller.get(),
            [&]() -> std::optional<display::HardwareSnapshot> {
                EXPECT_FALSE(controller->busy());
                if (++calls == 1)
                    return std::nullopt;
                auto hardware = original;
                hardware.topologyGeneration = manager.hardware()->topologyGeneration + 1;
                return hardware;
            },
            display::DisplayPreferences(filePath("display.json")));
    Spy operation(controller->operationFinished);
    Spy done(coordinator.reconciled);
    controller->apply(request(1), "internal", 0);
    coordinator.invalidate();
    ASSERT_TRUE(done.wait(2000));
    EXPECT_TRUE(done.last<0>());
    EXPECT_EQ(calls, 2);
    ASSERT_GE(operation.size(), 2);
    EXPECT_FALSE(operation.first<1>().succeeded);
    EXPECT_FALSE(controller->busy());
    EXPECT_EQ(manager.hardware()->topologyGeneration, 2u);
}

TEST_F(DisplayRuntimeTest, StartGivesUnsavedDisplaysTheirRecommendedScale)
{
    auto hardware = *manager.hardware();
    for (auto &physical : hardware.displays) {
        // 13.3" at 1080p, 166 dpi.
        physical.connectionType = display::DisplayConnectionType::Internal;
        physical.physicalSizeMm = {294, 165};
    }
    display::DisplayTopologyCoordinator coordinator(
            &manager, delegate.get(), controller.get(),
            [&] {
                auto next = hardware;
                next.topologyGeneration = manager.hardware()->topologyGeneration + 1;
                return std::optional(next);
            },
            display::DisplayPreferences(filePath("display.json")));
    Spy done(coordinator.reconciled);
    // The layout initialize() made runs at 1; start() must not keep it.
    EXPECT_EQ(input.pointerPosition(), base::PointF(960, 540));
    coordinator.start();
    ASSERT_TRUE(done.wait(2000));
    ASSERT_TRUE(done.last<0>());
    ASSERT_EQ(manager.applied()->logicalDisplays.size(), 2);
    for (const auto &logical : manager.applied()->logicalDisplays)
        EXPECT_EQ(logical.scale.value(), 1.25);
    // Still the centre of the primary display, now 1536x864.
    EXPECT_EQ(input.pointerPosition(), base::PointF(768, 432));
}

TEST_F(DisplayRuntimeTest, HotplugInsertionAndReorderingPreserveIdsAndRejectConcurrentRequests)
{
    const auto original = *manager.hardware();
    auto third = original.displays.front();
    third.sessionId = display::DisplayId("injected-third");
    third.portKey = "virtual/Virtual-3";
    third.persistentKey = "virtual:virtual/Virtual-3";
    third.connectorName = "Virtual-3";
    compositor::Output model;
    model.name = third.connectorName;
    model.displayId = third.sessionId.value();
    model.pixelSize = third.currentMode->pixelSize;
    bool rejected = false;
    display::DisplayTopologyCoordinator coordinator(
            &manager, delegate.get(), controller.get(),
            [&] {
                runtime->replaceOutputs({&second, &model, &first});
                core::mainLoop()->post([&] {
                    Spy completed(controller->operationFinished);
                    controller->setPower({}, display::NativeDisplayDelegate::PowerState::Off);
                    ASSERT_EQ(completed.size(), 1);
                    rejected = completed.first<1>()
                                       .error.code
                            == display::DisplayErrorCode::Busy;
                });
                return std::optional(display::HardwareSnapshot {
                        original.compositorInstanceId,
                        original.seatId,
                        manager.hardware()->topologyGeneration + 1,
                        {original.displays.back(), third, original.displays.front()}});
            },
            display::DisplayPreferences(filePath("display.json")));
    Spy done(coordinator.reconciled);
    coordinator.invalidate();
    ASSERT_TRUE(done.wait(2000));
    ASSERT_EQ(done.size(), 1);
    EXPECT_TRUE(done.first<0>());
    EXPECT_TRUE(rejected);
    ASSERT_EQ(manager.applied()->logicalDisplays.size(), 3);
    EXPECT_NE(manager.display(original.displays.front().sessionId), nullptr);
    EXPECT_NE(manager.display(original.displays.back().sessionId), nullptr);
    EXPECT_NE(manager.display(third.sessionId), nullptr);
    EXPECT_EQ(first.displayId, original.displays.front().sessionId.value());
    EXPECT_EQ(input.topology().size(), 3);
}

TEST_F(DisplayRuntimeTest, LegacyNativePreferencesRestoreWithoutRewritingTheFile)
{
    auto legacyHardware = *manager.hardware();
    for (auto &physical : legacyHardware.displays) {
        physical.persistentKey = "session:" + physical.model;
        physical.portKey = "port:" + physical.connectorName;
    }
    const std::string path = filePath("legacy.json");
    display::DisplayPreferences preferences(path);
    ASSERT_TRUE(preferences.save(legacyHardware, request(2, 180)));
    auto root = base::json::Value::parse(base::file::read(path).value_or("")).value();
    const auto profile = root["topologies"].toObject().front().second;
    std::vector<std::string> oldKeys;
    for (const auto &physical : legacyHardware.displays)
        oldKeys.push_back(physical.persistentKey + '@' + physical.portKey);
    std::sort(oldKeys.begin(), oldKeys.end());
    root.insert("topologies",
                base::json::Value::object({{base::strings::join(oldKeys, "|"), profile}}));
    const auto bytes = std::optional(root.serialize(false));
    ASSERT_TRUE(base::file::writeAtomically(path, *bytes));
    const auto restored = preferences.restore(*manager.hardware(), manager.applied()->revision);
    ASSERT_TRUE(restored);
    EXPECT_EQ(restored->displays.front().mode.pixelSize, base::Size(1280, 720));
    EXPECT_EQ(restored->displays.front().scale.numerator(), 180u);
    EXPECT_EQ(restored->primary, manager.hardware()->displays.front().sessionId);
    EXPECT_EQ(base::file::read(path), bytes);
}

TEST_F(DisplayRuntimeTest, InvalidationDuringPublicationDoesNotPersistTheCandidate)
{
    bool invalidated = false;
    const auto changed = manager.displaysChanged.connect([&](uint64_t, uint64_t) {
        if (!invalidated) {
            invalidated = true;
            delegate->invalidateSnapshots();
        }
    });
    const auto result = apply(request(1, 180));
    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(result.error.code, display::DisplayErrorCode::StaleConfiguration);
    EXPECT_FALSE(std::filesystem::exists(filePath("display.json")));
    EXPECT_FALSE(controller->busy());
}

TEST_F(DisplayRuntimeTest, ZeroOutputsParkMaximizedWindowsDuringPanelReservationRemoval)
{
    auto *client = window({100, 100, 300, 200});
    client->maximize();
    const auto parked = client->bounds();
    const auto previous = *manager.applied();
    manager.clearApplied(manager.hardware()->topologyGeneration);
    wm.setExclusiveZone(this, wm::ScreenEdge::Top, 24, first.name);
    wm.clearExclusiveZone(this);
    EXPECT_EQ(client->bounds(), parked);
    auto restored = previous;
    ++restored.revision;
    ASSERT_TRUE(manager.installApplied(restored));
    EXPECT_EQ(client->bounds(), parked);
    client->restore();
    EXPECT_EQ(client->bounds(), base::Rect(100, 100, 300, 200));
}

TEST_F(DisplayRuntimeTest, RecoveryFallbackPublishesSelectedModesAndPreservesSavedIntent)
{
    const auto savedRequest = request();
    ASSERT_TRUE(apply(savedRequest).succeeded);
    const std::string preferencesPath = filePath("display.json");
    const auto savedBytes = base::file::read(preferencesPath);
    ASSERT_TRUE(savedBytes);
    auto hardware = *manager.hardware();
    ++hardware.topologyGeneration;
    hardware.displays[0].connectionType = display::DisplayConnectionType::Internal;
    hardware.displays[1].connectionType = display::DisplayConnectionType::External;
    hardware.displays[1].baseConnectorKey = "DP-1";
    std::vector<display::DisplayConfiguration> tested;
    std::vector<display::DisplayConfiguration> committed;
    runtime->setNativeConfigureForTesting([&](const auto &candidate, auto phase) {
        if (phase == display::NativeDisplayDelegate::ConfigureMode::Commit) {
            committed.push_back(candidate);
            return display::ConfigurationResult {true, false, {}, std::nullopt};
        }
        tested.push_back(candidate);
        if (candidate.displays[1].mode.pixelSize.width() > 1600)
            return display::ConfigurationResult {false,
                                                 false,
                                                 {display::DisplayErrorCode::TestFailed,
                                                  "Injected combined link limit",
                                                  {candidate.displays[1].display}},
                                                 std::nullopt};
        return display::ConfigurationResult {true, false, {}, std::nullopt};
    });
    display::DisplayTopologyCoordinator topology(
            &manager, delegate.get(), controller.get(),
            [&]() -> std::optional<display::HardwareSnapshot> {
                return hardware;
            },
            display::DisplayPreferences(filePath("display.json")));
    Spy operations(controller->operationFinished);
    Spy reconciled(topology.reconciled);
    topology.invalidate();
    ASSERT_TRUE(reconciled.wait(2000));
    ASSERT_TRUE(reconciled.first<0>());
    ASSERT_EQ(tested.size(), 2);
    ASSERT_EQ(committed.size(), 1);
    EXPECT_EQ(committed.front().displays[0].mode.pixelSize, base::Size(1920, 1080));
    EXPECT_EQ(committed.front().displays[1].mode.pixelSize, base::Size(1600, 900));
    EXPECT_FALSE(committed.front().allowModeFallback);
    ASSERT_EQ(operations.size(), 1);
    const auto result = operations.first<1>();
    EXPECT_TRUE(result.succeeded);
    EXPECT_TRUE(result.degraded);
    EXPECT_NE(result.error.message.find("2 test rounds"), std::string::npos);
    EXPECT_EQ(manager.applied()->physicalDisplays[1].currentMode->pixelSize, base::Size(1600, 900));
    expectPublished();
    EXPECT_EQ(base::file::read(preferencesPath), savedBytes);
    auto intent = display::DisplayPreferences(preferencesPath)
                          .restore(hardware, manager.applied()->revision);
    ASSERT_TRUE(intent);
    EXPECT_EQ(intent->displays[1].mode.pixelSize, base::Size(1920, 1080));
}

TEST_F(DisplayRuntimeTest, ExhaustedCombinedModesUseOneBoundedSingleOutputRescue)
{
    auto hardware = *manager.hardware();
    for (auto &physical : hardware.displays)
        physical.connectionType = display::DisplayConnectionType::External;
    ASSERT_TRUE(replaceTestHardware(hardware));
    int tests = 0;
    runtime->setNativeConfigureForTesting([&](const auto &candidate, auto phase) {
        if (phase == display::NativeDisplayDelegate::ConfigureMode::Commit)
            return display::ConfigurationResult {true, false, {}, std::nullopt};
        ++tests;
        int active = 0;
        for (const auto &request : candidate.displays)
            active += request.enabled;
        return display::ConfigurationResult {active == 1,
                                             false,
                                             {active == 1 ? display::DisplayErrorCode::None
                                                          : display::DisplayErrorCode::TestFailed,
                                              "Combined resource limit",
                                              {}},
                                             std::nullopt};
    });
    Spy completed(controller->operationFinished);
    controller->apply(request(), "topology", -1);
    ASSERT_TRUE(completed.wait(2000));
    const auto result = completed.first<1>();
    EXPECT_TRUE(result.succeeded);
    EXPECT_TRUE(result.degraded);
    EXPECT_EQ(tests, 8);
    EXPECT_EQ(manager.applied()->logicalDisplays.size(), 1);
    EXPECT_EQ(result.error.affectedDisplays.size(), 1);
    EXPECT_FALSE(std::filesystem::exists(filePath("display.json")));
}

TEST_F(DisplayRuntimeTest, StableDevicePathReadsLegacyCardPortPreferencesWithoutRewriting)
{
    auto oldHardware = *manager.hardware();
    for (int i = 0; i < oldHardware.displays.size(); ++i) {
        auto &physical = oldHardware.displays[i];
        physical.devicePath = "/dev/dri/card" + std::to_string(i);
        physical.portKey = physical.devicePath + "/DP-1";
        physical.persistentKey = "port:" + physical.portKey;
    }
    display::DisplayPreferences preferences(filePath("display.json"));
    ASSERT_TRUE(preferences.save(oldHardware, request(1, 150)));
    const std::string jsonPath = filePath("display.json");
    const auto bytes = base::file::read(jsonPath);
    ASSERT_TRUE(bytes);
    auto hardware = oldHardware;
    for (int i = 0; i < hardware.displays.size(); ++i) {
        auto &physical = hardware.displays[i];
        physical.portKey = "/sys/devices/pci" + std::to_string(i) + "/DP-1";
        physical.persistentKey = "port:" + physical.portKey;
    }
    const auto restored = preferences.restore(hardware, manager.applied()->revision);
    ASSERT_TRUE(restored);
    EXPECT_EQ(restored->displays.front().mode.pixelSize, base::Size(1600, 900));
    EXPECT_EQ(restored->displays.front().scale.numerator(), 150);
    EXPECT_EQ(base::file::read(jsonPath), bytes);
}

TEST_F(DisplayRuntimeTest, FailedNativeCompensationUnpublishesAndKeepsSavedIntent)
{
    ASSERT_TRUE(apply(request()).succeeded);
    const std::string jsonPath = filePath("display.json");
    const auto saved = base::file::read(jsonPath);
    ASSERT_TRUE(saved);
    runtime->setNativeConfigureForTesting([](const auto &, auto phase) {
        if (phase == display::NativeDisplayDelegate::ConfigureMode::TestOnly)
            return display::ConfigurationResult {true, false, {}, std::nullopt};
        return display::ConfigurationResult {false,
                                             false,
                                             {display::DisplayErrorCode::RevertFailed,
                                              "A device compensation failed",
                                              {}},
                                             std::nullopt};
    });
    const auto result = apply(request(1));
    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(result.error.code, display::DisplayErrorCode::RevertFailed);
    EXPECT_FALSE(manager.applied());
    EXPECT_EQ(primaryGlobal, nullptr);
    EXPECT_TRUE(scene.size().isEmpty());
    EXPECT_FALSE(controller->busy());
    EXPECT_EQ(base::file::read(jsonPath), saved);
}

TEST_F(DisplayRuntimeTest, ExactUserRequestCannotOptIntoRecoveryFallback)
{
    int tests = 0;
    runtime->setNativeConfigureForTesting([&](const auto &, auto) {
        ++tests;
        return display::ConfigurationResult {
                false,
                false,
                {display::DisplayErrorCode::TestFailed, "Rejected exact mode", {}},
                std::nullopt};
    });
    auto requested = request();
    requested.allowModeFallback = true;
    const auto result = apply(requested);
    EXPECT_FALSE(result.succeeded);
    EXPECT_FALSE(result.degraded);
    EXPECT_EQ(tests, 1);
    EXPECT_FALSE(std::filesystem::exists(filePath("display.json")));
}

TEST_F(DisplayRuntimeTest, RecoveryFallbackStopsOnAllocationFailureAndBoundsTests)
{
    auto hardware = *manager.hardware();
    for (auto &physical : hardware.displays)
        physical.connectionType = display::DisplayConnectionType::External;
    ASSERT_TRUE(replaceTestHardware(hardware));
    auto requested = request();
    requested.allowModeFallback = true;
    int tests = 0;
    auto code = display::DisplayErrorCode::ResourceExhausted;
    runtime->setNativeConfigureForTesting([&](const auto &, auto) {
        ++tests;
        return display::ConfigurationResult {
                false, false, {code, "Injected failure", {}}, std::nullopt};
    });
    auto result =
            runtime->configure(requested, display::NativeDisplayDelegate::ConfigureMode::TestOnly);
    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(tests, 1);
    EXPECT_EQ(result.error.code, display::DisplayErrorCode::ResourceExhausted);
    code = display::DisplayErrorCode::TestFailed;
    tests = 0;
    result = runtime->configure(requested, display::NativeDisplayDelegate::ConfigureMode::TestOnly);
    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(tests, 8);
    EXPECT_FALSE(result.selected);
}

TEST_F(DisplayRuntimeTest, RecoveryInvalidationAfterSelectedTestPreventsCommit)
{
    auto hardware = *manager.hardware();
    hardware.displays[1].connectionType = display::DisplayConnectionType::External;
    ASSERT_TRUE(replaceTestHardware(hardware));
    int tests = 0, commits = 0;
    runtime->setNativeConfigureForTesting([&](const auto &candidate, auto phase) {
        if (phase == display::NativeDisplayDelegate::ConfigureMode::Commit) {
            ++commits;
            return display::ConfigurationResult {true, false, {}, std::nullopt};
        }
        ++tests;
        if (candidate.displays[1].mode.pixelSize.width() > 1600)
            return display::ConfigurationResult {false,
                                                 false,
                                                 {display::DisplayErrorCode::TestFailed,
                                                  "Link limit",
                                                  {candidate.displays[1].display}},
                                                 std::nullopt};
        delegate->invalidateSnapshots();
        return display::ConfigurationResult {true, false, {}, std::nullopt};
    });
    const auto revision = manager.applied()->revision;
    Spy operations(controller->operationFinished);
    controller->apply(request(), "topology", -1);
    ASSERT_TRUE(operations.wait(2000));
    const auto result = operations.first<1>();
    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(result.error.code, display::DisplayErrorCode::StaleConfiguration);
    EXPECT_EQ(tests, 2);
    EXPECT_EQ(commits, 0);
    EXPECT_EQ(manager.applied()->revision, revision);
}

TEST_F(DisplayRuntimeTest, RecoveryInvalidationDuringPublicationCannotReportSuccess)
{
    auto candidate = request();
    auto connection = manager.displaysChanged.connect([&](uint64_t, uint64_t) {
        delegate->invalidateSnapshots();
    });
    Spy operations(controller->operationFinished);
    controller->apply(candidate, "topology", -1);
    ASSERT_TRUE(operations.wait(2000));
    const auto result = operations.first<1>();
    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(result.error.code, display::DisplayErrorCode::StaleConfiguration);
    EXPECT_FALSE(controller->busy());
    EXPECT_FALSE(std::filesystem::exists(filePath("display.json")));
    connection.reset();
}

TEST_F(DisplayRuntimeTest, HardwareReplacementDuringFallbackStopsFurtherNativeTests)
{
    auto hardware = *manager.hardware();
    hardware.displays[1].connectionType = display::DisplayConnectionType::External;
    ASSERT_TRUE(replaceTestHardware(hardware));
    int tests = 0;
    runtime->setNativeConfigureForTesting([&](const auto &, auto) {
        ++tests;
        auto replacement = *manager.hardware();
        ++replacement.topologyGeneration;
        EXPECT_TRUE(manager.installHardware(replacement));
        return display::ConfigurationResult {
                false,
                false,
                {display::DisplayErrorCode::TestFailed, "Hotplug during ioctl", {}},
                std::nullopt};
    });
    auto candidate = request();
    candidate.allowModeFallback = true;
    const auto result =
            runtime->configure(candidate, display::NativeDisplayDelegate::ConfigureMode::TestOnly);
    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(result.error.code, display::DisplayErrorCode::StaleConfiguration);
    EXPECT_EQ(tests, 1);
    EXPECT_FALSE(result.selected);
}

TEST_F(DisplayRuntimeTest, RecoveryCommitFailureDoesNotPublishOrRetryLowerModes)
{
    auto hardware = *manager.hardware();
    hardware.displays[1].connectionType = display::DisplayConnectionType::External;
    ASSERT_TRUE(replaceTestHardware(hardware));
    int tests = 0, commits = 0;
    runtime->setNativeConfigureForTesting([&](const auto &candidate, auto phase) {
        if (phase == display::NativeDisplayDelegate::ConfigureMode::Commit) {
            ++commits;
            return display::ConfigurationResult {false,
                                                 false,
                                                 {display::DisplayErrorCode::CommitFailed,
                                                  "Commit rejected",
                                                  {}},
                                                 std::nullopt};
        }
        ++tests;
        const bool accepted = candidate.displays[1].mode.pixelSize.width() <= 1600;
        return display::ConfigurationResult {
                accepted,
                false,
                {accepted ? display::DisplayErrorCode::None : display::DisplayErrorCode::TestFailed,
                 "Combined test",
                 {candidate.displays[1].display}},
                std::nullopt};
    });
    const auto revision = manager.applied()->revision;
    Spy operations(controller->operationFinished);
    controller->apply(request(), "topology", -1);
    ASSERT_TRUE(operations.wait(2000));
    const auto result = operations.first<1>();
    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(result.error.code, display::DisplayErrorCode::CommitFailed);
    EXPECT_EQ(tests, 2);
    EXPECT_EQ(commits, 1);
    EXPECT_EQ(manager.applied()->revision, revision);
    EXPECT_FALSE(std::filesystem::exists(filePath("display.json")));
}

TEST(DisplayIdentity, ReorderPortMigrationDuplicatesMissingEdidAndVirtualModes)
{
    display::DisplayIdentity identity;
    display::DisplaySnapshot a;
    a.portKey = "card/DP-1";
    a.edidKey = "vendor-product-serial";
    a.connectionType = display::DisplayConnectionType::External;
    auto b = a;
    b.portKey = "card/DP-2";
    std::vector<display::DisplaySnapshot> pair {a, b};
    identity.assign(pair);
    const auto firstId = pair.front().sessionId;
    const auto firstKey = pair.front().persistentKey;
    const auto secondId = pair.back().sessionId;
    EXPECT_NE(firstId, secondId);
    EXPECT_NE(firstKey, pair.back().persistentKey);
    std::reverse(pair.begin(), pair.end());
    identity.assign(pair);
    EXPECT_EQ(pair.front().sessionId, firstId);
    pair.pop_back();
    identity.assign(pair);
    EXPECT_EQ(pair.front().persistentKey, firstKey);
    display::DisplayIdentity restarted;
    restarted.rememberPersistentKeys(std::vector<std::string> {firstKey});
    restarted.assign(pair);
    EXPECT_EQ(pair.front().persistentKey, firstKey);
    display::DisplayIdentity unique;
    pair = {a};
    unique.assign(pair);
    const auto portableId = pair.front().sessionId;
    const auto portableKey = pair.front().persistentKey;
    pair.front().portKey = "card/HDMI-A-1";
    unique.assign(pair);
    EXPECT_EQ(pair.front().sessionId, portableId);
    EXPECT_EQ(pair.front().persistentKey, portableKey);
    pair.front().edidKey.clear();
    unique.assign(pair);
    const auto noEdid = pair.front().sessionId;
    EXPECT_NE(noEdid, portableId);
    unique.assign(pair);
    EXPECT_EQ(pair.front().sessionId, noEdid);
    pair.front().connectionType = display::DisplayConnectionType::Virtual;
    unique.assign(pair);
    const auto virtualId = pair.front().sessionId;
    pair.front().edidKey = "changed-mode-dependent-edid";
    unique.assign(pair);
    EXPECT_EQ(pair.front().sessionId, virtualId);
}

} // namespace

TEST_F(DisplayRuntimeTest, SecureBarrierUsesPhysicalDisplayIdsForLockAndUnlock)
{
    session::SecureContainer secure(scene.root()->addChild(std::make_unique<scene::Node>()));
    session::SessionController session(&secure);
    session.setDisplayConfiguration(*manager.applied());
    ASSERT_NE(first.displayId, first.name);
    ASSERT_TRUE(session.requestLock());
    session.notifyOutputPresented(first.name);
    EXPECT_EQ(session.state(), session::SessionState::Locking);
    session.notifyOutputPresented(first.displayId);
    EXPECT_EQ(session.state(), session::SessionState::Locking);
    session.notifyOutputPresented(second.displayId);
    ASSERT_EQ(session.state(), session::SessionState::Locked);
    ASSERT_TRUE(session.requestUnlock());
    session.notifyOutputPresented(second.displayId);
    EXPECT_TRUE(session.isBlocked());
    session.notifyOutputPresented(first.displayId);
    EXPECT_EQ(session.state(), session::SessionState::Active);
}

TEST_F(DisplayRuntimeTest, MirroredSecureBarrierWaitsForEveryPhysicalDestination)
{
    auto mirror = request(1, 180);
    mirror.placements.clear();
    mirror.mirrorGroups = {{mirror.primary, mirror.displays.back().display}};
    ASSERT_TRUE(apply(mirror).succeeded);
    ASSERT_EQ(compositor.outputViews().size(), 1u);
    session::SecureContainer secure(scene.root()->addChild(std::make_unique<scene::Node>()));
    session::SessionController session(&secure);
    session.setDisplayConfiguration(*manager.applied());
    ASSERT_EQ(secure.outputs().size(), 2);
    ASSERT_TRUE(session.requestLock());
    session.notifyOutputPresented(first.displayId);
    EXPECT_EQ(session.state(), session::SessionState::Locking);
    session.notifyOutputPresented(second.displayId);
    EXPECT_EQ(session.state(), session::SessionState::Locked);
}
