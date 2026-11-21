// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <linux/input-event-codes.h>

#include <gtest/gtest.h>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "computer_control/controller.h"
#include "core/computer_control_environment.h"
#include "display/display_manager.h"
#include "fake_renderer.h"
#include "input/drag.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "scene/surface_node.h"
#include "shell/shell.h"
#include "shell/shell_drag.h"
#include "shell/shell_host.h"
#include "shell/shell_input.h"
#include "shell/shell_window.h"
#include "shell/window_renderer.h"
#include "shm_test_util.h"
#include "ui/embedded_window.h"
#include "ui/frame_client.h"
#include "ui/painter.h"
#include "ui/text_field.h"
#include "ui/view.h"
#include "wayland/protocols/text_input.h"
#include "wm/stacking.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"

using namespace Lingmo;

namespace {

input::Timestamp at(int ms)
{
    return input::Timestamp(std::chrono::milliseconds(ms));
}

const base::Rect kOutput(0, 0, 1000, 800);
const base::Rect kSecondary(1000, 0, 800, 600);

// A coloured view that records what reaches it.
class ProbeView : public ui::View
{
public:
    explicit ProbeView(ui::Color color)
        : m_color(color)
    {
        setTracksHover(true);
    }

    bool acceptsPointer() const override { return true; }
    void pointerEnter() override { events.push_back("enter"); }
    void pointerLeave() override { events.push_back("leave"); }
    void pointerPress(const ui::PointerEvent &event) override
    {
        events.push_back(event.button == ui::MouseButton::Right ? "right-press" : "press");
        lastPosition = event.position;
        if (onPress)
            onPress(event);
    }
    void pointerMove(const ui::PointerEvent &event) override
    {
        events.push_back("move");
        lastPosition = event.position;
    }
    void pointerRelease(const ui::PointerEvent &event) override
    {
        events.push_back("release");
        lastPosition = event.position;
    }
    void hoverMove(const ui::PointerEvent &event) override { hoverPosition = event.position; }
    bool scroll(const ui::ScrollEvent &event) override
    {
        scrolls.push_back(event);
        return true;
    }
    void paint(ui::Painter &painter) override { painter.fillRect(localRect(), m_color); }

    bool saw(const std::string &event) const
    {
        return std::find(events.begin(), events.end(), event) != events.end();
    }

    void setColor(ui::Color color)
    {
        m_color = color;
        requestPaint();
    }

    std::vector<std::string> events;
    std::vector<ui::ScrollEvent> scrolls;
    ui::PointF lastPosition;
    ui::PointF hoverPosition;
    std::function<void(const ui::PointerEvent &)> onPress;

private:
    ui::Color m_color;
};

// What the controller refused an agent.
class RejectionRecorder : public computer_control::ControlClient
{
public:
    void granted() override { ++granteds; }
    void revoked(computer_control::RevokeReason) override {}
    void rejected(computer_control::Rejection rejection) override { rejections.push_back(rejection); }
    void window(uint32_t handle, const computer_control::WindowInfo &window) override
    {
        windows[window.id] = handle;
    }
    void windowsDone() override {}
    void pointer(const base::PointF &) override {}

    int granteds = 0;
    std::vector<computer_control::Rejection> rejections;
    // Handles by window id.
    std::map<uint64_t, uint32_t> windows;
};

// The test component: a coloured panel along the top edge, reserving its
// height, with a text field on its right half and a menu on a right press.
class TestPanel : public shell::Component
{
public:
    static constexpr int kHeight = 28;

    explicit TestPanel(shell::Shell *shell)
    {
        window = std::make_unique<shell::ShellWindow>(shell, wm::WindowRole::Panel);
        auto content = std::make_unique<ProbeView>(ui::Color::rgb(0xff0000));
        probe = content.get();
        field = probe->addChild<ui::TextField>();
        field->setGeometry({500, 0, 500, double(kHeight)});
        probe->setCursorShape(ui::CursorShape::Pointer);
        probe->onPress = [this](const ui::PointerEvent &event) {
            if (event.button == ui::MouseButton::Right)
                openMenu(event.position);
        };
        window->setContentView(std::move(content));
        window->setFocusPolicy(wm::FocusPolicy::NoFocus);
        window->setGeometry(base::Rect(0, 0, kOutput.width(), kHeight));
        window->setExclusiveZone(wm::ScreenEdge::Top, kHeight);
        window->show();
    }

    void openMenu(ui::PointF at)
    {
        ui::PopupPlacement placement;
        placement.anchorRect = {at.x, 0, 1, double(kHeight)};
        placement.anchor = ui::PopupAnchor::BottomLeft;
        placement.gravity = ui::PopupAnchor::BottomRight;
        menu = ui::Window::createPopup(window->window(), placement, true);
        auto content = std::make_unique<ProbeView>(ui::Color::rgb(0x00ff00));
        menuProbe = content.get();
        menu->setContentView(std::move(content));
        menu->onCloseRequested = [this] { ++menuDismissals; };
        menu->resize({120, 80});
        menu->show();
    }

    // The shell window hosting the menu; null once the toolkit hid it.
    shell::ShellWindow *menuWindow(shell::Shell *shell) const
    {
        return menu ? shell->host()->windowFor(static_cast<ui::EmbeddedWindow *>(menu.get()))
                    : nullptr;
    }

    std::unique_ptr<shell::ShellWindow> window;
    ProbeView *probe = nullptr;
    ui::TextField *field = nullptr;
    std::unique_ptr<ui::Window> menu;
    ProbeView *menuProbe = nullptr;
    int menuDismissals = 0;
};

display::DisplayId displayId(const char *value)
{
    return display::DisplayId(value);
}

display::DisplaySnapshot physicalDisplay(const char *name, base::Size size)
{
    display::DisplaySnapshot result;
    result.sessionId = displayId(name);
    result.persistentKey = "edid-" + std::string(name);
    result.portKey = "port-" + std::string(name);
    result.connectorName = name;
    result.devicePath = "/dev/dri/card0";
    result.physicalSizeMm = {600, 340};
    result.modes = {{size, 60000, 148500, size.width() + 100, size.height() + 50, 0, "mode", true}};
    result.currentMode = result.modes.front();
    return result;
}

display::LogicalDisplay logicalDisplay(const char *name, base::Rect bounds, bool primary,
                                       double scale = 1)
{
    return {displayId(name),
            {displayId(name)},
            "Lingmo-" + std::string(name),
            bounds,
            bounds,
            *compositor::OutputScale::fromNumerator(uint32_t(scale * 120)),
            display::DisplayRotation::Normal,
            primary};
}

class ShellTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node *surfaceContainer =
            scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    compositor::Compositor compositor {surfaceContainer};
    testsupport::FakeRenderer renderer;
    input::Seat seat {&compositor};
    input::InputManager input {&seat};
    wm::WindowManager manager {&compositor, &seat};
    std::unique_ptr<shell::Shell> shell;
    std::vector<std::unique_ptr<compositor::Surface>> surfaces;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 22);
    int repaints = 0;
    base::Subscription repaintNeeded;

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
        input.setBounds(kOutput.size());
        input.setEventSink(&manager);
        repaintNeeded = compositor.repaintNeeded.connect([this](bool) { ++repaints; });
        shell = std::make_unique<shell::Shell>(
                shell::ShellEnvironment {&compositor, &manager, &seat, nullptr});
    }

    void TearDown() override
    {
        shell.reset();
        seat.shutdown();
    }

    // What an output frame does before it composes: the shell's pass, then
    // the surfaces' textures.
    void frame()
    {
        compositor.updateTextures(renderer);
    }

    // An application window, mapped and active.
    wm::Window *openApplication(const base::Rect &bounds)
    {
        surfaces.push_back(compositor.createSurface());
        compositor::Surface *surface = surfaces.back().get();
        surface->attach(pool->createBuffer(bounds.size(), 0, bounds.width() * 4,
                                           render::PixelFormat::Bgrx8888, nullptr),
                        {});
        surface->commit();
        frame();
        wm::Window *window = manager.createWindow(
                surface, {.requestedSize = bounds.size(), .applicationId = "app"});
        window->setBounds(bounds);
        window->activate();
        return window;
    }

    size_t stackIndex(const wm::Window *window) const
    {
        const auto &windows = manager.windows();
        return size_t(std::find(windows.begin(), windows.end(), window) - windows.begin());
    }

    static const testsupport::FakeTexture *textureOf(const shell::ShellWindow &window)
    {
        return dynamic_cast<const testsupport::FakeTexture *>(window.surface()->node()->texture());
    }

    // The texture's pixel at a device point, as 0xAARRGGBB.
    static uint32_t pixelAt(const testsupport::FakeTexture &texture, int x, int y)
    {
        const uint8_t *pixel =
                texture.pixels().data() + (size_t(y) * texture.size().width() + size_t(x)) * 4;
        return uint32_t(pixel[3]) << 24 | uint32_t(pixel[0]) << 16 | uint32_t(pixel[1]) << 8
                | pixel[2];
    }

    void move(int ms, double x, double y) { input.onPointerMotionAbsolute(at(ms), {x, y}); }

    void click(int ms, double x, double y, uint32_t button = BTN_LEFT)
    {
        move(ms, x, y);
        input.onPointerButton(at(ms), button, true);
        input.onPointerButton(at(ms + 1), button, false);
    }

    void key(int ms, uint32_t code)
    {
        input.onKey(at(ms), code, true);
        input.onKey(at(ms + 1), code, false);
    }
};

} // namespace

TEST_F(ShellTest, APanelIsAShellRoleWindowThatReservesItsEdge)
{
    auto *panel = shell->addComponent(std::make_unique<TestPanel>(shell.get()));
    wm::Window *window = panel->window->wmWindow();
    ASSERT_NE(nullptr, window);
    EXPECT_EQ(wm::WindowRole::Panel, window->role());
    EXPECT_EQ(wm::FocusPolicy::NoFocus, window->focusPolicy());
    EXPECT_EQ(base::Rect(0, 0, 1000, TestPanel::kHeight), window->bounds());
    EXPECT_FALSE(window->isDecorated());
    EXPECT_TRUE(window->skipTaskbar());
    EXPECT_EQ(window, manager.windowForSurface(panel->window->surface()));

    // The work area loses the panel's height, and the shell's outputs say so.
    EXPECT_EQ(base::Rect(0, TestPanel::kHeight, 1000, 800 - TestPanel::kHeight),
              manager.workAreaFor(nullptr));
    ASSERT_EQ(1u, shell->outputs().size());
    EXPECT_EQ(manager.workAreaFor(nullptr), shell->workArea(shell->outputs().front()));

    // A maximized application fills the rest, and stays under the panel.
    wm::Window *application = openApplication(base::Rect(100, 100, 400, 300));
    application->maximize();
    EXPECT_EQ(manager.workAreaFor(nullptr), application->bounds());
    manager.raise(application);
    EXPECT_GT(stackIndex(window), stackIndex(application));

    // The dock's layer is above the panel's, whatever order they came in.
    shell::ShellWindow dock(shell.get(), wm::WindowRole::Dock);
    dock.setGeometry(base::Rect(300, 740, 400, 56));
    dock.show();
    ASSERT_NE(nullptr, dock.wmWindow());
    manager.raise(window);
    EXPECT_GT(stackIndex(dock.wmWindow()), stackIndex(window));

    // A panel windows may cover drops into the applications' layer.
    panel->window->setPanelBehavior(wm::PanelBehavior::WindowsCanCover);
    manager.raise(application);
    EXPECT_GT(stackIndex(application), stackIndex(window));
    panel->window->setPanelBehavior(wm::PanelBehavior::StayAbove);
    EXPECT_GT(stackIndex(window), stackIndex(application));

    // Hiding unmaps the window and gives its edge back only when asked.
    panel->window->hide();
    EXPECT_EQ(nullptr, panel->window->wmWindow());
    EXPECT_FALSE(manager.hasWindow(window));
    panel->window->clearExclusiveZone();
    EXPECT_EQ(kOutput, manager.workAreaFor(nullptr));
    panel->window->show();
    EXPECT_NE(nullptr, panel->window->wmWindow());
}

TEST_F(ShellTest, TheFirstFrameIsRenderedBeforeTheWindowIsComposed)
{
    auto *panel = shell->addComponent(std::make_unique<TestPanel>(shell.get()));
    shell::WindowRenderer *windows = shell->renderer();
    // Mapped at once, drawn at the next frame, which showing asked for.
    EXPECT_GT(repaints, 0);
    EXPECT_TRUE(windows->isScheduled(panel->window.get()));
    EXPECT_TRUE(compositor.awaitsFrame(kOutput));
    EXPECT_FALSE(compositor.awaitsFrame(base::Rect(0, 400, 1000, 400)));
    EXPECT_EQ(nullptr, panel->window->surface()->node()->texture());

    frame();
    const testsupport::FakeTexture *texture = textureOf(*panel->window);
    ASSERT_NE(nullptr, texture);
    EXPECT_EQ(base::Size(1000, TestPanel::kHeight), texture->size());
    EXPECT_EQ(0xffff0000u, pixelAt(*texture, 10, 10));
    EXPECT_FALSE(windows->isScheduled(panel->window.get()));
    EXPECT_FALSE(compositor.awaitsFrame(kOutput));
    EXPECT_EQ(base::Rect(0, 0, 1000, TestPanel::kHeight),
              panel->window->surface()->node()->absoluteGeometry());
}

TEST_F(ShellTest, DirtyWindowsRenderTogetherInOnePassPerFrame)
{
    auto *panel = shell->addComponent(std::make_unique<TestPanel>(shell.get()));
    shell::ShellWindow osd(shell.get(), wm::WindowRole::Osd);
    auto content = std::make_unique<ProbeView>(ui::Color::rgb(0x0000ff));
    ProbeView *osdProbe = content.get();
    osd.setContentView(std::move(content));
    osd.setGeometry(base::Rect(400, 600, 200, 100));
    osd.show();
    EXPECT_EQ(wm::FocusPolicy::NoFocus, osd.wmWindow()->focusPolicy());

    shell::WindowRenderer *windows = shell->renderer();
    const uint64_t passes = windows->passCount();
    frame();
    EXPECT_EQ(passes + 1, windows->passCount());
    EXPECT_EQ(2u, windows->lastPass().size());
    ASSERT_NE(nullptr, textureOf(osd));
    EXPECT_EQ(0xff0000ffu, pixelAt(*textureOf(osd), 100, 50));

    // Nothing changed: the next frame renders nothing.
    frame();
    EXPECT_EQ(passes + 1, windows->passCount());

    // One window changed: it alone is rendered, and only its damage uploaded.
    osdProbe->setColor(ui::Color::rgb(0x00ffff));
    EXPECT_TRUE(compositor.awaitsFrame(osd.drawnBounds()));
    EXPECT_FALSE(compositor.awaitsFrame(base::Rect(0, 0, 100, 100)));
    const testsupport::FakeTexture *panelTexture = textureOf(*panel->window);
    const int panelUploads = panelTexture->uploadCount;
    frame();
    EXPECT_EQ(passes + 2, windows->passCount());
    ASSERT_EQ(1u, windows->lastPass().size());
    EXPECT_EQ(&osd, windows->lastPass().front());
    EXPECT_EQ(0xff00ffffu, pixelAt(*textureOf(osd), 100, 50));
    EXPECT_EQ(panelUploads, panelTexture->uploadCount);
    EXPECT_EQ(osd.surface()->node(), osd.wmWindow()->surfaceNode());

    // A hidden window is not rendered, and is again once shown.
    osd.hide();
    osdProbe->setColor(ui::Color::rgb(0xffffff));
    EXPECT_FALSE(windows->isScheduled(&osd));
    osd.show();
    frame();
    EXPECT_EQ(0xffffffffu, pixelAt(*textureOf(osd), 100, 50));
}

TEST_F(ShellTest, AWindowChangedByAnotherWindowsFrameIsDrawnInTheSameFrame)
{
    // Runs once on the next frame of its window.
    class Step : public ui::FrameClient
    {
    public:
        using ui::FrameClient::FrameClient;

        void start() { startTicking(); }

        std::function<void()> onTick;

    protected:
        void tick(uint64_t) override
        {
            stopTicking();
            onTick();
        }
    };

    auto *panel = shell->addComponent(std::make_unique<TestPanel>(shell.get()));
    shell::ShellWindow osd(shell.get(), wm::WindowRole::Osd);
    auto content = std::make_unique<ProbeView>(ui::Color::rgb(0x0000ff));
    ProbeView *osdProbe = content.get();
    osd.setContentView(std::move(content));
    osd.setGeometry(base::Rect(400, 600, 200, 100));
    osd.show();
    frame();

    // The panel's animation recolours the window and moves its glass, as the
    // dock does its tooltip: both reach the frame together.
    Step step(panel->window->window());
    step.onTick = [&] {
        osdProbe->setColor(ui::Color::rgb(0x00ffff));
        osd.setBackgroundBlur(base::Rect(0, 0, 200, 100), 8);
    };
    step.start();
    const uint64_t passes = shell->renderer()->passCount();
    frame();
    EXPECT_EQ(passes + 1, shell->renderer()->passCount());
    EXPECT_EQ(2u, shell->renderer()->lastPass().size());
    EXPECT_EQ(0xff00ffffu, pixelAt(*textureOf(osd), 100, 50));
    EXPECT_EQ(base::Rect(0, 0, 200, 100), osd.surface()->backgroundBlurRect());
    EXPECT_FALSE(shell->renderer()->isScheduled(&osd));
}

TEST_F(ShellTest, PointerInputReachesTheWindowThroughTheSeat)
{
    auto *panel = shell->addComponent(std::make_unique<TestPanel>(shell.get()));
    frame();
    ProbeView *probe = panel->probe;

    move(1, 50, 10);
    EXPECT_TRUE(probe->saw("enter"));
    EXPECT_EQ(ui::PointF(50, 10), probe->hoverPosition);
    EXPECT_EQ(panel->window.get(), shell->input()->pointerWindow());
    EXPECT_EQ(panel->window->surface(), seat.pointerFocus());
    // The view's cursor, as a client's set_cursor would give it.
    move(2, 60, 12);
    EXPECT_EQ(input::CursorShape::Pointer, seat.clientCursorShape());

    // An implicit grab: the press keeps the view until the release, wherever
    // the pointer goes.
    input.onPointerButton(at(3), BTN_LEFT, true);
    EXPECT_TRUE(probe->saw("press"));
    move(4, 60, 300);
    EXPECT_TRUE(probe->saw("move"));
    EXPECT_EQ(ui::PointF(60, 300), probe->lastPosition);
    EXPECT_EQ(panel->window->surface(), seat.pointerFocus());
    input.onPointerButton(at(5), BTN_LEFT, false);
    EXPECT_TRUE(probe->saw("release"));
    // The next motion finds what is under the pointer, as for a client.
    move(5, 60, 301);
    EXPECT_EQ(nullptr, shell->input()->pointerWindow());
    EXPECT_TRUE(probe->saw("leave"));

    // A wheel click is ten axis units and 120 value120s.
    move(6, 50, 10);
    input.onPointerAxis(at(7), {0, 10}, true);
    ASSERT_EQ(1u, probe->scrolls.size());
    EXPECT_EQ(10, probe->scrolls.front().deltaY);
    EXPECT_EQ(120, probe->scrolls.front().value120Y);
    EXPECT_FALSE(probe->scrolls.front().precise);

    // The panel never takes the keyboard from the application.
    wm::Window *application = openApplication(base::Rect(100, 100, 400, 300));
    click(8, 50, 10);
    EXPECT_TRUE(application->isActive());
    EXPECT_EQ(application->rootSurface(), seat.focusedSurface());
}

TEST_F(ShellTest, AnActivatedWindowTakesTheKeyboardAndGivesItBack)
{
    wm::Window *application = openApplication(base::Rect(100, 100, 400, 300));
    shell::ShellWindow launcher(shell.get(), wm::WindowRole::Launcher);
    auto field = std::make_unique<ui::TextField>();
    ui::TextField *text = field.get();
    launcher.setContentView(std::move(field));
    launcher.setGeometry(kOutput);
    launcher.show();
    frame();
    text->setFocus();
    // Mapped, the launcher is activated.
    EXPECT_TRUE(launcher.wmWindow()->isActive());
    EXPECT_FALSE(application->isActive());

    application->activate();
    EXPECT_FALSE(launcher.window()->isActive());
    EXPECT_EQ(nullptr, shell->input()->keyboardWindow());
    launcher.activate();
    EXPECT_TRUE(launcher.wmWindow()->isActive());
    EXPECT_EQ(launcher.surface(), seat.focusedSurface());
    EXPECT_EQ(&launcher, shell->input()->keyboardWindow());
    EXPECT_TRUE(launcher.window()->isActive());
    key(10, KEY_C);
    key(12, KEY_F);
    EXPECT_EQ("cf", text->text());

    // Unmapped, the keyboard returns to the application.
    launcher.hide();
    EXPECT_TRUE(application->isActive());
    EXPECT_EQ(application->rootSurface(), seat.focusedSurface());
    EXPECT_EQ(nullptr, shell->input()->keyboardWindow());
    key(14, KEY_X);
    EXPECT_EQ("cf", text->text());
}

TEST_F(ShellTest, APopupIsPlacedByThePositionerAndDismissedByAPressOutside)
{
    auto *panel = shell->addComponent(std::make_unique<TestPanel>(shell.get()));
    frame();
    // A right press on the panel opens its menu under the pointer.
    click(1, 40, 20, BTN_RIGHT);
    shell::ShellWindow *menu = panel->menuWindow(shell.get());
    ASSERT_NE(nullptr, menu);
    ASSERT_NE(nullptr, menu->wmWindow());
    EXPECT_TRUE(menu->isPopup());
    EXPECT_TRUE(menu->wmWindow()->isPopup());
    EXPECT_EQ(panel->window->wmWindow(), menu->wmWindow()->transientParent());
    EXPECT_EQ(base::Rect(40, TestPanel::kHeight, 120, 80), menu->wmWindow()->bounds());
    EXPECT_EQ(base::Rect(40, TestPanel::kHeight, 120, 80), menu->popupBounds());
    auto *embedded = static_cast<ui::EmbeddedWindow *>(panel->menu.get());
    EXPECT_EQ(40, embedded->popupGeometry().x);
    EXPECT_EQ(TestPanel::kHeight, embedded->popupGeometry().y);
    EXPECT_GT(stackIndex(menu->wmWindow()), stackIndex(panel->window->wmWindow()));
    EXPECT_EQ(1u, manager.popupGrabs().size());
    frame();
    EXPECT_EQ(0xff00ff00u, pixelAt(*textureOf(*menu), 5, 5));

    // Near the right edge it flips to the left of its anchor.
    panel->openMenu({990, 20});
    menu = panel->menuWindow(shell.get());
    ASSERT_NE(nullptr, menu);
    EXPECT_EQ(base::Rect(991 - 120, TestPanel::kHeight, 120, 80), menu->wmWindow()->bounds());
    test::processPendingEvents();
    EXPECT_EQ(1u, manager.popupGrabs().size());
    frame();

    // A press inside goes to the menu.
    click(3, 900, 50);
    EXPECT_TRUE(panel->menuProbe->saw("press"));
    EXPECT_EQ(0, panel->menuDismissals);

    // A press outside the chain ends the grab, reaches nothing else, and the
    // toolkit is told: its popup closes and the shell lets go of it.
    wm::Window *application = openApplication(base::Rect(100, 300, 400, 300));
    const size_t probeEvents = panel->probe->events.size();
    wm::Window *menuWindow = menu->wmWindow();
    click(5, 300, 400);
    EXPECT_EQ(1, panel->menuDismissals);
    EXPECT_TRUE(panel->menu->isHidden());
    EXPECT_EQ(nullptr, panel->menuWindow(shell.get()));
    EXPECT_FALSE(manager.hasWindow(menuWindow));
    EXPECT_TRUE(manager.popupGrabs().empty());
    EXPECT_EQ(probeEvents, panel->probe->events.size());
    EXPECT_TRUE(application->isActive());
    test::processPendingEvents();
}

TEST_F(ShellTest, APopupFollowsItsSizeAndGoesWithItsParent)
{
    auto *panel = shell->addComponent(std::make_unique<TestPanel>(shell.get()));
    panel->openMenu({100, 10});
    shell::ShellWindow *menu = panel->menuWindow(shell.get());
    ASSERT_NE(nullptr, menu);
    ASSERT_NE(nullptr, menu->wmWindow());

    // Bigger: placed again, kept under its anchor.
    panel->menu->resize({200, 120});
    EXPECT_EQ(base::Rect(100, TestPanel::kHeight, 200, 120), menu->wmWindow()->bounds());
    frame();
    EXPECT_EQ(base::Size(200, 120), textureOf(*menu)->size());

    // Destroyed by the toolkit: the shell lets go of it at once.
    wm::Window *menuWindow = menu->wmWindow();
    panel->menu.reset();
    EXPECT_FALSE(manager.hasWindow(menuWindow));
    test::processPendingEvents();

    // The parent hidden, its popups close.
    panel->openMenu({100, 10});
    ASSERT_NE(nullptr, panel->menuWindow(shell.get()));
    panel->window->hide();
    EXPECT_TRUE(panel->menu->isHidden());
    EXPECT_EQ(nullptr, panel->menuWindow(shell.get()));
    test::processPendingEvents();
}

TEST_F(ShellTest, TouchDrivesTheWindowItLandsOn)
{
    auto *panel = shell->addComponent(std::make_unique<TestPanel>(shell.get()));
    frame();
    // Touch goes through the seat's touch targeting to the toolkit, which
    // turns its first point into a pointer.
    input.onTouchDown(at(1), 0, {50.0 / 1000, 10.0 / 800});
    input.onTouchFrame(at(1));
    EXPECT_TRUE(panel->probe->saw("press"));
    input.onTouchMotion(at(2), 0, {80.0 / 1000, 12.0 / 800});
    input.onTouchFrame(at(2));
    EXPECT_EQ(ui::PointF(80, 12), panel->probe->lastPosition);
    input.onTouchUp(at(3), 0);
    input.onTouchFrame(at(3));
    EXPECT_TRUE(panel->probe->saw("release"));
}

class MultiOutputShellTest : public ShellTest
{
protected:
    display::DisplayManager displays;
    uint64_t revision = 0;

    void SetUp() override
    {
        ShellTest::SetUp();
        ASSERT_TRUE(displays.installHardware(
                {"instance",
                 "seat0",
                 1,
                 {physicalDisplay("a", kOutput.size()), physicalDisplay("b", {1600, 1200})}}));
        apply(true, 1);
        manager.setDisplayManager(&displays);
    }

    void TearDown() override
    {
        shell.reset();
        manager.setDisplayManager(nullptr);
        ShellTest::TearDown();
    }

    void apply(bool withSecondary, double secondaryScale)
    {
        std::vector<display::LogicalDisplay> logical {logicalDisplay("a", kOutput, true)};
        std::vector<display::DisplayPlacement> placements;
        if (withSecondary) {
            logical.push_back(logicalDisplay("b", kSecondary, false, secondaryScale));
            placements.push_back({displayId("b"), displayId("a"), display::LayoutEdge::Right, 0});
        }
        ASSERT_TRUE(displays.installApplied({"instance", 1, ++revision, {}, logical, placements}));
    }
};

TEST_F(MultiOutputShellTest, AWindowTakesItsOutputsScaleAndOutlivesTheOutput)
{
    ASSERT_EQ(2u, shell->outputs().size());
    EXPECT_EQ("Lingmo-a", shell->outputs().front().name);
    EXPECT_TRUE(shell->outputs().front().primary);
    int outputChanges = 0;
    base::Subscription changes = shell->outputsChanged.connect([&] { ++outputChanges; });

    shell::ShellWindow panel(shell.get(), wm::WindowRole::Panel);
    panel.setContentView(std::make_unique<ProbeView>(ui::Color::rgb(0xff0000)));
    panel.setOutput("Lingmo-b");
    panel.setGeometry(base::Rect(1000, 0, 800, 30));
    panel.setExclusiveZone(wm::ScreenEdge::Top, 30);
    panel.show();
    frame();
    EXPECT_EQ(1.0, panel.scale());
    EXPECT_EQ(base::Size(800, 30), textureOf(panel)->size());
    // The reservation is on its own output only.
    EXPECT_EQ(kOutput, manager.workAreaForOutput(kOutput));
    EXPECT_EQ(base::Rect(1000, 30, 800, 570), manager.workAreaForOutput(kSecondary));

    // Its output's scale changes: the window renders at the new one, at the
    // same logical size.
    apply(true, 2);
    EXPECT_GT(outputChanges, 0);
    EXPECT_EQ(2.0, panel.scale());
    EXPECT_TRUE(shell->renderer()->isScheduled(&panel));
    frame();
    EXPECT_EQ(base::Size(1600, 60), textureOf(panel)->size());
    EXPECT_EQ(base::Rect(1000, 0, 800, 30), panel.wmWindow()->bounds());
    EXPECT_EQ(base::Rect(1000, 0, 800, 30), panel.surface()->node()->absoluteGeometry());

    // The output goes: the shell's list follows, and the window takes the
    // primary output's scale until its component moves or hides it.
    outputChanges = 0;
    apply(false, 1);
    EXPECT_GT(outputChanges, 0);
    ASSERT_EQ(1u, shell->outputs().size());
    EXPECT_EQ(1.0, panel.scale());
    frame();
    EXPECT_EQ(base::Size(800, 30), textureOf(panel)->size());

    // It comes back.
    apply(true, 2);
    EXPECT_EQ(2u, shell->outputs().size());
    EXPECT_EQ(2.0, panel.scale());
}

namespace {

// The input method's side, recording what the shell's text input asks.
class FakeInputMethod final : public wayland::InputMethodRelay
{
public:
    explicit FakeInputMethod(input::Seat *seat)
        : m_seat(seat)
    {
    }

    input::Seat *seat() const override { return m_seat; }
    wayland::WaylandSeat *waylandSeat() const override { return nullptr; }
    compositor::Surface *focusedSurface() const override { return m_seat->focusedSurface(); }
    bool focusedClient(const wl_client *) const override { return false; }
    wayland::TextInput *active() const override { return activeInput; }
    wayland::TextInputEdit currentPreedit(const wayland::TextInput *) const override { return {}; }
    void activate(wayland::TextInput *textInput, bool) override
    {
        activeInput = textInput;
        ++activations;
    }
    void deactivate(wayland::TextInput *textInput) override
    {
        if (activeInput == textInput)
            activeInput = nullptr;
    }
    void stateChanged(wayland::TextInput *, bool) override { ++stateChanges; }
    void add(wayland::TextInput *textInput) override { inputs.push_back(textInput); }
    void remove(wayland::TextInput *textInput) override
    {
        std::erase(inputs, textInput);
        deactivate(textInput);
    }
    wl_resource *createResource(wl_client *, const wl_interface *, uint32_t, const void *, void *,
                                wl_resource_destroy_func_t) override
    {
        return nullptr;
    }
    void forgetResource(wl_resource *) override {}

    std::vector<wayland::TextInput *> inputs;
    wayland::TextInput *activeInput = nullptr;
    int activations = 0;
    int stateChanges = 0;

private:
    input::Seat *m_seat;
};

class TextInputShellTest : public ShellTest
{
protected:
    std::unique_ptr<FakeInputMethod> inputMethod;

    void SetUp() override
    {
        ShellTest::SetUp();
        shell.reset();
        inputMethod = std::make_unique<FakeInputMethod>(&seat);
        shell = std::make_unique<shell::Shell>(
                shell::ShellEnvironment {&compositor, &manager, &seat, nullptr, inputMethod.get()});
    }

    void TearDown() override
    {
        ShellTest::TearDown();
        inputMethod.reset();
    }
};

// Starts a drag of `text` when pressed.
class DragView : public ProbeView
{
public:
    DragView()
        : ProbeView(ui::Color::rgb(0x808080))
    {
        onPress = [this](const ui::PointerEvent &) {
            ui::DragSource source;
            source.mimeTypes = {"text/plain"};
            source.data = [](const std::string &) { return std::string("dragged text"); };
            source.actions = ui::CopyAction | ui::MoveAction;
            source.icon = ui::Bitmap(base::Size(8, 8), ui::Bitmap::Format::Rgba8888Premultiplied);
            source.hotSpot = {4, 4};
            source.onFinished = [this](ui::DropAction action) { finished.push_back(action); };
            started = startDrag(std::move(source));
        };
    }

    bool started = false;
    std::vector<ui::DropAction> finished;
};

// Takes plain text.
class DropView : public ProbeView
{
public:
    DropView()
        : ProbeView(ui::Color::rgb(0x404040))
    {
    }

    bool canDrop(const ui::DropOffer &offer) const override
    {
        return offer.hasMimeType("text/plain");
    }
    void dragEnter(const ui::DragEvent &) override { events.push_back("drag-enter"); }
    uint32_t dragUpdate(const ui::DragEvent &event) override
    {
        dragPosition = event.position;
        return ui::CopyAction;
    }
    void dragLeave() override { events.push_back("drag-leave"); }
    uint32_t drop(const ui::DragEvent &event) override
    {
        events.push_back("drop");
        event.offer->read("text/plain", [this](std::string data) { received = std::move(data); });
        return ui::CopyAction;
    }

    ui::PointF dragPosition;
    std::optional<std::string> received;
};

} // namespace

TEST_F(TextInputShellTest, AFocusedTextFieldIsOnTheInputMethodRelay)
{
    ASSERT_EQ(1u, inputMethod->inputs.size());
    wayland::TextInput *textInput = inputMethod->inputs.front();
    wm::Window *application = openApplication(base::Rect(100, 100, 400, 300));
    shell::ShellWindow launcher(shell.get(), wm::WindowRole::Launcher);
    auto field = std::make_unique<ui::TextField>();
    ui::TextField *text = field.get();
    launcher.setContentView(std::move(field));
    launcher.setGeometry(base::Rect(0, 0, 400, 40));
    launcher.show();
    frame();
    text->setFocus();
    frame();
    ASSERT_EQ(textInput, inputMethod->activeInput);
    EXPECT_TRUE(textInput->attached());
    EXPECT_TRUE(textInput->ownsSurface(launcher.surface()));
    EXPECT_EQ(std::string(), textInput->state().surrounding);
    ASSERT_TRUE(textInput->state().rectangle);

    // Typing changes the surrounding text the input method sees.
    key(1, KEY_A);
    frame();
    EXPECT_EQ(std::string("a"), textInput->state().surrounding);
    EXPECT_EQ(1, textInput->state().cursor);
    EXPECT_GT(inputMethod->stateChanges, 0);

    // What the input method commits goes into the field.
    wayland::TextInputEdit edit;
    edit.commit = "\xe4\xbd\xa0\xe5\xa5\xbd";
    textInput->sendEdit(edit);
    EXPECT_EQ("a\xe4\xbd\xa0\xe5\xa5\xbd", text->text());
    edit = {};
    edit.deletion = std::pair<uint32_t, uint32_t>(3, 0);
    textInput->sendEdit(edit);
    EXPECT_EQ("a\xe4\xbd\xa0", text->text());

    // The keyboard gone, so is the text input.
    application->activate();
    frame();
    EXPECT_EQ(nullptr, inputMethod->activeInput);
    EXPECT_FALSE(textInput->ownsSurface(launcher.surface()));
}

TEST_F(ShellTest, ADragFromOneShellWindowDropsOnAnother)
{
    shell::ShellWindow source(shell.get(), wm::WindowRole::Panel);
    auto dragView = std::make_unique<DragView>();
    DragView *dragger = dragView.get();
    source.setContentView(std::move(dragView));
    source.setGeometry(base::Rect(0, 0, 400, 40));
    source.show();
    shell::ShellWindow target(shell.get(), wm::WindowRole::Dock);
    auto dropView = std::make_unique<DropView>();
    DropView *dropper = dropView.get();
    target.setContentView(std::move(dropView));
    target.setGeometry(base::Rect(300, 700, 400, 60));
    target.show();
    frame();

    move(1, 50, 20);
    input.onPointerButton(at(2), BTN_LEFT, true);
    ASSERT_TRUE(dragger->started);
    test::processPendingEvents();
    input::DragController *drag = seat.drag();
    ASSERT_TRUE(drag->active());
    EXPECT_EQ(source.surface(), drag->origin());

    move(3, 400, 720);
    EXPECT_EQ(target.surface(), drag->target());
    EXPECT_EQ(&target, shell->host()->drag()->target());
    EXPECT_TRUE(dropper->saw("drag-enter"));
    move(4, 420, 730);
    EXPECT_EQ(ui::PointF(120, 30), dropper->dragPosition);
    EXPECT_TRUE(drag->targetAcceptsDrop());

    input.onPointerButton(at(5), BTN_LEFT, false);
    EXPECT_TRUE(dropper->saw("drop"));
    ASSERT_TRUE(test::runUntil([&] { return dropper->received.has_value(); }));
    EXPECT_EQ("dragged text", *dropper->received);
    ASSERT_TRUE(test::runUntil([&] { return !dragger->finished.empty(); }));
    EXPECT_EQ(ui::CopyAction, dragger->finished.front());
    EXPECT_FALSE(drag->busy());

    // Cancelled with Escape: nothing taken.
    move(6, 50, 20);
    dragger->started = false;
    input.onPointerButton(at(7), BTN_LEFT, true);
    test::processPendingEvents();
    ASSERT_TRUE(drag->active());
    key(8, KEY_ESC);
    input.onPointerButton(at(9), BTN_LEFT, false);
    ASSERT_TRUE(test::runUntil([&] { return dragger->finished.size() == 2; }));
    EXPECT_EQ(ui::NoAction, dragger->finished.back());
}

TEST_F(ShellTest, AGestureGoesToTheWindowUnderThePointer)
{
    auto *panel = shell->addComponent(std::make_unique<TestPanel>(shell.get()));
    frame();
    struct GestureProbe : ProbeView {
        GestureProbe()
            : ProbeView(ui::Color::rgb(0))
        {
        }
        bool gesture(const ui::GestureEvent &event) override
        {
            types.push_back(event.type);
            if (event.type == ui::GestureEvent::Type::PinchUpdate)
                scale = event.scale;
            return true;
        }
        std::vector<ui::GestureEvent::Type> types;
        double scale = 1;
    };
    auto probe = std::make_unique<GestureProbe>();
    GestureProbe *gestures = probe.get();
    panel->probe->addChild(std::move(probe));
    gestures->setGeometry({0, 0, 100, 28});
    frame();
    move(1, 50, 10);
    input.onPointerGestureBegin(at(2), input::GestureType::Pinch, 2);
    input.onPointerGestureUpdate(at(3), input::GestureType::Pinch, {0, 0}, {0, 0}, 1.5, 0);
    input.onPointerGestureEnd(at(4), input::GestureType::Pinch, false);
    ASSERT_EQ(3u, gestures->types.size());
    EXPECT_EQ(ui::GestureEvent::Type::PinchBegin, gestures->types[0]);
    EXPECT_EQ(ui::GestureEvent::Type::PinchEnd, gestures->types[2]);
    EXPECT_EQ(1.5, gestures->scale);
}

TEST_F(ShellTest, AMovedAndResizedWindowDrawsWhereItIs)
{
    // As the dock does when its output's scale changes: a new place and size.
    shell::ShellWindow dock(shell.get(), wm::WindowRole::Dock);
    dock.setContentView(std::make_unique<ProbeView>(ui::Color::rgb(0x00ff00)));
    dock.setGeometry(base::Rect(5, 706, 990, 89));
    dock.show();
    frame();
    dock.setGeometry(base::Rect(5, 500, 600, 89));
    frame();
    EXPECT_EQ(base::Rect(5, 500, 600, 89), dock.wmWindow()->bounds());
    EXPECT_EQ(base::Rect(5, 500, 600, 89), dock.surface()->node()->absoluteGeometry());
    dock.setSize(base::Size(700, 100));
    frame();
    EXPECT_EQ(base::Rect(5, 500, 700, 100), dock.surface()->node()->absoluteGeometry());
}

TEST_F(ShellTest, AWindowDrawsWhereItIsAfterItsOutputShrinks)
{
    // A scale change shrinks the output under the dock before the dock
    // moves; the window manager fits it in first.
    manager.setOutputBounds(base::Rect(0, 0, 2160, 1440));
    shell::ShellWindow dock(shell.get(), wm::WindowRole::Dock);
    dock.setContentView(std::make_unique<ProbeView>(ui::Color::rgb(0x00ff00)));
    dock.setGeometry(base::Rect(5, 1346, 2150, 89));
    dock.show();
    frame();
    manager.setOutputBounds(kOutput);
    frame();
    dock.setGeometry(base::Rect(5, 706, 990, 89));
    frame();
    EXPECT_EQ(base::Rect(5, 706, 990, 89), dock.wmWindow()->bounds());
    EXPECT_EQ(base::Rect(5, 706, 990, 89), dock.surface()->node()->absoluteGeometry());
}

TEST_F(ShellTest, ComputerControlLeavesTheShellsSecretPromptsAlone)
{
    using computer_control::Rejection;

    auto *panel = shell->addComponent(std::make_unique<TestPanel>(shell.get()));
    // A prompt as the secret dialogs make it, with a popup of its own.
    shell::ShellWindow prompt(shell.get(), wm::WindowRole::Normal);
    auto content = std::make_unique<ProbeView>(ui::Color::rgb(0x0000ff));
    ProbeView *probe = content.get();
    ui::TextField *secret = probe->addChild<ui::TextField>();
    secret->setGeometry({0, 150, 300, 30});
    prompt.setContentView(std::move(content));
    prompt.setProtectedFromControl(true);
    prompt.setGeometry(base::Rect(300, 300, 300, 200));
    prompt.show();
    ui::PopupPlacement placement;
    placement.anchorRect = {10, 200, 1, 1};
    placement.anchor = ui::PopupAnchor::BottomLeft;
    placement.gravity = ui::PopupAnchor::BottomRight;
    std::unique_ptr<ui::Window> popup = ui::Window::createPopup(prompt.window(), placement, false);
    auto popupContent = std::make_unique<ProbeView>(ui::Color::rgb(0x00ff00));
    ProbeView *popupProbe = popupContent.get();
    popup->setContentView(std::move(popupContent));
    popup->resize({100, 50});
    popup->show();
    frame();
    ASSERT_TRUE(prompt.wmWindow()->isProtectedFromControl());
    shell::ShellWindow *popupWindow =
            shell->host()->windowFor(static_cast<ui::EmbeddedWindow *>(popup.get()));
    ASSERT_NE(nullptr, popupWindow);
    ASSERT_NE(nullptr, popupWindow->wmWindow());
    EXPECT_EQ(prompt.wmWindow(), popupWindow->wmWindow()->transientParent());

    core::ComputerControlHosts hosts;
    hosts.windowManager = &manager;
    hosts.seat = &seat;
    hosts.inputMethod = []() -> wayland::InputMethodManager * {
        return nullptr;
    };
    hosts.showsSystemDialogs = [](int64_t) {
        return false;
    };
    computer_control::Environment environment = core::computerControlEnvironment(std::move(hosts));
    std::function<void(computer_control::ConsentResult)> answer;
    environment.askConsent = [&answer](const std::string &, int64_t,
                                       std::function<void(computer_control::ConsentResult)> reply) {
        answer = std::move(reply);
        return std::function<void()>([] {});
    };
    environment.sessionBlocked = [] {
        return false;
    };
    environment.now = [] {
        return at(100000);
    };
    computer_control::Controller controller(&input, std::move(environment));
    RejectionRecorder agent;
    std::unique_ptr<computer_control::Control> control = controller.request(&agent, &agent, "Agent");
    ASSERT_TRUE(answer);
    answer(computer_control::ConsentResult::Allowed);
    ASSERT_EQ(1, agent.granteds);

    // No press on the prompt or its popup, no key or text into its field.
    control->motion(base::PointF(400, 320));
    control->button(BTN_LEFT, true);
    control->motion(base::PointF(330, 520));
    EXPECT_EQ(popupWindow->surface(), seat.pointerFocus());
    control->button(BTN_LEFT, true);
    prompt.activate();
    secret->setFocus();
    ASSERT_EQ(prompt.surface(), seat.focusedSurface());
    control->key(KEY_A, true);
    control->typeText("hunter2");
    // Nor is it closed or moved.
    control->listWindows();
    ASSERT_TRUE(agent.windows.contains(prompt.wmWindow()->id()));
    const uint32_t handle = agent.windows[prompt.wmWindow()->id()];
    control->closeWindow(handle);
    control->setWindowFrame(handle, base::Rect(0, 0, 300, 200));
    EXPECT_EQ(std::vector<Rejection>(6, Rejection::Protected), agent.rejections);
    EXPECT_EQ(base::Rect(300, 300, 300, 200), prompt.wmWindow()->bounds());
    EXPECT_FALSE(probe->saw("press"));
    EXPECT_FALSE(popupProbe->saw("press"));
    EXPECT_EQ("", secret->text());
    agent.rejections.clear();

    // Other shell windows take the agent's input as the user's.
    shell::ShellWindow launcher(shell.get(), wm::WindowRole::Launcher);
    auto field = std::make_unique<ui::TextField>();
    ui::TextField *text = field.get();
    launcher.setContentView(std::move(field));
    launcher.setGeometry(base::Rect(700, 600, 200, 40));
    launcher.show();
    frame();
    launcher.activate();
    text->setFocus();
    ASSERT_EQ(launcher.surface(), seat.focusedSurface());
    control->typeText("cf");
    EXPECT_EQ("cf", text->text());

    panel->openMenu({40, 20});
    frame();
    control->motion(base::PointF(60, 50));
    control->button(BTN_LEFT, true);
    control->button(BTN_LEFT, false);
    EXPECT_TRUE(panel->menuProbe->saw("press"));
    EXPECT_TRUE(agent.rejections.empty());
}
