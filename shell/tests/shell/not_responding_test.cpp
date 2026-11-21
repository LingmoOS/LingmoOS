// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The sheets that offer force quit for an application that stopped
// responding, as the compositor draws them, headless on a fake renderer.

#include <chrono>

#include <linux/input-event-codes.h>

#include <gtest/gtest.h>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shell/notresponding/not_responding_sheets.h"
#include "shell/shell.h"
#include "shell/shell_window.h"
#include "shell_test_util.h"
#include "shm_test_util.h"
#include "temporary_directory.h"
#include "ui/dialog_window.h"
#include "wm/activation.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;

namespace {

input::Timestamp at(int ms)
{
    return input::Timestamp(std::chrono::milliseconds(ms));
}

const base::Rect kOutput(0, 0, 1440, 960);
constexpr int64_t kPid = 4242;

class NotRespondingTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node *surfaceContainer = scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    compositor::Compositor compositor {surfaceContainer};
    testsupport::FakeRenderer renderer;
    input::Seat seat {&compositor};
    input::InputManager input {&seat};
    wm::WindowManager manager {&compositor, &seat};
    test::TemporaryDirectory directory;
    std::unique_ptr<shell::Shell> shell;
    std::vector<std::unique_ptr<compositor::Surface>> surfaces;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 22);
    shell::NotRespondingSheets *sheets = nullptr;
    int kills = 0;
    std::vector<int64_t> ended;
    int ms = 0;

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
        manager.setForceQuitProcess([this](int64_t pid) { ended.push_back(pid); });
        input.setBounds(kOutput.size());
        input.setEventSink(&manager);
        shell::ShellEnvironment environment {&compositor, &manager, &seat};
        environment.servicesSettings = directory.filePath("services.conf");
        shell = std::make_unique<shell::Shell>(environment);
        auto applications = std::make_unique<test::FakeApplications>();
        applications->addSimple("terminal", "Terminal", "utilities-terminal");
        applications->scan();
        shell->setApplications(std::move(applications));
        sheets = shell->addComponent(std::make_unique<shell::NotRespondingSheets>(shell.get()));
    }

    void TearDown() override
    {
        shell.reset();
        seat.shutdown();
    }

    void frame() { compositor.updateTextures(renderer); }

    // An application window of kPid that answers pings until told otherwise.
    wm::Window *openApplication(const base::Rect &bounds)
    {
        surfaces.push_back(compositor.createSurface());
        compositor::Surface *surface = surfaces.back().get();
        surface->attach(pool->createBuffer(bounds.size(), 0, bounds.width() * 4, render::PixelFormat::Bgrx8888,
                                           nullptr),
                        {});
        surface->commit();
        frame();
        wm::Window *window = manager.createWindow(
                surface, {.requestedSize = bounds.size(), .applicationId = "terminal", .processId = kPid});
        window->setTitle("scratch");
        window->setBounds(bounds);
        window->setPingCallback([] {});
        window->setKillCallback([this] { ++kills; });
        window->activate();
        return window;
    }

    void click(double x, double y)
    {
        input.onPointerMotionAbsolute(at(++ms), {x, y});
        input.onPointerButton(at(++ms), BTN_LEFT, true);
        input.onPointerButton(at(++ms), BTN_LEFT, false);
    }

    void key(uint32_t code)
    {
        input.onKey(at(++ms), code, true);
        input.onKey(at(++ms), code, false);
    }

    // Where a view of the sheet is on screen.
    base::PointF centreOf(const ui::View *view) const
    {
        const base::Rect bounds = sheets->sheet(kPid)->drawnBounds();
        const ui::RectF rect = view->geometry();
        return base::PointF(bounds.x() + rect.x + rect.width / 2, bounds.y() + rect.y + rect.height / 2);
    }
};

} // namespace

TEST_F(NotRespondingTest, APressOnAWindowThatStoppedAnsweringHangsASheetFromIt)
{
    wm::Window *window = openApplication(base::Rect(292, 175, 856, 546));
    // Nothing while it answers.
    click(400, 400);
    EXPECT_EQ(0u, sheets->sheetCount());

    window->setResponsive(false);
    click(400, 400);
    ASSERT_EQ(1u, sheets->sheetCount());
    shell::ShellWindow *sheet = sheets->sheet(kPid);
    ASSERT_NE(nullptr, sheet);
    frame();
    wm::Window *sheetWindow = sheet->wmWindow();
    ASSERT_NE(nullptr, sheetWindow);
    EXPECT_EQ(wm::WindowRole::Sheet, sheetWindow->role());
    EXPECT_EQ(window, sheetWindow->transientParent());
    EXPECT_EQ(window->id(), sheets->parentWindow(kPid));
    EXPECT_EQ("“Terminal” is not responding", sheets->view(kPid)->title());
    EXPECT_EQ("“Terminal” is not responding", sheetWindow->title());

    // 400 wide, centred on the window, and it has the keyboard.
    const base::Rect bounds = sheetWindow->bounds();
    EXPECT_EQ(400, bounds.width());
    EXPECT_GE(bounds.height(), 130);
    EXPECT_LE(bounds.height(), 150);
    EXPECT_EQ(292 + (856 - 400) / 2, bounds.x());
    EXPECT_EQ(175 + (546 - bounds.height()) / 2, bounds.y());
    EXPECT_EQ(sheetWindow, manager.activation()->activeWindow());
    EXPECT_TRUE(sheets->view(kPid)->waitButton()->hasFocus());

    // Wait: the sheet goes, and the application is left alone until it answers.
    key(KEY_ESC);
    EXPECT_EQ(0u, sheets->sheetCount());
    EXPECT_TRUE(sheets->isWaiting(kPid));
    test::runFor(std::chrono::milliseconds(10));
    window->requestClose();
    EXPECT_EQ(0u, sheets->sheetCount());
    EXPECT_EQ(0, kills);

    // Once it answers, the next hang asks again; Return waits too.
    window->setResponsive(true);
    EXPECT_FALSE(sheets->isWaiting(kPid));
    window->setResponsive(false);
    click(400, 400);
    ASSERT_EQ(1u, sheets->sheetCount());
    key(KEY_ENTER);
    EXPECT_EQ(0u, sheets->sheetCount());
    EXPECT_TRUE(sheets->isWaiting(kPid));
}

TEST_F(NotRespondingTest, ForceQuitEndsTheApplication)
{
    wm::Window *window = openApplication(base::Rect(292, 175, 856, 546));
    window->setResponsive(false);
    // Closing a window that stopped answering asks first.
    window->requestClose();
    ASSERT_EQ(1u, sheets->sheetCount());
    EXPECT_EQ(0, kills);
    frame();

    const base::PointF button = centreOf(sheets->view(kPid)->forceQuitButton());
    click(button.x(), button.y());
    EXPECT_EQ(1, kills);
    EXPECT_EQ(std::vector<int64_t> {kPid}, ended);
    EXPECT_EQ(0u, sheets->sheetCount());
    EXPECT_FALSE(sheets->isWaiting(kPid));
    // The closed sheet goes on the next loop turn.
    test::runFor(std::chrono::milliseconds(10));
}

TEST_F(NotRespondingTest, OneSheetPerApplicationFollowsItsWindowsAndGoesWhenItAnswers)
{
    wm::Window *first = openApplication(base::Rect(100, 100, 500, 400));
    wm::Window *second = openApplication(base::Rect(700, 300, 600, 500));
    first->setResponsive(false);
    second->setResponsive(false);

    click(200, 200);
    ASSERT_EQ(1u, sheets->sheetCount());
    EXPECT_EQ(first->id(), sheets->parentWindow(kPid));
    frame();
    // Another window of the same application takes the sheet.
    click(1000, 700);
    ASSERT_EQ(1u, sheets->sheetCount());
    EXPECT_EQ(second->id(), sheets->parentWindow(kPid));
    ASSERT_NE(nullptr, sheets->sheet(kPid)->wmWindow());
    EXPECT_EQ(second, sheets->sheet(kPid)->wmWindow()->transientParent());
    EXPECT_EQ(700 + (600 - 400) / 2, sheets->sheet(kPid)->wmWindow()->bounds().x());

    // Any window answering closes it.
    first->setResponsive(true);
    EXPECT_EQ(0u, sheets->sheetCount());

    // So does its window going away.
    click(1000, 700);
    ASSERT_EQ(1u, sheets->sheetCount());
    manager.destroyWindow(second);
    EXPECT_EQ(0u, sheets->sheetCount());
    test::runFor(std::chrono::milliseconds(10));
}
