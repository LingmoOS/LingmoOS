// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The clipboard history's picker as the compositor draws it: its window,
// place, glass, keys, pointer and paste, headless on a fake renderer.

#include <chrono>
#include <unistd.h>

#include <linux/input-event-codes.h>

#include <gtest/gtest.h>

#include "clipboard/selection.h"
#include "clipboard/stored_source.h"
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shell/clipboard/clipboard_history.h"
#include "shell/clipboard/clipboard_picker.h"
#include "shell/clipboard/clipboard_picker_view.h"
#include "shell/shell.h"
#include "shell/shell_input.h"
#include "shell/shell_window.h"
#include "shm_test_util.h"
#include "temporary_directory.h"
#include "ui/label.h"
#include "ui/list_view.h"
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
constexpr int kStatusBar = 32;

std::shared_ptr<clipboard::StoredSource> textSource(const std::string &text)
{
    auto source = std::make_shared<clipboard::StoredSource>();
    source->offer("text/plain");
    source->payloads["text/plain"] = text;
    return source;
}

// Reads one type of the selection as a client would.
std::string receive(clipboard::DataSource &source, const std::string &type)
{
    int fds[2];
    EXPECT_EQ(pipe(fds), 0);
    source.send(type, fds[1]);
    std::string data;
    char buffer[4096];
    test::runUntil([&] {
        const ssize_t count = read(fds[0], buffer, sizeof(buffer));
        if (count > 0) {
            data.append(buffer, size_t(count));
            return false;
        }
        return count == 0;
    });
    close(fds[0]);
    return data;
}

class ClipboardPickerTest : public testing::Test
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
    int statusBar = 0;
    wm::Window *application = nullptr;
    shell::ClipboardPicker *picker = nullptr;
    int ms = 0;

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
        manager.setExclusiveZone(&statusBar, wm::ScreenEdge::Top, kStatusBar);
        input.setBounds(kOutput.size());
        input.setEventSink(&manager);
        shell::ShellEnvironment environment {&compositor, &manager, &seat};
        environment.servicesSettings = directory.filePath("services.conf");
        shell = std::make_unique<shell::Shell>(environment);
        application = openApplication(base::Rect(100, 100, 400, 300));

        auto history = std::make_unique<shell::ClipboardHistory>(nullptr, *seat.selection(),
                                                                 directory.filePath("history"));
        shell::ClipboardHistory *backend = history.get();
        picker = shell->addComponent(std::make_unique<shell::ClipboardPicker>(shell.get(), std::move(history)));
        ASSERT_TRUE(test::runUntil([backend] { return backend->store().isLoaded(); }));
        int count = 0;
        for (const char *text : {"alpha", "beta", "gamma"}) {
            seat.selection()->setSource(textSource(text));
            ++count;
            ASSERT_TRUE(test::runUntil([&] { return backend->model().count() == count; }));
        }
        frame();
    }

    void TearDown() override
    {
        shell.reset();
        seat.shutdown();
    }

    void frame() { compositor.updateTextures(renderer); }

    wm::Window *openApplication(const base::Rect &bounds)
    {
        surfaces.push_back(compositor.createSurface());
        compositor::Surface *surface = surfaces.back().get();
        surface->attach(pool->createBuffer(bounds.size(), 0, bounds.width() * 4, render::PixelFormat::Bgrx8888,
                                           nullptr),
                        {});
        surface->commit();
        frame();
        wm::Window *window = manager.createWindow(surface, {.requestedSize = bounds.size(), .applicationId = "app"});
        window->setBounds(bounds);
        window->activate();
        return window;
    }

    void move(double x, double y) { input.onPointerMotionAbsolute(at(++ms), {x, y}); }

    void click(double x, double y)
    {
        move(x, y);
        input.onPointerButton(at(++ms), BTN_LEFT, true);
        input.onPointerButton(at(++ms), BTN_LEFT, false);
    }

    void key(uint32_t code)
    {
        input.onKey(at(++ms), code, true);
        input.onKey(at(++ms), code, false);
    }

    void shiftKey(uint32_t code)
    {
        input.onKey(at(++ms), KEY_LEFTSHIFT, true);
        key(code);
        input.onKey(at(++ms), KEY_LEFTSHIFT, false);
    }

    shell::ClipboardPickerView *view() const { return picker->view(); }
    shell::ClipboardHistoryModel &model() const { return picker->history().model(); }

    // A row's centre on screen.
    base::PointF rowCentre(int row) const
    {
        const base::Rect bounds = picker->window()->drawnBounds();
        return base::PointF(bounds.x() + 12 + 100, bounds.y() + 65 + 36 * row + 18);
    }
};

} // namespace

TEST_F(ClipboardPickerTest, OpensCentredInTheWorkAreaBorrowingTheKeyboard)
{
    EXPECT_FALSE(picker->isOpen());
    picker->history().toggle();
    ASSERT_TRUE(picker->isOpen());
    frame();
    wm::Window *window = picker->window()->wmWindow();
    ASSERT_NE(nullptr, window);
    EXPECT_EQ(wm::WindowRole::Picker, window->role());
    // 720 x 440 in the middle of what the bar leaves.
    EXPECT_EQ(base::Rect(360, kStatusBar + (960 - kStatusBar - 440) / 2, 720, 440), window->bounds());
    EXPECT_EQ(picker->geometry(), window->bounds());

    // The application stays active; the picker has the keys.
    EXPECT_EQ(application, manager.activation()->activeWindow());
    EXPECT_EQ(window, manager.keyboardGrab());
    EXPECT_EQ(picker->window(), shell->input()->keyboardWindow());
    EXPECT_TRUE(view()->filterField()->hasFocus());

    // PopupGlass in PanelStyle.glassTint over the whole card.
    EXPECT_TRUE(picker->window()->surface()->glass().enabled);
    EXPECT_EQ(base::Rect(0, 0, 720, 440), picker->window()->surface()->backgroundBlurRect());
    EXPECT_EQ(18, picker->window()->surface()->backgroundBlurRadius());

    // Newest first, the first selected.
    EXPECT_EQ(3u, view()->list()->rowCount());
    EXPECT_EQ(0, view()->currentIndex());
    EXPECT_EQ("gamma", model().title(0));
    EXPECT_EQ("gamma", view()->preview()->text());
    EXPECT_FALSE(view()->emptyLabel()->isVisible());

    // The shortcut again closes it, and the application has the keys again.
    picker->history().toggle();
    EXPECT_FALSE(picker->isOpen());
    EXPECT_EQ(nullptr, manager.keyboardGrab());
    EXPECT_EQ(application->rootSurface(), seat.focusedSurface());
}

TEST_F(ClipboardPickerTest, KeysMoveTheSelectionFilterAndClose)
{
    picker->open();
    frame();
    key(KEY_DOWN);
    EXPECT_EQ(1, view()->currentIndex());
    EXPECT_EQ("beta", view()->preview()->text());
    key(KEY_END);
    EXPECT_EQ(2, view()->currentIndex());
    key(KEY_DOWN);
    EXPECT_EQ(2, view()->currentIndex());
    key(KEY_PAGEUP);
    EXPECT_EQ(0, view()->currentIndex());
    key(KEY_PAGEDOWN);
    EXPECT_EQ(2, view()->currentIndex());
    key(KEY_HOME);
    EXPECT_EQ(0, view()->currentIndex());

    // Typing filters and selects the first row; Escape clears the filter.
    key(KEY_DOWN);
    key(KEY_B);
    key(KEY_E);
    EXPECT_EQ("be", view()->filterField()->text());
    EXPECT_EQ(1, model().count());
    EXPECT_EQ(0, view()->currentIndex());
    EXPECT_EQ("beta", view()->preview()->text());
    key(KEY_Z);
    EXPECT_EQ(0, model().count());
    EXPECT_TRUE(view()->emptyLabel()->isVisible());
    EXPECT_EQ("No Results", view()->emptyLabel()->text());
    key(KEY_ESC);
    EXPECT_TRUE(view()->filterField()->text().empty());
    EXPECT_EQ(3, model().count());
    EXPECT_TRUE(picker->isOpen());

    // Escape with no filter closes; opening again starts afresh.
    key(KEY_ESC);
    EXPECT_FALSE(picker->isOpen());
    EXPECT_EQ(application->rootSurface(), seat.focusedSurface());
    picker->open();
    EXPECT_EQ(0, view()->currentIndex());
}

TEST_F(ClipboardPickerTest, ReturnPastesTheEntryIntoTheApplication)
{
    picker->open();
    frame();
    key(KEY_DOWN);
    key(KEY_ENTER);
    ASSERT_TRUE(test::runUntil([this] { return !picker->isOpen(); }));
    EXPECT_EQ(application, manager.activation()->activeWindow());
    EXPECT_EQ(application->rootSurface(), seat.focusedSurface());
    // On the clipboard, and on top of the history.
    const auto source = seat.selection()->source();
    ASSERT_TRUE(source);
    EXPECT_EQ("beta", receive(*source, "text/plain"));
    EXPECT_EQ("beta", model().title(0));
    EXPECT_EQ(3, model().count());

    // Shift+Return only copies.
    picker->open();
    key(KEY_END);
    shiftKey(KEY_ENTER);
    ASSERT_TRUE(test::runUntil([this] { return !picker->isOpen(); }));
    EXPECT_EQ("alpha", receive(*seat.selection()->source(), "text/plain"));
}

TEST_F(ClipboardPickerTest, APressOutsideClosesIt)
{
    picker->open();
    frame();
    ASSERT_TRUE(picker->isOpen());
    click(200, 200);
    EXPECT_FALSE(picker->isOpen());
    EXPECT_EQ(application, manager.activation()->activeWindow());
}

TEST_F(ClipboardPickerTest, DeleteRemovesAndShiftDeleteClearsAfterAsking)
{
    picker->open();
    frame();
    key(KEY_DOWN);
    key(KEY_DELETE);
    EXPECT_EQ(2, model().count());
    EXPECT_EQ("gamma", model().title(0));
    EXPECT_EQ("alpha", model().title(1));
    EXPECT_EQ(1, view()->currentIndex());

    shiftKey(KEY_DELETE);
    EXPECT_TRUE(view()->isConfirmingClear());
    EXPECT_FALSE(view()->filterField()->isEnabled());
    // Escape asks no more and keeps the history and the picker.
    key(KEY_ESC);
    EXPECT_FALSE(view()->isConfirmingClear());
    EXPECT_TRUE(picker->isOpen());
    EXPECT_EQ(2, model().count());

    shiftKey(KEY_DELETE);
    key(KEY_ENTER);
    EXPECT_FALSE(view()->isConfirmingClear());
    EXPECT_EQ(0, model().count());
    EXPECT_TRUE(view()->emptyLabel()->isVisible());
    EXPECT_EQ("Clipboard Is Empty", view()->emptyLabel()->text());
    // Nothing more to clear.
    shiftKey(KEY_DELETE);
    EXPECT_FALSE(view()->isConfirmingClear());
}

TEST_F(ClipboardPickerTest, ThePointerSelectsOnlyOnceItMovesAndAClickPastes)
{
    picker->open();
    frame();
    // The pointer's first position over the card only counts as where it was.
    const base::PointF first = rowCentre(1);
    move(first.x(), first.y());
    EXPECT_EQ(0, view()->currentIndex());
    move(first.x() + 1, first.y());
    EXPECT_EQ(1, view()->currentIndex());
    const base::PointF third = rowCentre(2);
    move(third.x(), third.y());
    EXPECT_EQ(2, view()->currentIndex());

    click(first.x(), first.y());
    ASSERT_TRUE(test::runUntil([this] { return !picker->isOpen(); }));
    EXPECT_EQ("beta", receive(*seat.selection()->source(), "text/plain"));
}
