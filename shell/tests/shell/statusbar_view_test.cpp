// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The status bar as the compositor draws it: its window, the layout of its
// items, panels, the tray, the clock and the colours it takes from what is
// behind it, headless on a fake renderer and a private bus.

#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <thread>

#include <linux/input-event-codes.h>

#include <gtest/gtest.h>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "dbus_test_client.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shell/shell.h"
#include "shell/dock/dock.h"
#include "shell/dock/dock_window.h"
#include "shell/shell_data.h"
#include "shell/shell_window.h"
#include "shell/statusbar/bar_item.h"
#include "shell/statusbar/battery.h"
#include "shell/statusbar/dbus_menu_popup.h"
#include "shell/statusbar/device_usage.h"
#include "shell/statusbar/status_bar.h"
#include "shell/statusbar/status_bar_backdrop.h"
#include "shell/wallpaper/wallpaper.h"
#include "shell/statusbar/status_bar_window.h"
#include "shell/statusbar/status_icon.h"
#include "shell/statusbar/status_items_view.h"
#include "shell/statusbar/system_tray_view.h"
#include "shell_test_util.h"
#include "shm_test_util.h"
#include "statusbar_fakes.h"
#include "temporary_directory.h"
#include "ui/bitmap.h"
#include "ui/embedded_window.h"
#include "ui/menu.h"
#include "wm/status_items.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_visual_controller.h"

using namespace Lingmo;
namespace dbus = base::dbus;
using Panel = shell::StatusBarPanels::Panel;

namespace {

input::Timestamp at(int ms)
{
    return input::Timestamp(std::chrono::milliseconds(ms));
}

const base::Rect kOutput(0, 0, 1440, 960);

dbus::Connection *sessionBus()
{
    return dbus::Connection::session(core::mainLoop());
}

class FakeIndicators final : public shell::StatusBarIndicators
{
public:
    Audio audio() const override { return m_audio; }
    Network network() const override { return m_network; }

    Audio m_audio;
    Network m_network;
};

class FakePanels final : public shell::StatusBarPanels
{
public:
    bool notifications = false;
    bool hasPanel(Panel panel) const override { return notifications || panel != Panel::NotificationCenter; }

    void openPanel(Panel panel, shell::ShellWindow *bar, const base::Rect &anchor) override
    {
        opened.push_back(panel);
        m_open = panel;
        m_isOpen = true;
        lastBar = bar;
        lastAnchor = anchor;
        panelsChanged();
    }

    void closePanels() override
    {
        ++closes;
        if (!m_isOpen)
            return;
        m_isOpen = false;
        panelsChanged();
    }

    bool isOpen(Panel panel) const override { return m_isOpen && m_open == panel; }

    std::vector<Panel> opened;
    int closes = 0;
    shell::ShellWindow *lastBar = nullptr;
    base::Rect lastAnchor;

private:
    Panel m_open = Panel::ControlCenter;
    bool m_isOpen = false;
};

class StatusBarViewTest : public testing::Test
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
    shell::StatusBarWindow *bar = nullptr;
    int time = 1000;

    void SetUp() override
    {
        shell::setShellDataDirectoryForTesting(LINGMO_SOURCE_DIR "/data/shell");
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
        input.setBounds(kOutput.size());
        input.setEventSink(&manager);
        shell::ShellEnvironment environment {&compositor, &manager, &seat};
        environment.servicesSettings = directory.filePath("services.conf");
        shell = std::make_unique<shell::Shell>(environment);
        auto apps = std::make_unique<test::FakeApplications>();
        apps->scan();
        shell->setApplications(std::move(apps));
        shell->setTaskWindows(std::make_unique<test::FakeTaskWindows>());
        directory.write("shell.conf", "");
        auto backend = std::make_unique<shell::StatusBar>(sessionBus(), dbus::Connection::system(core::mainLoop()),
                                                          &shell->tasks(), directory.filePath("shell.conf"));
        bar = shell->addComponent(std::make_unique<shell::StatusBarWindow>(shell.get(), std::move(backend)));
        frame();
    }

    void TearDown() override
    {
        shell.reset();
        seat.shutdown();
    }

    void frame() { compositor.updateTextures(renderer); }

    void move(double x, double y)
    {
        input.onPointerMotionAbsolute(at(time += 20), {x, y});
        frame();
    }

    void click(double x, double y, uint32_t button = BTN_LEFT)
    {
        move(x, y);
        input.onPointerButton(at(time += 20), button, true);
        input.onPointerButton(at(time += 20), button, false);
        frame();
    }

    // An item's rect on the screen.
    static ui::RectF onScreen(const ui::View *view)
    {
        const ui::PointF origin = view->mapToWindow({0, 0});
        return {origin.x, origin.y, view->geometry().width, view->geometry().height};
    }
};

} // namespace

TEST_F(StatusBarViewTest, TheBarIsAPanelAlongThePrimaryOutputsTop)
{
    wm::Window *window = bar->window()->wmWindow();
    ASSERT_NE(nullptr, window);
    EXPECT_EQ(wm::WindowRole::Panel, window->role());
    EXPECT_EQ(wm::FocusPolicy::NoFocus, window->focusPolicy());
    EXPECT_EQ(base::Rect(0, 0, 1440, 32), window->bounds());
    EXPECT_EQ(base::Rect(0, 32, 1440, 928), shell->workArea(*shell->output({})));

    // The output's bounds change: the bar follows.
    manager.setOutputBounds({0, 0, 1280, 800});
    test::runFor(std::chrono::milliseconds(20));
    frame();
    EXPECT_EQ(base::Rect(0, 0, 1280, 32), bar->window()->wmWindow()->bounds());
}

TEST_F(StatusBarViewTest, TheBarBlursWhatIsBehindIt)
{
    ASSERT_TRUE(test::runUntil([&] { return bar->statusBar().statusSettings().isReady(); }));
    compositor::Surface *surface = bar->window()->surface();
    ASSERT_TRUE(test::runUntil([&] {
        frame();
        return !surface->backgroundBlurRect().isEmpty();
    }));
    // The whole bar, in window coordinates: a plain Gaussian blur, no glass
    // lens, at the file's defaults.
    EXPECT_EQ(base::Rect(0, 0, 1440, 32), surface->backgroundBlurRect());
    EXPECT_FALSE(surface->glass().enabled);
    EXPECT_NEAR(shell::StatusBarSettings::kDefaultBlurSigma, surface->glass().plainSigma, 1e-3);
    EXPECT_NEAR(shell::StatusBarSettings::kDefaultOpacity, surface->glass().opacity, 1e-3);

    // The settings service changes it at once: zero opacity is a clear bar.
    ASSERT_TRUE(bar->statusBar().registerServices());
    test::DBusTestClient settings;
    settings.call("org.lingmo.Statusbar", "/Statusbar", "org.lingmo.Statusbar", "setOpacity", 0.0);
    ASSERT_TRUE(test::runUntil([&] {
        frame();
        return surface->backgroundBlurRect().isEmpty();
    }));

    // And a zero blur is as clear, whatever the opacity.
    settings.call("org.lingmo.Statusbar", "/Statusbar", "org.lingmo.Statusbar", "setOpacity", 0.5);
    settings.call("org.lingmo.Statusbar", "/Statusbar", "org.lingmo.Statusbar", "setBlurSigma", 0.0);
    ASSERT_TRUE(test::runUntil([&] {
        frame();
        return surface->backgroundBlurRect().isEmpty();
    }));

    settings.call("org.lingmo.Statusbar", "/Statusbar", "org.lingmo.Statusbar", "setBlurSigma", 40.0);
    ASSERT_TRUE(test::runUntil([&] {
        frame();
        return !surface->backgroundBlurRect().isEmpty();
    }));
    EXPECT_NEAR(40.0f, surface->glass().plainSigma, 1e-3);
    EXPECT_NEAR(0.5f, surface->glass().opacity, 1e-3);
}

TEST_F(StatusBarViewTest, ItemsAbutTheEdgeAndEachOther)
{
    // The clock's slot reaches the screen's right edge; its plate stops 9
    // short of it.
    const ui::RectF clock = onScreen(bar->clock());
    EXPECT_NEAR(1440, clock.x + clock.width, 1);
    EXPECT_DOUBLE_EQ(32, clock.height);
    // 6 + text + 12 + 12.
    EXPECT_GT(clock.width, 30);

    // The control centre's icon: 12 + 16 + 12, right before the clock.
    const ui::RectF control = onScreen(bar->controlIcon());
    EXPECT_DOUBLE_EQ(40, control.width);
    EXPECT_DOUBLE_EQ(clock.x, control.x + control.width);
    EXPECT_EQ(shell::shellDataFile("statusbar/light/control.svg").empty(), false);

    // Hovering lights nothing; checked, the plate shows inset 3 each side,
    // 4 from the top, 24 tall.
    move(control.x + 20, 16);
    EXPECT_FALSE(bar->controlIcon()->isPlateShown());
    bar->controlIcon()->setChecked(true);
    EXPECT_TRUE(bar->controlIcon()->isPlateShown());
    const ui::RectF plate = bar->controlIcon()->plateRect();
    EXPECT_DOUBLE_EQ(3, plate.x);
    EXPECT_DOUBLE_EQ(34, plate.width);
    EXPECT_DOUBLE_EQ(4, plate.y);
    EXPECT_DOUBLE_EQ(24, plate.height);
    // Dark contents over a light theme: black at 0.1.
    EXPECT_FALSE(bar->lightContents());
    EXPECT_EQ(26, bar->controlIcon()->plateColor().a);
    EXPECT_EQ(0, bar->controlIcon()->plateColor().r);
    bar->controlIcon()->setChecked(false);

    // Without indicators there is no volume or Wi-Fi icon.
    EXPECT_FALSE(bar->volumeIcon()->isVisible());
    EXPECT_FALSE(bar->networkIcon()->isVisible());
    // The battery shows when the system has one.
    EXPECT_EQ(bar->statusBar().battery().isAvailable(), bar->batteryIcon()->isVisible());
}

TEST_F(StatusBarViewTest, RecordingUsesAnAnimatedClockCapsuleAndCanBeStoppedThere)
{
    int stopRequests = 0;
    bar->onStopRecording = [&] { ++stopRequests; };

    bar->setRecording(true);
    EXPECT_TRUE(bar->clock()->isRecording());
    frame();
    click(onScreen(bar->clock()).x + onScreen(bar->clock()).width / 2, 16);
    EXPECT_EQ(1, stopRequests);

    bar->setRecording(false);
    EXPECT_FALSE(bar->clock()->isRecording());
    frame();
}

TEST_F(StatusBarViewTest, AnUnchangedClockDrawsNoFrame)
{
    // Not across a minute's end.
    if (std::time(nullptr) % 60 >= 58)
        std::this_thread::sleep_for(std::chrono::seconds(3));
    bar->updateClock();
    frame();

    const ui::FrameStatistics &statistics = bar->window()->window()->frameStatistics();
    const uint64_t rendered = statistics.rendered;
    bar->updateClock();
    frame();
    EXPECT_EQ(rendered, statistics.rendered);
    // Hovering changes nothing either.
    move(onScreen(bar->clock()).x + 20, 16);
    EXPECT_EQ(rendered, statistics.rendered);

    bar->clock()->setChecked(true);
    frame();
    EXPECT_GT(statistics.rendered, rendered);
}

TEST_F(StatusBarViewTest, IndicatorsPickTheIcons)
{
    FakeIndicators indicators;
    indicators.m_audio = {true, false, 0.5};
    indicators.m_network = {true, true, false, "Home", "network-wireless-connected-66"};
    bar->setIndicators(&indicators);
    frame();
    EXPECT_TRUE(bar->volumeIcon()->isVisible());
    EXPECT_EQ("audio-volume-medium-symbolic", bar->volumeIcon()->iconName());
    EXPECT_EQ("network-wireless-connected-75", bar->networkIcon()->iconName());

    indicators.m_audio = {true, false, 0.2};
    indicators.m_network = {true, true, false, "", ""};
    indicators.changed();
    EXPECT_EQ("audio-volume-low-symbolic", bar->volumeIcon()->iconName());
    EXPECT_EQ("network-wireless-connected-00", bar->networkIcon()->iconName());

    indicators.m_audio = {true, true, 0.9};
    indicators.m_network = {true, false, false, "", ""};
    indicators.changed();
    EXPECT_EQ("audio-volume-muted-symbolic", bar->volumeIcon()->iconName());
    EXPECT_EQ("network-wireless-disabled", bar->networkIcon()->glyph());

    indicators.m_audio = {true, false, 0.9};
    indicators.m_network = {true, true, true, "", ""};
    indicators.changed();
    EXPECT_EQ("audio-volume-high-symbolic", bar->volumeIcon()->iconName());
    EXPECT_EQ("network-wireless-hotspot", bar->networkIcon()->glyph());

    // The privacy dot widens the control icon by the gap and the dot.
    bar->setDeviceUsage(std::make_unique<shell::DeviceUsage>(nullptr, nullptr, nullptr));
    bar->deviceUsage()->setUsesForTesting({{shell::DeviceUse::Microphone, 42, {}, "recorder"}});
    frame();
    EXPECT_DOUBLE_EQ(12 + 16 + 6 + 6 + 12, onScreen(bar->controlIcon()).width);
    ASSERT_EQ(1u, bar->deviceUsage()->applications().size());
    EXPECT_EQ("recorder", bar->deviceUsage()->applications().front().name);
    EXPECT_TRUE(bar->deviceUsage()->applications().front().microphone);

    indicators.m_network.wirelessHwEnabled = false;
    indicators.m_audio.hasSink = false;
    indicators.changed();
    EXPECT_FALSE(bar->volumeIcon()->isVisible());
    EXPECT_FALSE(bar->networkIcon()->isVisible());
    bar->setIndicators(nullptr);
}

TEST_F(StatusBarViewTest, AnIconTogglesItsPanelUnderItsPlate)
{
    ASSERT_TRUE(test::runUntil([&] { return bar->statusBar().statusSettings().isReady(); }));
    // Exercise plate-only blur explicitly; normal startup now blurs the bar.
    bar->statusBar().statusSettings().setOpacity(0.0);
    frame();
    FakePanels panels;
    bar->addPanels(&panels);
    const ui::RectF control = onScreen(bar->controlIcon());
    click(control.x + 20, 16);
    ASSERT_EQ(1u, panels.opened.size());
    EXPECT_EQ(Panel::ControlCenter, panels.opened.front());
    EXPECT_EQ(bar->window(), panels.lastBar);
    EXPECT_EQ(int(control.x), panels.lastAnchor.x());
    EXPECT_EQ(32, panels.lastAnchor.height());
    EXPECT_TRUE(bar->controlIcon()->isChecked());
    EXPECT_TRUE(bar->panelOpen());

    // The plate shown is blurred behind, with no outline; it keeps its own colour.
    const compositor::Surface *surface = bar->window()->surface();
    const auto blurBehind = [&](const shell::BarItem *item) {
        const ui::RectF plate = item->plateRect();
        const ui::PointF origin = item->mapToWindow({plate.x, plate.y});
        const int left = int(std::floor(origin.x));
        return base::Rect(left, 4, int(std::ceil(origin.x + plate.width)) - left, 24);
    };
    EXPECT_EQ(blurBehind(bar->controlIcon()), surface->backgroundBlurRect());
    EXPECT_EQ(8, surface->backgroundBlurRadius());
    EXPECT_FALSE(surface->glass().enabled);
    EXPECT_FALSE(surface->backgroundBlurShape().isEmpty());
    EXPECT_FALSE(wm::WindowVisualController::evaluate(*bar->window()->wmWindow()).drawOutline);

    const ui::RectF clock = onScreen(bar->clock());

    // Its icon again closes it, and opens nothing.
    click(control.x + 20, 16);
    EXPECT_EQ(1u, panels.opened.size());
    EXPECT_FALSE(bar->controlIcon()->isChecked());
    EXPECT_FALSE(bar->panelOpen());

    // The plate and its blur go with the panel; hovering brings neither back.
    EXPECT_FALSE(bar->controlIcon()->isPlateShown());
    EXPECT_TRUE(surface->backgroundBlurRect().isEmpty());
    move(clock.x + 20, 16);
    EXPECT_FALSE(bar->clock()->isPlateShown());
    EXPECT_TRUE(surface->backgroundBlurRect().isEmpty());

    // The clock's panel is nobody's here.
    click(clock.x + 20, 16);
    EXPECT_EQ(1u, panels.opened.size());
    bar->removePanels(&panels);
}

TEST_F(StatusBarViewTest, ThePictureBehindTheBarPicksItsColours)
{
    using Backdrop = shell::StatusBarBackdrop;
    Backdrop::Sample sample;
    sample.valid = true;
    // A smooth dark strip: light contents, no gradient.
    sample.red = sample.green = sample.blue = 20;
    Backdrop::Style style = Backdrop::styleForSample(sample);
    EXPECT_TRUE(style.useLightForeground);
    EXPECT_FALSE(style.gradientVisible);
    // A smooth mid tone: dark contents on a light gradient.
    sample.red = sample.green = sample.blue = 170;
    style = Backdrop::styleForSample(sample);
    EXPECT_FALSE(style.useLightForeground);
    EXPECT_TRUE(style.gradientVisible);
    EXPECT_EQ(115, style.gradientColor.a);
    // Busy: light contents up to a lighter average, and a dark gradient.
    sample.detail = 9;
    sample.red = sample.green = sample.blue = 150;
    style = Backdrop::styleForSample(sample);
    EXPECT_TRUE(style.useLightForeground);
    EXPECT_TRUE(style.gradientVisible);
    EXPECT_EQ(77, style.gradientColor.a);

    // The strip's mean colour and its texture.
    ui::Bitmap stripes(512, 64, ui::Bitmap::Format::Argb32Premultiplied);
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 512; ++x)
            stripes.setPixel(x, y, y < 32 ? 0xff102030u : 0xffffffffu);
    }
    const Backdrop::Sample flat = Backdrop::sampleImage(stripes, 32);
    EXPECT_TRUE(flat.valid);
    EXPECT_NEAR(0x10, flat.red, 0.5);
    EXPECT_NEAR(0x30, flat.blue, 0.5);
    EXPECT_NEAR(0, flat.detail, 1e-9);

    // A dark colour behind the bar: white contents, with the shadow edge.
    bar->backdrop().setColor(ui::Color::rgb(0x101820));
    EXPECT_TRUE(bar->lightContents());
    EXPECT_EQ(ui::Color::rgb(0xFFFFFF), bar->textColor());
    // The desktop's picture, judged once decoded on a worker.
    ui::Bitmap light(64, 64, ui::Bitmap::Format::Argb32Premultiplied);
    light.fill(0xfff0f0f0u);
    ASSERT_TRUE(light.savePng(directory.filePath("light.png")));
    auto *wallpaper = shell->addComponent(std::make_unique<shell::Wallpaper>(shell.get(), directory.filePath("cache")));
    shell::Wallpaper::Background background;
    background.path = directory.filePath("light.png");
    wallpaper->setBackground(background);
    bar->setWallpaper(wallpaper);
    EXPECT_TRUE(test::runUntil([&] { return !bar->lightContents(); }));
    EXPECT_EQ((ui::Color {0, 0, 0, 217}), bar->textColor());
    EXPECT_EQ(ui::Color::rgb(0x000000), bar->menuTextColor());
    bar->setWallpaper(nullptr);
    // Nothing shown: the theme decides again.
    bar->backdrop().setUnavailable();
    EXPECT_FALSE(bar->backdrop().isValid());
}

TEST_F(StatusBarViewTest, TrayItemsSitRightToLeftAndTakeClicks)
{
    ASSERT_TRUE(bar->statusBar().registerServices());
    test::DBusTestClient application;
    test::FakeTrayItem item(application.connection.get());
    item.menuPath = "/MenuBar";
    item.iconName = "no-such-icon-anywhere";
    test::FakeMenuExporter exporter(application.connection.get(), "/MenuBar");
    exporter.layouts[0] = test::layout(0, {},
                                       {test::layout(1, {{"label", dbus::Variant(std::string("_Open"))}}),
                                        test::layout(2, {{"type", dbus::Variant(std::string("separator"))}}),
                                        test::layout(3, {{"label", dbus::Variant(std::string("_Quit"))}})});
    application.call("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher", "org.kde.StatusNotifierWatcher",
                     "RegisterStatusNotifierItem", std::string("/StatusNotifierItem"));
    ASSERT_TRUE(test::runUntil([&] { return bar->tray()->count() == 1; }));
    frame();

    // 40 wide, right before the volume, Wi-Fi, battery and control icons.
    shell::TrayItemView *view = bar->tray()->item(0);
    const ui::RectF slot = onScreen(view);
    EXPECT_DOUBLE_EQ(40, slot.width);
    const ui::RectF tray = onScreen(bar->tray());
    EXPECT_DOUBLE_EQ(tray.x + tray.width, slot.x + slot.width);

    // Left: activated at the click.
    click(slot.x + 20, 16);
    ASSERT_TRUE(test::runUntil([&] { return !item.calls.empty(); }));
    EXPECT_EQ("Activate " + std::to_string(int(slot.x + 20)) + ",16", item.calls.front());

    // Right: its menu under the plate, the plate lit while it is up.
    ASSERT_TRUE(test::runUntil([&] { return bar->statusBar().systemTray().at(0)->menu(); }));
    click(slot.x + 20, 16, BTN_RIGHT);
    EXPECT_TRUE(view->isPlateShown());
    ASSERT_TRUE(test::runUntil([&] { return bar->tray()->openMenu() != nullptr; })) << item.calls.size();
    EXPECT_TRUE(view->isPlateShown());
    EXPECT_TRUE(test::runUntil([&] { return exporter.called("AboutToShow 0"); }));
    // A right click on it again closes it.
    click(slot.x + 20, 16, BTN_RIGHT);
    EXPECT_EQ(nullptr, bar->tray()->openMenu());
    move(700, 500);
    EXPECT_FALSE(view->isPlateShown());
    EXPECT_TRUE(test::runUntil([&] { return exporter.called("Event 0 closed"); }));
}

TEST_F(StatusBarViewTest, ARightClickOpensTheTrayMenuOnce)
{
    ASSERT_TRUE(bar->statusBar().registerServices());
    test::DBusTestClient application;
    test::FakeTrayItem item(application.connection.get());
    item.menuPath = "/MenuBar";
    test::FakeMenuExporter exporter(application.connection.get(), "/MenuBar");
    exporter.layouts[0] = test::layout(0, {}, {test::layout(1, {{"label", dbus::Variant(std::string("_Open"))}})});
    application.call("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher", "org.kde.StatusNotifierWatcher",
                     "RegisterStatusNotifierItem", std::string("/StatusNotifierItem"));
    ASSERT_TRUE(test::runUntil([&] { return bar->tray()->count() == 1; }));
    frame();
    ASSERT_TRUE(test::runUntil([&] { return bar->statusBar().systemTray().at(0)->menu(); }));
    ASSERT_TRUE(test::runUntil([&] { return exporter.called("GetLayout 0 1"); }));

    const ui::RectF slot = onScreen(bar->tray()->item(0));
    click(slot.x + 20, 16, BTN_RIGHT);
    ASSERT_TRUE(test::runUntil([&] { return bar->tray()->openMenu() != nullptr; }));
    ASSERT_TRUE(test::runUntil([&] { return exporter.called("Event 0 opened"); }));
    test::runFor(std::chrono::milliseconds(100));

    // AboutToShow, then "opened" once the menu is up, and "closed" as it goes.
    std::vector<std::string> events;
    for (const std::string &call : exporter.calls) {
        if (!call.starts_with("GetLayout"))
            events.push_back(call);
    }
    EXPECT_EQ((std::vector<std::string>{"AboutToShow 0", "Event 0 opened"}), events);
    EXPECT_TRUE(item.calls.empty());

    click(slot.x + 20, 16, BTN_RIGHT);
    EXPECT_EQ(nullptr, bar->tray()->openMenu());
    ASSERT_TRUE(test::runUntil([&] { return exporter.called("Event 0 closed"); }));
    test::runFor(std::chrono::milliseconds(50));
    EXPECT_EQ(1, std::count(exporter.calls.begin(), exporter.calls.end(), "Event 0 closed"));
}

TEST_F(StatusBarViewTest, ALeftClickOpensTheMenuOfAnItemThatIsOnlyAMenu)
{
    ASSERT_TRUE(bar->statusBar().registerServices());
    test::DBusTestClient application;
    test::FakeTrayItem item(application.connection.get());
    item.menuPath = "/MenuBar";
    item.itemIsMenu = true;
    test::FakeMenuExporter exporter(application.connection.get(), "/MenuBar");
    exporter.layouts[0] = test::layout(0, {}, {test::layout(1, {{"label", dbus::Variant(std::string("_Open"))}})});
    application.call("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher", "org.kde.StatusNotifierWatcher",
                     "RegisterStatusNotifierItem", std::string("/StatusNotifierItem"));
    ASSERT_TRUE(test::runUntil([&] { return bar->tray()->count() == 1; }));
    frame();
    ASSERT_TRUE(test::runUntil([&] { return bar->statusBar().systemTray().at(0)->menu(); }));

    const ui::RectF slot = onScreen(bar->tray()->item(0));
    click(slot.x + 20, 16);
    ASSERT_TRUE(test::runUntil([&] { return bar->tray()->openMenu() != nullptr; }));
    EXPECT_TRUE(exporter.called("AboutToShow 0"));
    // Again, it closes.
    click(slot.x + 20, 16);
    EXPECT_EQ(nullptr, bar->tray()->openMenu());
    test::runFor(std::chrono::milliseconds(50));
    EXPECT_TRUE(item.calls.empty());

    // No longer only a menu: a left click activates it again.
    item.itemIsMenu = false;
    item.object.emitSignal("NewTitle");
    test::runFor(std::chrono::milliseconds(100));
    click(slot.x + 20, 16);
    ASSERT_TRUE(test::runUntil([&] { return !item.calls.empty(); }));
    EXPECT_EQ("Activate " + std::to_string(int(slot.x + 20)) + ",16", item.calls.front());
    EXPECT_EQ(nullptr, bar->tray()->openMenu());
}

TEST_F(StatusBarViewTest, AnItemThatLeavesTakesItsOpenMenu)
{
    ASSERT_TRUE(bar->statusBar().registerServices());
    auto application = std::make_unique<test::DBusTestClient>();
    auto item = std::make_unique<test::FakeTrayItem>(application->connection.get());
    item->menuPath = "/MenuBar";
    auto exporter = std::make_unique<test::FakeMenuExporter>(application->connection.get(), "/MenuBar");
    exporter->layouts[0] = test::layout(0, {},
                                        {test::layout(1, {{"label", dbus::Variant(std::string("_Open"))}}),
                                         test::layout(2, {{"label", dbus::Variant(std::string("_Recent"))},
                                                          {"children-display", dbus::Variant(std::string("submenu"))}})});
    application->call("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher", "org.kde.StatusNotifierWatcher",
                      "RegisterStatusNotifierItem", std::string("/StatusNotifierItem"));
    ASSERT_TRUE(test::runUntil([&] { return bar->tray()->count() == 1; }));
    frame();
    ASSERT_TRUE(test::runUntil([&] { return bar->statusBar().systemTray().at(0)->menu(); }));

    const ui::RectF slot = onScreen(bar->tray()->item(0));
    click(slot.x + 20, 16, BTN_RIGHT);
    ASSERT_TRUE(test::runUntil([&] { return bar->tray()->openMenu() != nullptr; }));

    // The application quits with its menu up: the menu closes without
    // reaching the item's menu client, which went first.
    exporter.reset();
    item.reset();
    application.reset();
    ASSERT_TRUE(test::runUntil([&] { return bar->tray()->count() == 0; }));
    frame();
    EXPECT_EQ(nullptr, bar->tray()->openMenu());
}

TEST_F(StatusBarViewTest, ApplicationItemsSitBeforeTheTrayOnWholePixels)
{
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 20);
    struct Heard {
        int height = 0;
        int maxWidth = 0;
        bool dark = true;
        uint32_t foreground = 0;
        bool highlighted = false;
        base::Rect place;
    };
    const auto client = [](Heard *heard) {
        wm::StatusItem::Client client;
        client.configure = [heard](int height, int maxWidth) {
            heard->height = height;
            heard->maxWidth = maxWidth;
            return 1u;
        };
        client.appearance = [heard](bool dark, uint32_t foreground) {
            heard->dark = dark;
            heard->foreground = foreground;
        };
        client.highlight = [heard](bool highlighted) { heard->highlighted = highlighted; };
        client.placed = [heard](const base::Rect &rect) { heard->place = rect; };
        return client;
    };
    const auto draw = [&](compositor::Surface *surface, int width) {
        surface->attach(pool->createBuffer({width, 32}, 0, width * 4, render::PixelFormat::Bgra8888, nullptr), {});
        surface->damage(base::Rect(0, 0, width, 32));
        surface->commit();
    };

    std::unique_ptr<compositor::Surface> firstSurface = compositor.createSurface();
    std::unique_ptr<compositor::Surface> secondSurface = compositor.createSurface();
    Heard firstHeard;
    Heard secondHeard;
    auto first = std::make_unique<wm::StatusItem>(manager.statusItems(), firstSurface.get(), "app", client(&firstHeard));
    auto second = std::make_unique<wm::StatusItem>(manager.statusItems(), secondSurface.get(), "app",
                                                   client(&secondHeard));
    // The bar's height, and a third of its width at most; its look.
    EXPECT_EQ(32, firstHeard.height);
    EXPECT_EQ(480, firstHeard.maxWidth);
    EXPECT_EQ(bar->lightContents(), firstHeard.dark);
    EXPECT_EQ(bar->textColor().argb(), firstHeard.foreground);
    EXPECT_EQ(0, bar->statusItems()->count());

    ASSERT_TRUE(first->acknowledgeConfigure(1));
    ASSERT_TRUE(second->acknowledgeConfigure(1));
    draw(firstSurface.get(), 37);
    draw(secondSurface.get(), 52);
    frame();
    frame();
    ASSERT_EQ(2, bar->statusItems()->count());

    // The newest leftmost, each slot 6 wider than its item, the last one right
    // before the tray's place; every item on whole logical pixels.
    shell::StatusItemSlot *left = bar->statusItems()->slot(0);
    shell::StatusItemSlot *right = bar->statusItems()->slot(1);
    EXPECT_EQ(second.get(), left->item());
    EXPECT_EQ(first.get(), right->item());
    const ui::RectF leftSlot = onScreen(left);
    const ui::RectF rightSlot = onScreen(right);
    EXPECT_DOUBLE_EQ(58, leftSlot.width);
    EXPECT_DOUBLE_EQ(43, rightSlot.width);
    EXPECT_DOUBLE_EQ(leftSlot.x + leftSlot.width, rightSlot.x);
    const ui::RectF next = onScreen(bar->batteryIcon()->isVisible() ? static_cast<ui::View *>(bar->batteryIcon())
                                                                     : bar->controlIcon());
    EXPECT_NEAR(next.x, rightSlot.x + rightSlot.width, 1);
    EXPECT_EQ(base::Rect(int(leftSlot.x) + 3, 0, 52, 32), second->screenRect());
    EXPECT_DOUBLE_EQ(std::round(rightSlot.x + 3), rightSlot.x + 3);
    EXPECT_EQ(base::Rect(int(rightSlot.x) + 3, 0, 37, 32), first->screenRect());
    EXPECT_EQ(first->screenRect(), firstHeard.place);
    EXPECT_EQ(bar->window()->surface(), firstSurface->parentSurface());

    // A new width shows with the bar's next frame, which moves the items
    // beside it in the same commit: no frame shows the item at its old place.
    const int rightEdge = first->screenRect().right();
    draw(firstSurface.get(), 60);
    EXPECT_EQ(60, first->width());
    EXPECT_EQ(37, firstSurface->contentSize().width());
    frame();
    EXPECT_EQ(60, firstSurface->contentSize().width());
    EXPECT_EQ(rightEdge, first->screenRect().right());
    EXPECT_EQ(base::Rect(rightEdge - 60, 0, 60, 32), first->screenRect());
    EXPECT_EQ(first->screenRect().x() - 6 - 52, second->screenRect().x());
    draw(firstSurface.get(), 37);
    frame();
    EXPECT_EQ(rightEdge, first->screenRect().right());

    // Asked for, the bar's plate lies exactly under the item.
    first->setHighlightRequested(true);
    frame();
    EXPECT_TRUE(firstHeard.highlighted);
    EXPECT_TRUE(right->isPlateShown());
    const ui::RectF plate = right->plateRect();
    EXPECT_DOUBLE_EQ(3, plate.x);
    EXPECT_DOUBLE_EQ(37, plate.width);

    // An item that goes leaves its slot to the others.
    second.reset();
    frame();
    ASSERT_EQ(1, bar->statusItems()->count());
    EXPECT_DOUBLE_EQ(rightSlot.x, onScreen(bar->statusItems()->slot(0)).x);
    first.reset();
    frame();
    EXPECT_EQ(0, bar->statusItems()->count());
    EXPECT_FALSE(bar->statusItems()->isVisible());
}

TEST(DBusMenuPopupTest, LabelsLoseTheirMnemonics)
{
    EXPECT_EQ("File", shell::DBusMenuPopup::plainLabel("_File"));
    EXPECT_EQ("Save_As", shell::DBusMenuPopup::plainLabel("Save__As"));
    EXPECT_EQ("Tom & Jerry", shell::DBusMenuPopup::plainLabel("Tom & Jerry"));
}

TEST_F(StatusBarViewTest, EfficientModeMovesSystemItemsBesideTheDockAndRestoresTopBar)
{
    auto &applications = static_cast<test::FakeApplications &>(shell->applications());
    applications.addSimple("files", "Files", "system-file-manager");
    applications.addSimple("terminal", "Terminal", "utilities-terminal");
    applications.addSimple("settings", "Settings", "preferences-system");
    applications.scan();
    directory.write("shell.conf", "[Dock]\nPinned=files,terminal,settings\n");
    auto backend = std::make_unique<shell::Dock>(sessionBus(), shell->tasks(), shell->launcher(),
        directory.filePath("shell.conf"));
    auto *dock = shell->addComponent(std::make_unique<shell::DockWindow>(shell.get(), std::move(backend)));
    ASSERT_TRUE(test::runUntil([&] { return dock->dock().settings().isReady(); }));
    bar->followDesktopMode(dock->dock().settings());
    auto connection = bar->taskbarWidthChanged.connect([&](int width) { dock->setTaskbarRightWidth(width); });
    dock->setTaskbarRightWidth(bar->taskbarWidth());
    FakeIndicators indicators;
    indicators.m_audio = {true, false, 0.5};
    indicators.m_network = {true, true, false, "Test network", "network-wireless-connected-66"};
    bar->setIndicators(&indicators);
    FakePanels panels;
    panels.notifications = true;
    bar->addPanels(&panels);
    for (int repeat = 0; repeat < 3; ++repeat) {
        dock->dock().settings().setEfficientMode(true);
        frame();
        const auto left = dock->window()->geometry();
        const auto right = bar->window()->geometry();
        EXPECT_EQ(left.y(), 960 - shell::DockSettings::taskbarHeight);
        EXPECT_EQ(right.y(), left.y());
        EXPECT_EQ(left.height(), shell::DockSettings::taskbarHeight);
        EXPECT_EQ(left.x(), 0);
        EXPECT_EQ(left.right(), right.x());
        EXPECT_EQ(right.right(), 1440);
        EXPECT_EQ(dock->panelRect().width(), left.width());
        EXPECT_EQ(dock->cornerRadius(), 0);
        EXPECT_EQ(shell->workArea(*shell->output({})), base::Rect(0, 0, 1440, 908));
        EXPECT_TRUE(bar->clock()->isVisible());
        EXPECT_TRUE(bar->notificationIcon()->isVisible());
        bar->togglePanel(Panel::NotificationCenter);
        EXPECT_EQ(panels.opened.back(), Panel::NotificationCenter);
        EXPECT_EQ(panels.lastAnchor.height(), shell::DockSettings::taskbarHeight);
        EXPECT_TRUE(bar->notificationIcon()->isChecked());
        const char *captures = std::getenv("LINGMO_TASKBAR_CAPTURES");
        if (captures && repeat == 0) {
            std::filesystem::create_directories(captures);
            EXPECT_TRUE(dock->window()->window()->grab().savePng(std::string(captures) + "/taskbar-apps.png"));
            EXPECT_TRUE(bar->window()->window()->grab().savePng(std::string(captures) + "/taskbar-status.png"));
            const auto apps = dock->window()->window()->grab();
            const auto status = bar->window()->window()->grab();
            ui::Image combined(apps.width() + status.width(), apps.height());
            for (int y = 0; y < combined.height(); ++y)
                for (int x = 0; x < combined.width(); ++x)
                    combined.setPixel(x, y, x < apps.width() ? apps.pixel(x, y) : status.pixel(x - apps.width(), y));
            EXPECT_TRUE(combined.savePng(std::string(captures) + "/taskbar.png"));
        }
        dock->dock().settings().setEfficientMode(false);
        frame();
        EXPECT_FALSE(panels.isOpen(Panel::NotificationCenter));
        EXPECT_FALSE(bar->notificationIcon()->isVisible());
        EXPECT_EQ(bar->window()->geometry(), base::Rect(0, 0, 1440, 32));
        EXPECT_EQ(shell->workArea(*shell->output({})).y(), 32);
    }
    dock->dock().settings().setEfficientMode(true);
    manager.setOutputBounds({0, 0, 1280, 800});
    test::runFor(std::chrono::milliseconds(20)); frame();
    EXPECT_EQ(bar->window()->geometry().bottom(), 800);
    EXPECT_EQ(dock->window()->geometry().right(), bar->window()->geometry().x());
    bar->setIndicators(nullptr);
    bar->removePanels(&panels);
}
