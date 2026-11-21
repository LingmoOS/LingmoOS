// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The screen capture interface as the compositor draws it: its overlays,
// toolbar, selection, window picking, timer, what a capture becomes and
// org.lingmo.Screenshot, headless on a fake renderer and a fake grabber.

#include <chrono>
#include <cmath>
#include <filesystem>

#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include "base/settings.h"
#include "clipboard/selection.h"
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "dbus_test_client.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shell/common/file_thread.h"
#include "shell/screenshot/capture_notifier.h"
#include "shell/screenshot/capture_overlay.h"
#include "shell/screenshot/capture_settings.h"
#include "shell/screenshot/capture_toolbar.h"
#include "shell/screenshot/screen_capture.h"
#include "shell/screenshot/screen_grabber.h"
#include "shell/screenshot/screen_recording.h"
#include "shell/shell.h"
#include "shell/shell_data.h"
#include "shell/shell_window.h"
#include "shell_test_util.h"
#include "shm_test_util.h"
#include "temporary_directory.h"
#include "ui/label.h"
#include "ui/menu.h"
#include "wm/shell_surface.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;

namespace {

input::Timestamp at(int ms)
{
    return input::Timestamp(std::chrono::milliseconds(ms));
}

const base::Rect kOutput(0, 0, 1440, 960);

// Pictures of the size asked for, filled with one colour, a turn later.
class FakeGrabber final : public shell::ScreenGrabber
{
public:
    void grabOutputs(const std::vector<std::string> &outputs, bool cursor, Done done) override
    {
        calls.push_back("outputs " + std::to_string(outputs.size()) + (cursor ? " cursor" : ""));
        std::vector<ui::Bitmap> images;
        for (size_t i = 0; i < outputs.size(); ++i) {
            ui::Bitmap image(outputSize, ui::Bitmap::Format::Argb32Premultiplied);
            image.fill(0xff336699u);
            images.push_back(image);
        }
        core::mainLoop()->post([done, images] { done(images); });
    }

    void grabWindow(wm::Window *window, bool cursor, Done done) override
    {
        calls.push_back(std::string("window ") + (window ? std::to_string(window->id()) : "none")
                        + (cursor ? " cursor" : ""));
        ui::Bitmap image(base::Size(100, 50), ui::Bitmap::Format::Argb32Premultiplied);
        image.fill(0xff00ff00u);
        core::mainLoop()->post([done, image, window] { done({window ? image : ui::Bitmap()}); });
    }

    base::Size outputSize {2160, 1440};
    std::vector<std::string> calls;
};

class ScreenshotTest : public testing::Test
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
    test::FakeTaskWindows *windows = nullptr;
    std::unique_ptr<shell::Shell> shell;
    FakeGrabber *grabber = nullptr;
    shell::ScreenCapture *capture = nullptr;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 22);
    std::vector<std::unique_ptr<compositor::Surface>> surfaces;
    std::vector<std::unique_ptr<wm::ShellSurface>> shellSurfaces;
    int time = 0;

    void SetUp() override
    {
        shell::setShellDataDirectoryForTesting(LINGMO_SOURCE_DIR "/data/shell");
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
        input.setBounds(kOutput.size());
        input.setEventSink(&manager);
        makeShell(nullptr);
    }

    void TearDown() override
    {
        shell.reset();
        shellSurfaces.clear();
        surfaces.clear();
        shell::waitForFileThread();
        seat.shutdown();
    }

    void makeShell(base::dbus::Connection *bus, const std::string &settings = {})
    {
        capture = nullptr;
        shell.reset();
        shell::ShellEnvironment environment {&compositor, &manager, &seat, bus};
        environment.servicesSettings = directory.filePath("services.conf");
        shell = std::make_unique<shell::Shell>(environment);
        auto tasks = std::make_unique<test::FakeTaskWindows>();
        windows = tasks.get();
        shell->setTaskWindows(std::move(tasks));
        shell->setApplications(std::make_unique<test::FakeApplications>());
        directory.write("shell.conf", settings);
        shell::ScreenCaptureServices services;
        auto fake = std::make_unique<FakeGrabber>();
        grabber = fake.get();
        services.grabber = std::move(fake);
        services.selection = seat.selection();
        services.settingsPath = directory.filePath("shell.conf");
        services.recorder = "/bin/false";
        capture = shell->addComponent(std::make_unique<shell::ScreenCapture>(shell.get(), std::move(services)));
        capture->notifier().playSound = [this](const std::string &) { ++shutters; };
        ASSERT_TRUE(test::runUntil([this] { return capture->settings().isReady(); }));
    }

    void frame() { compositor.updateTextures(renderer); }

    void move(double x, double y) { input.onPointerMotionAbsolute(at(++time), {x, y}); }

    void press(uint32_t button = BTN_LEFT) { input.onPointerButton(at(++time), button, true); }
    void release(uint32_t button = BTN_LEFT) { input.onPointerButton(at(++time), button, false); }

    void click(double x, double y, uint32_t button = BTN_LEFT)
    {
        move(x, y);
        press(button);
        release(button);
    }

    void key(uint32_t code, bool pressed)
    {
        input.onKey(at(++time), code, pressed);
        test::processPendingEvents();
    }

    void tap(uint32_t code)
    {
        key(code, true);
        key(code, false);
    }

    shell::CaptureOverlay *overlay() const
    {
        return capture->overlays().empty() ? nullptr : capture->overlays().front().get();
    }

    // The toolbar's rect on the screen.
    base::RectF toolbarRect() const
    {
        const ui::RectF rect = overlay()->toolbarRect();
        return {rect.x, rect.y, rect.width, rect.height};
    }

    base::PointF centreOf(shell::ToolbarButton *button) const
    {
        const ui::RectF rect = button->geometry();
        const base::RectF toolbar = toolbarRect();
        return {toolbar.x() + rect.x + rect.width / 2, toolbar.y() + rect.y + rect.height / 2};
    }

    // An application window as a client's would be, and its task.
    wm::Window *openWindow(const base::Rect &bounds)
    {
        surfaces.push_back(compositor.createSurface());
        compositor::Surface *surface = surfaces.back().get();
        auto shellSurface = std::make_unique<wm::ShellSurface>(&manager, surface);
        shellSurface->setToplevel();
        shellSurface->setApplicationId("terminal");
        surface->attach(pool->createBuffer(bounds.size(), 0, bounds.width() * 4, render::PixelFormat::Bgrx8888,
                                           nullptr),
                        {});
        surface->damage(base::Rect(base::Point(), bounds.size()));
        surface->commit();
        wm::Window *window = shellSurface->window();
        window->setBounds(bounds);
        shellSurfaces.push_back(std::move(shellSurface));
        shell::TaskWindow task;
        task.id = window->id();
        task.appId = "terminal";
        task.geometry = bounds;
        windows->add(task);
        return window;
    }

    int shutters = 0;
};

} // namespace

TEST_F(ScreenshotTest, CaptureModeCoversTheOutputWithTheToolbarAndASelection)
{
    capture->startCaptureMode();
    frame();
    ASSERT_TRUE(capture->isActive());
    ASSERT_EQ(1u, capture->overlays().size());
    shell::ShellWindow *window = overlay()->window();
    ASSERT_NE(nullptr, window->wmWindow());
    EXPECT_EQ(wm::WindowRole::Capture, window->wmWindow()->role());
    EXPECT_EQ(kOutput, window->wmWindow()->bounds());
    // The keyboard, for Escape and Return.
    EXPECT_EQ(window->surface(), seat.focusedSurface());

    // A portion of half the screen in the middle, picture of a portion.
    EXPECT_EQ(shell::ScreenCapture::Image, capture->type());
    EXPECT_EQ(shell::ScreenCapture::Region, capture->source());
    EXPECT_EQ(base::Rect(359, 239, 720, 480), capture->selection());

    // Close | three pictures | three recordings | Options and Capture: 48
    // high, centred, 32 above the bottom without a dock.
    const base::RectF toolbar = toolbarRect();
    EXPECT_DOUBLE_EQ(48, toolbar.height());
    EXPECT_DOUBLE_EQ(960 - 32 - 48, toolbar.y());
    EXPECT_NEAR(720, toolbar.x() + toolbar.width() / 2, 0.51);
    shell::CaptureToolbar *bar = overlay()->toolbar();
    EXPECT_DOUBLE_EQ(40, bar->closeButton()->geometry().width);
    EXPECT_DOUBLE_EQ(36, bar->closeButton()->geometry().height);
    EXPECT_TRUE(bar->modeButton(shell::ScreenCapture::Image, shell::ScreenCapture::Region)->isChecked());
    EXPECT_FALSE(bar->modeButton(shell::ScreenCapture::Image, shell::ScreenCapture::Screen)->isChecked());
    EXPECT_TRUE(bar->actionButton()->isVisible());

    // The compositor's glass under the toolbar, 18 round.
    const compositor::Surface *surface = window->surface();
    EXPECT_TRUE(surface->glass().enabled);
    EXPECT_EQ(18, surface->backgroundBlurRadius());
    EXPECT_EQ(int(std::lround(toolbar.y())), surface->backgroundBlurRect().y());

    // The same shortcut closes it again, keeping the selection.
    capture->startCaptureMode();
    EXPECT_FALSE(capture->isActive());
    EXPECT_EQ(nullptr, window->wmWindow());
    EXPECT_EQ(base::Rect(359, 239, 720, 480), capture->settings().lastSelection());
    shell::waitForFileThread();
    const base::Settings settings(directory.filePath("shell.conf"));
    EXPECT_EQ((base::Settings::Rect {359, 239, 720, 480}), settings.rectValue("Screenshot/LastSelection"));
}

TEST_F(ScreenshotTest, TheSelectionIsDrawnMovedAndResized)
{
    capture->startCaptureMode();
    frame();
    // A new one outside the old.
    move(100, 100);
    EXPECT_EQ(shell::CaptureOverlay::Shape::Crosshair, overlay()->shape());
    press();
    move(150, 140);
    move(300, 200);
    release();
    EXPECT_EQ(base::Rect(100, 100, 200, 100), capture->selection());

    // Inside moves it.
    move(200, 150);
    EXPECT_EQ(shell::CaptureOverlay::Shape::OpenHand, overlay()->shape());
    press();
    EXPECT_EQ(shell::CaptureOverlay::Shape::ClosedHand, overlay()->shape());
    move(250, 170);
    release();
    EXPECT_EQ(base::Rect(150, 120, 200, 100), capture->selection());

    // The bottom right corner resizes; edges and corners show their arrows.
    move(350, 220);
    EXPECT_EQ(shell::CaptureOverlay::Shape::ResizeDiagonal, overlay()->shape());
    move(250, 220);
    EXPECT_EQ(shell::CaptureOverlay::Shape::ResizeVertical, overlay()->shape());
    move(350, 160);
    EXPECT_EQ(shell::CaptureOverlay::Shape::ResizeHorizontal, overlay()->shape());
    move(350, 220);
    press();
    move(400, 300);
    release();
    EXPECT_EQ(base::Rect(150, 120, 250, 180), capture->selection());

    // Dragged to the far edges it takes them whole.
    move(300, 200);
    press();
    move(1439.7, 959.7);
    release();
    EXPECT_EQ(1440, capture->selection().right());
    EXPECT_EQ(960, capture->selection().bottom());

    // Over the toolbar, the arrow.
    const base::RectF toolbar = toolbarRect();
    move(toolbar.x() + 3, toolbar.y() + 24);
    EXPECT_EQ(shell::CaptureOverlay::Shape::Arrow, overlay()->shape());
}

TEST_F(ScreenshotTest, CaptureTakesThePortionAtTheScreensDensityToAFile)
{
    capture->settings().setCustomFolder(false, directory.filePath("Shots"));
    capture->startCaptureMode();
    frame();
    capture->setSelection(base::Rect(100, 50, 200, 100));
    std::vector<std::string> saved;
    capture->onSaved = [&saved](const std::vector<std::string> &paths) { saved = paths; };
    click(centreOf(overlay()->toolbar()->actionButton()).x(), centreOf(overlay()->toolbar()->actionButton()).y());
    EXPECT_FALSE(capture->isActive());
    ASSERT_TRUE(test::runUntil([&saved] { return !saved.empty(); }));
    EXPECT_EQ((std::vector<std::string> {"outputs 1"}), grabber->calls);
    ASSERT_EQ(1u, saved.size());
    const std::string name = std::filesystem::path(saved.front()).filename().string();
    EXPECT_TRUE(name.starts_with("Screenshot ")) << name;
    EXPECT_TRUE(name.ends_with(".png")) << name;
    EXPECT_NE(std::string::npos, name.find(" at ")) << name;
    // 1.5 device pixels a logical one.
    const ui::Bitmap picture = ui::Bitmap::decodeFile(saved.front());
    EXPECT_EQ(base::Size(300, 150), picture.size());
    EXPECT_EQ(1, shutters);

    // A second one the same second takes the next name.
    saved.clear();
    capture->startCaptureMode();
    tap(KEY_ENTER);
    ASSERT_TRUE(test::runUntil([&saved] { return !saved.empty(); }));
    const std::string second = std::filesystem::path(saved.front()).filename().string();
    EXPECT_NE(name, second);
}

TEST_F(ScreenshotTest, CopiesGoToTheClipboardAsPng)
{
    capture->startCaptureMode();
    frame();
    capture->setSelection(base::Rect(0, 0, 40, 20));
    // Ctrl+Return.
    key(KEY_LEFTCTRL, true);
    tap(KEY_ENTER);
    key(KEY_LEFTCTRL, false);
    EXPECT_FALSE(capture->isActive());
    ASSERT_TRUE(test::runUntil([this] { return seat.selection()->source() != nullptr; }));
    const auto source = seat.selection()->source();
    EXPECT_EQ((std::vector<std::string> {"image/png"}), source->formats());
    int pipes[2];
    ASSERT_EQ(0, pipe2(pipes, O_NONBLOCK));
    source->send("image/png", pipes[1]);
    std::string png;
    ASSERT_TRUE(test::runUntil([&] {
        char buffer[4096];
        ssize_t n;
        while ((n = read(pipes[0], buffer, sizeof(buffer))) > 0)
            png.append(buffer, size_t(n));
        return n == 0;
    }));
    close(pipes[0]);
    EXPECT_EQ(base::Size(60, 30), ui::Bitmap::decode(png).size());
}

TEST_F(ScreenshotTest, TheCrosshairCapturesOnRelease)
{
    capture->settings().setCustomFolder(false, directory.filePath("Shots"));
    std::vector<std::string> saved;
    capture->onSaved = [&saved](const std::vector<std::string> &paths) { saved = paths; };
    capture->takePartialScreenshot(false);
    frame();
    EXPECT_TRUE(capture->isActive());
    EXPECT_FALSE(capture->isToolbarStyle());
    EXPECT_FALSE(overlay()->toolbar()->isVisible());
    move(200, 200);
    EXPECT_EQ("200\n200", overlay()->coordinates());
    press();
    move(250, 260);
    move(300, 300);
    EXPECT_EQ("100\n100", overlay()->coordinates());
    EXPECT_EQ(shell::CaptureOverlay::Shape::Crosshair, overlay()->shape());
    release();
    EXPECT_FALSE(capture->isActive());
    ASSERT_TRUE(test::runUntil([&saved] { return !saved.empty(); }));
    EXPECT_EQ(base::Size(150, 150), ui::Bitmap::decodeFile(saved.front()).size());

    // Space picks a window instead; a click takes it, with its shadow.
    wm::Window *window = openWindow({400, 300, 300, 200});
    capture->takePartialScreenshot(false);
    tap(KEY_SPACE);
    EXPECT_EQ(shell::ScreenCapture::Window, capture->source());
    move(500, 400);
    EXPECT_EQ(shell::CaptureOverlay::Shape::Camera, overlay()->shape());
    EXPECT_EQ(base::Rect(400, 300, 300, 200), capture->hoveredWindow());
    EXPECT_EQ(14, capture->hoveredWindowRadius());
    ASSERT_TRUE(overlay()->window()->cursor());
    EXPECT_FALSE(overlay()->window()->cursor()->image.isNull());
    saved.clear();
    click(500, 400);
    ASSERT_TRUE(test::runUntil([&saved] { return !saved.empty(); }));
    EXPECT_EQ("window " + std::to_string(window->id()), grabber->calls.back());
}

TEST_F(ScreenshotTest, TheTimerCountsDownOverWhatItTakesAndLetsInputThrough)
{
    capture->settings().setCustomFolder(false, directory.filePath("Shots"));
    capture->settings().setTimer(5);
    std::vector<std::string> saved;
    capture->onSaved = [&saved](const std::vector<std::string> &paths) { saved = paths; };
    capture->startCaptureMode();
    frame();
    capture->setMode(shell::ScreenCapture::Image, shell::ScreenCapture::Screen);
    EXPECT_FALSE(overlay()->toolbar()->modeButton(0, 2)->isChecked());
    capture->perform(false);
    EXPECT_TRUE(capture->isActive());
    EXPECT_EQ(5, capture->countdown());
    EXPECT_EQ(kOutput, capture->countdownArea());
    // The toolbar goes; input reaches what is under the interface.
    EXPECT_FALSE(overlay()->toolbar()->isVisible());
    ASSERT_TRUE(overlay()->window()->surface()->inputRegion());
    EXPECT_TRUE(overlay()->window()->surface()->inputRegion()->isEmpty());
    test::runFor(std::chrono::milliseconds(1100));
    EXPECT_EQ(4, capture->countdown());
    ASSERT_TRUE(test::runUntil([&saved] { return !saved.empty(); }, std::chrono::seconds(8)));
    EXPECT_FALSE(capture->isActive());
    EXPECT_EQ(0, capture->countdown());
}

TEST_F(ScreenshotTest, OptionsListsWhereItGoesTheTimerAndThePointer)
{
    capture->startCaptureMode();
    frame();
    shell::CaptureToolbar *bar = overlay()->toolbar();
    // Hovering a button shows its name above it.
    const base::PointF window = centreOf(bar->modeButton(0, 1));
    move(window.x(), window.y());
    ASSERT_TRUE(bar->tooltip()->isShown());
    EXPECT_EQ("Capture Selected Window", bar->tooltip()->text());

    const base::PointF options = centreOf(bar->optionsButton());
    click(options.x(), options.y());
    ASSERT_TRUE(bar->isOptionsOpen());
    EXPECT_FALSE(bar->tooltip()->isShown());
    EXPECT_TRUE(bar->optionsButton()->isChecked());
    EXPECT_EQ(shell::CaptureOverlay::Shape::Arrow, overlay()->shape());
    // A press on Options again only closes it.
    click(options.x(), options.y());
    EXPECT_FALSE(bar->isOptionsOpen());
    click(options.x(), options.y());
    EXPECT_TRUE(bar->isOptionsOpen());
    // Escape closes the menu, then the interface.
    tap(KEY_ESC);
    test::processPendingEvents();
    EXPECT_FALSE(bar->isOptionsOpen());
    EXPECT_TRUE(capture->isActive());
    tap(KEY_ESC);
    EXPECT_FALSE(capture->isActive());
}

// The tooltip's text fills the label on one line, the same gap on both sides
// of it in the bubble, at whole and fractional scales alike.
TEST_F(ScreenshotTest, TooltipsFitTheirTextWithEqualSidePadding)
{
    for (const int numerator : {120, 180}) {
        compositor.setOutputScale(*compositor::OutputScale::fromNumerator(numerator));
        makeShell(nullptr);
        capture->startCaptureMode();
        frame();
        shell::CaptureToolbar *bar = overlay()->toolbar();
        const double scale = numerator / 120.0;
        for (int type = 0; type < 2; ++type) {
            for (int source = 0; source < 3; ++source) {
                const base::PointF centre = centreOf(bar->modeButton(type, source));
                move(centre.x(), centre.y());
                shell::CaptureTooltip *tooltip = bar->tooltip();
                ASSERT_TRUE(tooltip->isShown());
                ASSERT_DOUBLE_EQ(scale, tooltip->window()->scale());
                const ui::Label *label = tooltip->label();
                const ui::RectF text = label->geometry();
                const ui::RectF body = tooltip->bodyRect();
                SCOPED_TRACE(tooltip->text() + " at " + std::to_string(scale));
                EXPECT_EQ(1u, label->lineCount());
                EXPECT_DOUBLE_EQ(label->sizeHint().width, text.width);
                const double left = (text.x - body.x) * scale;
                const double right = (body.right() - text.right()) * scale;
                EXPECT_NEAR(left, right, 0.5);
            }
        }
        tap(KEY_ESC);
        ASSERT_FALSE(capture->isActive());
    }
}

TEST_F(ScreenshotTest, TheToolbarIsDraggedAndRemembered)
{
    capture->startCaptureMode();
    frame();
    const base::RectF toolbar = toolbarRect();
    // Its background, between the last button and the end.
    const double x = toolbar.x() + toolbar.width() - 2;
    const double y = toolbar.y() + 24;
    move(x, y);
    press();
    move(x - 5, y - 5);
    EXPECT_EQ(toolbar, toolbarRect());
    move(x - 100, y - 300);
    release();
    EXPECT_NEAR(toolbar.x() - 100, toolbarRect().x(), 1e-9);
    EXPECT_NEAR(toolbar.y() - 300, toolbarRect().y(), 1e-9);
    // The selection stays.
    EXPECT_EQ(base::Rect(359, 239, 720, 480), capture->selection());
    const base::Rect saved = capture->settings().toolbarGeometry();
    EXPECT_EQ(int(std::lround(toolbar.y() - 300)), saved.y());
    capture->cancel();
    capture->startCaptureMode();
    EXPECT_NEAR(saved.y(), toolbarRect().y(), 1e-9);
}

TEST_F(ScreenshotTest, ServesOrgLingmoScreenshot)
{
    base::dbus::Connection *bus = base::dbus::Connection::session(core::mainLoop());
    ASSERT_NE(nullptr, bus);
    makeShell(bus);
    ASSERT_TRUE(capture->registerService());

    test::DBusTestClient client;
    const std::string xml = client.introspect("org.lingmo.Screenshot", "/Screenshot");
    EXPECT_NE(std::string::npos, xml.find("<interface name=\"org.lingmo.Screenshot\">")) << xml;
    EXPECT_NE(std::string::npos, xml.find("name=\"toClipboard\"")) << xml;
    EXPECT_NE(std::string::npos, xml.find("name=\"recording\"")) << xml;
    EXPECT_FALSE(client.getProperty("org.lingmo.Screenshot", "/Screenshot", "org.lingmo.Screenshot", "recording")
                         .value<base::dbus::Variant>()
                         ->value<bool>(true));

    client.call("org.lingmo.Screenshot", "/Screenshot", "org.lingmo.Screenshot", "startCaptureMode");
    EXPECT_TRUE(capture->isActive());
    client.call("org.lingmo.Screenshot", "/Screenshot", "org.lingmo.Screenshot", "startCaptureMode");
    EXPECT_FALSE(capture->isActive());
    client.call("org.lingmo.Screenshot", "/Screenshot", "org.lingmo.Screenshot", "takePartialScreenshot", true);
    EXPECT_TRUE(capture->isActive());
    EXPECT_FALSE(capture->isToolbarStyle());
    // Every screen at once, the interface gone.
    client.call("org.lingmo.Screenshot", "/Screenshot", "org.lingmo.Screenshot", "takeScreenshot", true);
    EXPECT_FALSE(capture->isActive());
    ASSERT_TRUE(test::runUntil([this] { return seat.selection()->source() != nullptr; }));
    EXPECT_EQ("outputs 1", grabber->calls.back());
}

// lingmo-screenshot captures nothing itself: the shell takes the picture,
// with the shutter and a notification, and hands it over.
TEST_F(ScreenshotTest, TheCommandLineToolGetsItsPictureFromTheShell)
{
    base::dbus::Connection *bus = base::dbus::Connection::session(core::mainLoop());
    ASSERT_NE(nullptr, bus);
    makeShell(bus);
    ASSERT_TRUE(capture->registerService());
    const auto run = [](std::vector<std::string> arguments) {
        const pid_t child = fork();
        if (child == 0) {
            std::vector<char *> argv {const_cast<char *>(SCREENSHOT_EXECUTABLE)};
            for (std::string &argument : arguments)
                argv.push_back(argument.data());
            argv.push_back(nullptr);
            execv(SCREENSHOT_EXECUTABLE, argv.data());
            _exit(127);
        }
        int status = -1;
        EXPECT_TRUE(test::runUntil([&] { return waitpid(child, &status, WNOHANG) == child; },
                                   std::chrono::seconds(10)));
        return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    };

    const std::string path = directory.filePath("screen.png");
    EXPECT_EQ(0, run({"--cursor", path}));
    EXPECT_EQ("outputs 1 cursor", grabber->calls.back());
    EXPECT_EQ(1, shutters);
    const ui::Bitmap image = ui::Bitmap::decodeFile(path);
    EXPECT_EQ(grabber->outputSize, image.size());
    EXPECT_EQ(0xff336699u, image.pixel(0, 0));

    const std::string missing = directory.filePath("missing.png");
    EXPECT_EQ(1, run({"--output", "HDMI-9", missing}));
    EXPECT_FALSE(std::filesystem::exists(missing));
    EXPECT_EQ(1, shutters);
}

TEST(ScreenshotHelpers, PicturesAreTurnedUpright)
{
    ui::Bitmap image(base::Size(3, 2), ui::Bitmap::Format::Argb32Premultiplied);
    image.fill(0xff000000u);
    image.setPixel(0, 0, 0xffff0000u);
    // A 90 degree output: the buffer's top left shows at the top right.
    const ui::Bitmap turned = shell::uprightImage(image, 1);
    ASSERT_EQ(base::Size(2, 3), turned.size());
    EXPECT_EQ(0xffff0000u, turned.pixel(1, 0));
    const ui::Bitmap flipped = shell::uprightImage(image, 4);
    ASSERT_EQ(base::Size(3, 2), flipped.size());
    EXPECT_EQ(0xffff0000u, flipped.pixel(2, 0));
    EXPECT_EQ(image, shell::uprightImage(image, 0));
}

TEST(ScreenshotHelpers, TheRecorderIsToldWhatToRecord)
{
    shell::ScreenRecording::Target region;
    region.output = "eDP-1";
    region.region = base::Rect(10, 20, 300, 200);
    region.microphone = "alsa_input.analog";
    EXPECT_EQ((std::vector<std::string> {"--output", "eDP-1", "--region", "10,20,300,200", "--cursor",
                                         "--microphone", "alsa_input.analog", "/tmp/a.mp4"}),
              shell::ScreenRecording::arguments(region, "/tmp/a.mp4"));
    shell::ScreenRecording::Target window;
    window.window = 42;
    window.cursor = false;
    EXPECT_EQ((std::vector<std::string> {"--window", "lingmo-42", "/tmp/b.mp4"}),
              shell::ScreenRecording::arguments(window, "/tmp/b.mp4"));
}

// The recorder captures on a connection the compositor made for it, never by
// finding the public socket.
TEST(ScreenshotHelpers, TheRecorderRunsOnTheConnectionItIsGiven)
{
    test::TemporaryDirectory directory;
    ASSERT_TRUE(directory.isValid());
    ASSERT_TRUE(directory.write("recorder", "#!/bin/sh\n"
                                            "[ \"$WAYLAND_SOCKET\" = 3 ] && [ -z \"$WAYLAND_DISPLAY\" ] || exit 3\n"
                                            "printf connected >&3\n"
                                            "echo started\n"
                                            "read line\n"));
    ASSERT_EQ(0, chmod(directory.filePath("recorder").c_str(), 0755));
    int compositorEnd = -1;
    shell::ScreenRecording recording(directory.filePath("recorder"), {"WAYLAND_DISPLAY=wayland-0", "PATH=/usr/bin:/bin"},
                                     [&compositorEnd] {
                                         int fds[2];
                                         if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds))
                                             return -1;
                                         compositorEnd = fds[0];
                                         return fds[1];
                                     });
    std::optional<bool> ok;
    base::Subscription finished = recording.finished.connect([&](bool success, const std::string &) { ok = success; });
    bool started = false;
    base::Subscription changed = recording.recordingChanged.connect([&] { started = true; });
    shell::ScreenRecording::Target target;
    target.output = "eDP-1";
    ASSERT_TRUE(recording.start(target, directory.filePath("a.mp4")));
    ASSERT_GE(compositorEnd, 0);
    ASSERT_TRUE(test::runUntil([&] { return started || ok.has_value(); }));
    ASSERT_FALSE(ok.has_value());
    char text[16] = {};
    EXPECT_EQ(9, recv(compositorEnd, text, sizeof(text) - 1, MSG_DONTWAIT));
    EXPECT_EQ(std::string("connected"), text);
    recording.stop();
    ASSERT_TRUE(test::runUntil([&] { return ok.has_value(); }));
    EXPECT_TRUE(*ok);
    // The recorder held the only other end.
    char byte = 0;
    EXPECT_EQ(0, recv(compositorEnd, &byte, 1, 0));
    close(compositorEnd);

    // No connection, no recorder.
    shell::ScreenRecording refused(directory.filePath("recorder"), {}, [] { return -1; });
    std::optional<bool> refusedOk;
    base::Subscription refusedFinished =
            refused.finished.connect([&](bool success, const std::string &) { refusedOk = success; });
    refused.start(target, directory.filePath("b.mp4"));
    EXPECT_FALSE(refused.isRunning());
    EXPECT_EQ(std::optional<bool>(false), refusedOk);
}

TEST(ScreenshotHelpers, AThumbnailCoversTheCentresSquare)
{
    const ui::Bitmap wide(base::Size(2160, 1440), ui::Bitmap::Format::Argb32Premultiplied);
    EXPECT_EQ(base::Size(198, 132), shell::CaptureNotifier::thumbnail(wide).size());
    const ui::Bitmap small(base::Size(100, 400), ui::Bitmap::Format::Argb32Premultiplied);
    EXPECT_EQ(base::Size(100, 400), shell::CaptureNotifier::thumbnail(small).size());
}
