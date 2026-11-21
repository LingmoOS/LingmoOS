// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The control centre, the status panels and the power dialog as the
// compositor draws them: their popups, places, glass, input and behaviour,
// headless on a fake renderer with fake services.

#include <chrono>
#include <cmath>

#include <linux/input-event-codes.h>

#include <gtest/gtest.h>

#include "base/dbus.h"
#include "compositor/compositor.h"
#include "core/main_loop.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shell/controlcenter/control_center.h"
#include "shell/controlcenter/control_center_view.h"
#include "shell/controlcenter/level_slider.h"
#include "shell/controlcenter/power_dialog.h"
#include "shell/controlcenter/status_panel.h"
#include "shell/controlcenter/tiles.h"
#include "shell/osd/level_track.h"
#include "shell/shell.h"
#include "shell/shell_data.h"
#include "shell/shell_window.h"
#include "shell/statusbar/status_bar.h"
#include "shell/statusbar/status_bar_window.h"
#include "shell/statusbar/status_icon.h"
#include "shell_test_util.h"
#include "temporary_directory.h"
#include "ui/application.h"
#include "ui/dialog_window.h"
#include "ui/switch.h"
#include "ui/window.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;
using Panel = shell::StatusBarPanels::Panel;

namespace {

input::Timestamp at(int ms)
{
    return input::Timestamp(std::chrono::milliseconds(ms));
}

const base::Rect kOutput(0, 0, 1440, 960);
constexpr int kStatusBar = 32;
constexpr int kDock = 92;

class FakeServices final : public shell::ControlCenterServices
{
public:
    bool wirelessHardwareEnabled() const override { return hardware; }
    bool wirelessEnabled() const override { return wireless; }
    bool hotspotActive() const override { return hotspot; }
    std::string hotspotName() const override { return "Laptop"; }
    std::string wirelessName() const override { return wireless ? "Office 5G" : ""; }
    std::string wirelessIcon() const override { return "network-wireless-connected-100"; }
    std::vector<shell::WirelessNetwork> wirelessNetworks() const override { return wireless ? networks : decltype(networks)(); }
    void enableWireless(bool enable) override { wireless = enable; changed(); }
    void requestScan() override { ++scans; }
    void stopHotspot() override { hotspot = false; changed(); }
    bool activateNetwork(const shell::WirelessNetwork &network) override
    {
        activated.push_back(network.name);
        return network.security == shell::WirelessNetwork::Security::Password && network.connectionPath.empty();
    }
    bool bluetoothAvailable() const override { return true; }
    bool bluetoothEnabled() const override { return bluetooth; }
    void setBluetoothEnabled(bool enabled) override { bluetooth = enabled; changed(); }
    std::vector<shell::BluetoothDevice> bluetoothDevices() const override { return bluetoothList; }
    void requestBluetoothScan() override { ++bluetoothScans; }
    void stopBluetoothScan() override { ++bluetoothScanStops; }
    void toggleBluetoothDevice(const shell::BluetoothDevice &device) override
    {
        bluetoothToggled.push_back(device.path);
    }
    void setDarkMode(bool dark) override { darkRequested = dark ? 1 : 0; }
    bool doNotDisturb() const override { return dnd; }
    void setDoNotDisturb(bool enabled) override { dnd = enabled; changed(); }
    bool cameraEnabled() const override { return camera; }
    void setCameraEnabled(bool enabled) override { camera = enabled; changed(); }
    bool microphoneEnabled() const override { return microphone; }
    void setMicrophoneEnabled(bool enabled) override { microphone = enabled; changed(); }
    std::vector<shell::DeviceUser> deviceUsers() const override { return users; }
    shell::MediaState media() const override { return mediaState; }
    void mediaPrevious() override { media_.push_back("previous"); }
    void mediaPlay() override { media_.push_back("play"); }
    void mediaPause() override { media_.push_back("pause"); }
    void mediaNext() override { media_.push_back("next"); }
    bool brightnessEnabled() const override { return true; }
    int brightness() const override { return brightnessValue; }
    void setBrightness(int value) override { brightnessValue = value; ++brightnessWrites; }
    bool hasSink() const override { return sink; }
    double volume() const override { return volumeValue; }
    bool muted() const override { return mutedValue; }
    void setVolume(double volume) override { volumeValue = volume; }
    void setMuted(bool muted) override { mutedValue = muted; }
    std::vector<shell::AudioOutput> outputs() const override { return outputList; }
    void setDefaultOutput(uint32_t id) override { defaultRequested = int(id); }
    int chargePercent() const override { return 80; }
    bool onBattery() const override { return false; }
    std::string batteryStatus() const override { return "Charging paused at 80%"; }
    std::vector<std::string> powerProfiles() const override { return profiles; }
    std::string powerProfile() const override { return profile; }
    void setPowerProfile(const std::string &value) override { profile = value; changed(); }
    void openSettings(const std::vector<std::string> &arguments) override { settings.push_back(arguments); }

    bool hardware = true;
    bool wireless = true;
    bool hotspot = false;
    bool bluetooth = true;
    bool dnd = false;
    bool camera = false;
    bool microphone = false;
    int darkRequested = -1;
    int scans = 0;
    int bluetoothScans = 0;
    int bluetoothScanStops = 0;
    int brightnessValue = 63;
    int brightnessWrites = 0;
    bool sink = true;
    double volumeValue = 0.36;
    bool mutedValue = false;
    int defaultRequested = -1;
    std::string profile = "balanced";
    std::vector<std::string> profiles {"power-saver", "balanced"};
    std::vector<shell::WirelessNetwork> networks;
    std::vector<shell::BluetoothDevice> bluetoothList;
    std::vector<std::string> bluetoothToggled;
    std::vector<std::string> activated;
    std::vector<shell::DeviceUser> users;
    shell::MediaState mediaState;
    std::vector<std::string> media_;
    std::vector<shell::AudioOutput> outputList {{1, "Speakers", true}, {2, "HDMI", false}};
    std::vector<std::vector<std::string>> settings;
};

shell::WirelessNetwork network(const std::string &name, shell::WirelessNetwork::State state,
                               shell::WirelessNetwork::Security security, const std::string &connection = {})
{
    shell::WirelessNetwork result;
    result.key = name;
    result.name = name;
    result.icon = "network-wireless-connected-66";
    result.state = state;
    result.security = security;
    result.connectionPath = connection;
    result.devicePath = "/dev/1";
    result.specificPath = "/ap/" + name;
    return result;
}

class ControlCenterTest : public testing::Test
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
    std::unique_ptr<shell::ShellWindow> bar;
    FakeServices *services = nullptr;
    shell::ControlCenter *center = nullptr;
    int dock = 0;
    int time = 1000;

    void SetUp() override
    {
        shell::setShellDataDirectoryForTesting(LINGMO_SOURCE_DIR "/data/shell");
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
        manager.setExclusiveZone(&dock, wm::ScreenEdge::Bottom, kDock);
        input.setBounds(kOutput.size());
        input.setEventSink(&manager);
        shell::ShellEnvironment environment {&compositor, &manager, &seat};
        environment.servicesSettings = directory.filePath("services.conf");
        shell = std::make_unique<shell::Shell>(environment);
        shell->setApplications(std::make_unique<test::FakeApplications>());
        shell->setTaskWindows(std::make_unique<test::FakeTaskWindows>());

        // The status bar's window, which the panels hang from.
        bar = std::make_unique<shell::ShellWindow>(shell.get(), wm::WindowRole::Panel);
        bar->setContentView(std::make_unique<ui::View>());
        bar->setFocusPolicy(wm::FocusPolicy::NoFocus);
        bar->setGeometry(base::Rect(0, 0, kOutput.width(), kStatusBar));
        bar->setExclusiveZone(wm::ScreenEdge::Top, kStatusBar);
        bar->show();

        auto fake = std::make_unique<FakeServices>();
        services = fake.get();
        center = shell->addComponent(std::make_unique<shell::ControlCenter>(shell.get(), std::move(fake), nullptr));
        frame();
    }

    void TearDown() override
    {
        // The bar outlives the panels hanging from it, the shell the bar.
        center->closePanels();
        test::runFor(std::chrono::milliseconds(5));
        center = nullptr;
        bar.reset();
        shell.reset();
        seat.shutdown();
    }

    void frame() { compositor.updateTextures(renderer); }

    void move(double x, double y) { input.onPointerMotionAbsolute(at(time += 5), {x, y}); }

    void press(double x, double y)
    {
        move(x, y);
        input.onPointerButton(at(time += 5), BTN_LEFT, true);
    }

    void release() { input.onPointerButton(at(time += 5), BTN_LEFT, false); }

    void click(double x, double y)
    {
        press(x, y);
        release();
        frame();
    }

    void key(uint32_t code)
    {
        input.onKey(at(time += 5), code, true);
        input.onKey(at(time += 5), code, false);
    }

    // The bar items the reference captures used: x from the right edge.
    base::Rect item(int fromRight, int width = 40) const
    {
        return base::Rect(kOutput.width() - fromRight, 0, width, kStatusBar);
    }

    // The centre of a view in global logical coordinates.
    base::PointF centre(Panel panel, const ui::View *view) const
    {
        const ui::PointF origin = view->mapToWindow({0, 0});
        const base::Rect popup = center->popupWindow(panel)->drawnBounds();
        return {popup.x() + origin.x + view->geometry().width / 2, popup.y() + origin.y + view->geometry().height / 2};
    }

    void clickView(Panel panel, const ui::View *view)
    {
        const base::PointF point = centre(panel, view);
        click(point.x(), point.y());
    }
};

} // namespace

TEST_F(ControlCenterTest, TheControlCentreHangsBelowTheBarWithThePanelGlass)
{
    int changes = 0;
    base::Subscription subscription = center->panelsChanged.connect([&] { ++changes; });
    center->openPanel(Panel::ControlCenter, bar.get(), item(178));
    frame();
    ASSERT_TRUE(center->isOpen(Panel::ControlCenter));
    EXPECT_EQ(1, changes);
    shell::ShellWindow *window = center->popupWindow(Panel::ControlCenter);
    ASSERT_NE(nullptr, window);
    ASSERT_NE(nullptr, window->wmWindow());
    EXPECT_TRUE(window->wmWindow()->isPopup());
    // Right of the screen it does not fit: 6 from the edge, 6 below the bar.
    const base::Rect bounds = window->drawnBounds();
    EXPECT_EQ(base::Rect(1440 - 360 - 6, 38, 360, bounds.height()), bounds);
    EXPECT_GT(bounds.height(), 400);
    EXPECT_TRUE(window->surface()->glass().enabled);
    EXPECT_EQ(base::Rect(0, 0, 360, bounds.height()), window->surface()->backgroundBlurRect());
    EXPECT_EQ(16, window->surface()->backgroundBlurRadius());

    // Escape closes it; the bar lends it the keyboard.
    key(KEY_ESC);
    test::runFor(std::chrono::milliseconds(20));
    EXPECT_FALSE(center->isOpen(Panel::ControlCenter));
    EXPECT_EQ(2, changes);

    // Left of the screen it starts at the item's plate.
    center->openPanel(Panel::ControlCenter, bar.get(), base::Rect(100, 0, 40, kStatusBar));
    frame();
    EXPECT_EQ(103, center->popupWindow(Panel::ControlCenter)->drawnBounds().x());
    // A press outside closes it and goes nowhere.
    click(700, 600);
    test::runFor(std::chrono::milliseconds(20));
    EXPECT_FALSE(center->isOpen(Panel::ControlCenter));
}

TEST_F(ControlCenterTest, CardsToggleTheirServicesAndHoldingOpensSettings)
{
    center->openPanel(Panel::ControlCenter, bar.get(), item(178));
    frame();
    shell::ControlCenterView *view = center->controlCenterView();
    ASSERT_NE(nullptr, view);
    EXPECT_TRUE(view->wirelessCard()->isChecked());
    EXPECT_TRUE(view->bluetoothCard()->isChecked());
    EXPECT_FALSE(view->doNotDisturbCard()->isChecked());

    clickView(Panel::ControlCenter, view->bluetoothCard());
    test::runFor(std::chrono::milliseconds(5));
    EXPECT_FALSE(services->bluetooth);
    EXPECT_FALSE(view->bluetoothCard()->isChecked());
    clickView(Panel::ControlCenter, view->doNotDisturbCard());
    clickView(Panel::ControlCenter, view->cameraCard());
    clickView(Panel::ControlCenter, view->darkModeCard());
    test::runFor(std::chrono::milliseconds(5));
    EXPECT_TRUE(services->dnd);
    EXPECT_TRUE(services->camera);
    EXPECT_EQ(1, services->darkRequested);
    EXPECT_TRUE(view->doNotDisturbCard()->isChecked());

    // Wi-Fi hardware off hides its card and the grid closes up.
    const int before = center->popupWindow(Panel::ControlCenter)->drawnBounds().height();
    services->hardware = false;
    services->bluetooth = false;
    services->changed();
    test::runFor(std::chrono::milliseconds(5));
    frame();
    EXPECT_FALSE(view->wirelessCard()->isVisible());
    EXPECT_EQ(before, center->popupWindow(Panel::ControlCenter)->drawnBounds().height());
    services->hardware = true;
    services->changed();
    test::runFor(std::chrono::milliseconds(5));
    frame();

    // Held 800 ms: the control centre goes and Settings opens on Wi-Fi.
    const base::PointF point = centre(Panel::ControlCenter, view->wirelessCard());
    press(point.x(), point.y());
    test::runFor(std::chrono::milliseconds(900));
    release();
    test::runFor(std::chrono::milliseconds(20));
    ASSERT_EQ(1u, services->settings.size());
    EXPECT_EQ((std::vector<std::string> {"-m", "wlan"}), services->settings.front());
    EXPECT_FALSE(center->isOpen(Panel::ControlCenter));
    EXPECT_TRUE(services->wireless);
}

TEST_F(ControlCenterTest, SlidersMoveTheLevelsAndTheMediaCardControlsThePlayer)
{
    center->openPanel(Panel::ControlCenter, bar.get(), item(178));
    frame();
    shell::ControlCenterView *view = center->controlCenterView();
    shell::LevelSlider *sound = view->soundSlider()->slider();
    EXPECT_NEAR(0.36, sound->position(), 1e-9);

    // A press puts the knob's centre under the pointer.
    const ui::PointF origin = sound->mapToWindow({0, 0});
    const base::Rect popup = center->popupWindow(Panel::ControlCenter)->drawnBounds();
    const double travel = sound->geometry().width - sound->geometry().height;
    click(popup.x() + origin.x + sound->geometry().height / 2 + travel * 0.5, popup.y() + origin.y + 11);
    EXPECT_NEAR(0.5, services->volumeValue, 0.011);
    EXPECT_FALSE(services->mutedValue);

    // A wheel notch moves five steps.
    input.onPointerAxis(at(time += 5), base::PointF(0, -10), true);
    frame();
    EXPECT_NEAR(0.55, services->volumeValue, 0.011);

    // The brightness slider writes once the knob rests 100 ms.
    shell::LevelSlider *display = view->brightnessSlider()->slider();
    const ui::PointF displayOrigin = display->mapToWindow({0, 0});
    click(popup.x() + displayOrigin.x + display->geometry().height / 2, popup.y() + displayOrigin.y + 11);
    EXPECT_EQ(0, services->brightnessWrites);
    test::runFor(std::chrono::milliseconds(150));
    EXPECT_EQ(1, services->brightnessWrites);
    EXPECT_EQ(1, services->brightnessValue);

    // No player: "Not Playing", three dimmed, disabled buttons.
    shell::MediaTile *media = view->media();
    EXPECT_FALSE(media->playButton()->isEnabled());
    EXPECT_DOUBLE_EQ(0.35, media->playButton()->opacity());
    services->mediaState.hasPlayer = true;
    services->mediaState.playing = true;
    services->mediaState.title = "Song";
    services->mediaState.canControl = true;
    services->mediaState.canPause = true;
    services->mediaState.canGoNext = true;
    services->changed();
    test::runFor(std::chrono::milliseconds(5));
    frame();
    EXPECT_TRUE(media->playButton()->isEnabled());
    EXPECT_FALSE(media->previousButton()->isEnabled());
    clickView(Panel::ControlCenter, media->playButton());
    clickView(Panel::ControlCenter, media->nextButton());
    clickView(Panel::ControlCenter, media->previousButton());
    EXPECT_EQ((std::vector<std::string> {"pause", "next"}), services->media_);
}

TEST_F(ControlCenterTest, TheWifiPanelListsNetworksAndJoinsThem)
{
    using State = shell::WirelessNetwork::State;
    using Security = shell::WirelessNetwork::Security;
    services->networks = {network("Office 5G", State::Activated, Security::Password, "/c/1"),
                          network("Cafe", State::Deactivated, Security::None),
                          network("Home", State::Deactivated, Security::Password),
                          network("Busy", State::Activating, Security::Password, "/c/2")};
    center->openPanel(Panel::Network, bar.get(), item(293));
    frame();
    shell::ShellWindow *window = center->popupWindow(Panel::Network);
    ASSERT_NE(nullptr, window);
    // As tall as its content, glass and input over all of it, and allowed
    // to grow to the work area's height.
    const base::Rect bounds = window->drawnBounds();
    auto *panel = static_cast<shell::NetworkPanel *>(center->statusPanel(Panel::Network));
    const int shown = int(std::lround(panel->shownBottom()));
    EXPECT_EQ(int(panel->contentHeight()), shown);
    // Kept 6 inside the work area's right edge.
    EXPECT_EQ(base::Rect(1440 - 320 - 6, 38, 320, shown), bounds);
    EXPECT_EQ(base::Rect(1440 - 320 - 6, 38, 320, 960 - kStatusBar - kDock - 12),
              center->panelGeometry(Panel::Network, bar.get(), item(293)));
    EXPECT_EQ(base::Rect(0, 0, 320, shown), window->surface()->backgroundBlurRect());
    EXPECT_FALSE(window->surface()->inputRegion());
    EXPECT_EQ(1, services->scans);

    ASSERT_EQ(4u, panel->rows().size());
    EXPECT_TRUE(panel->rows()[0]->isActive());
    EXPECT_FALSE(panel->rows()[1]->isActive());
    clickView(Panel::Network, panel->rows()[1]);
    // A connecting network takes no clicks.
    clickView(Panel::Network, panel->rows()[3]);
    EXPECT_EQ((std::vector<std::string> {"Cafe"}), services->activated);
    EXPECT_TRUE(center->isOpen(Panel::Network));
    // A new password network closes the panel for the agent's prompt.
    clickView(Panel::Network, panel->rows()[2]);
    test::runFor(std::chrono::milliseconds(20));
    EXPECT_FALSE(center->isOpen(Panel::Network));

    // The switch turns Wi-Fi off; the body goes, the panel shrinks over 380 ms.
    center->openPanel(Panel::Network, bar.get(), item(293));
    frame();
    panel = static_cast<shell::NetworkPanel *>(center->statusPanel(Panel::Network));
    const double open = panel->shownBottom();
    clickView(Panel::Network, panel->wirelessSwitch());
    test::runFor(std::chrono::milliseconds(5));
    EXPECT_FALSE(services->wireless);
    test::runFor(std::chrono::milliseconds(100));
    frame();
    EXPECT_LT(panel->shownBottom(), open);
    EXPECT_GT(panel->shownBottom(), panel->contentHeight());
    // The window and its glass shrink with it, frame by frame.
    window = center->popupWindow(Panel::Network);
    const int shrinking = int(std::lround(panel->shownBottom()));
    EXPECT_EQ(shrinking, window->geometry().height());
    EXPECT_EQ(base::Rect(0, 0, 320, shrinking), window->surface()->backgroundBlurRect());
    test::runFor(std::chrono::milliseconds(400));
    frame();
    EXPECT_DOUBLE_EQ(panel->contentHeight(), panel->shownBottom());
    EXPECT_EQ(int(panel->contentHeight()), window->geometry().height());

    // The settings link closes the panel and opens Settings.
    clickView(Panel::Network, panel->settingsLink());
    test::runFor(std::chrono::milliseconds(20));
    EXPECT_FALSE(center->isOpen(Panel::Network));
    EXPECT_EQ((std::vector<std::string> {"-m", "wlan"}), services->settings.back());
}

TEST_F(ControlCenterTest, TheBluetoothPanelListsDevicesAndOwnsItsScanLifetime)
{
    shell::BluetoothDevice headphones;
    headphones.path = "/org/bluez/hci0/dev_AA_BB";
    headphones.name = "Headphones";
    headphones.icon = "audio-headphones";
    headphones.connected = true;
    headphones.paired = true;
    headphones.batteryPercentage = 72;
    shell::BluetoothDevice keyboard;
    keyboard.path = "/org/bluez/hci0/dev_CC_DD";
    keyboard.name = "Keyboard";
    keyboard.icon = "input-keyboard";
    keyboard.paired = true;
    keyboard.busy = true;
    services->bluetoothList = {headphones, keyboard};

    center->openPanel(Panel::Bluetooth, bar.get(), item(293));
    frame();
    auto *panel = static_cast<shell::BluetoothPanel *>(center->statusPanel(Panel::Bluetooth));
    ASSERT_NE(nullptr, panel);
    EXPECT_EQ(1, services->bluetoothScans);
    ASSERT_EQ(2u, panel->rows().size());
    EXPECT_TRUE(panel->rows()[0]->isActive());
    EXPECT_TRUE(panel->rows()[0]->isEnabled());
    EXPECT_FALSE(panel->rows()[1]->isEnabled());
    clickView(Panel::Bluetooth, panel->rows()[0]);
    EXPECT_EQ((std::vector<std::string> {headphones.path}), services->bluetoothToggled);

    center->closePanels();
    EXPECT_EQ(1, services->bluetoothScanStops);
    EXPECT_FALSE(center->isOpen(Panel::Bluetooth));
}

TEST_F(ControlCenterTest, TheSoundAndBatteryPanelsPickOutputsAndModes)
{
    center->openPanel(Panel::Volume, bar.get(), item(333));
    frame();
    auto *volume = static_cast<shell::VolumePanel *>(center->statusPanel(Panel::Volume));
    ASSERT_EQ(2u, volume->rows().size());
    EXPECT_TRUE(volume->rows()[0]->isActive());
    clickView(Panel::Volume, volume->rows()[0]);
    EXPECT_EQ(-1, services->defaultRequested);
    clickView(Panel::Volume, volume->rows()[1]);
    EXPECT_EQ(2, services->defaultRequested);

    // Opening another panel is the bar's: it closes this one first.
    center->closePanels();
    test::runFor(std::chrono::milliseconds(5));
    center->openPanel(Panel::Battery, bar.get(), item(247));
    frame();
    EXPECT_FALSE(center->isOpen(Panel::Volume));
    auto *battery = static_cast<shell::BatteryPanel *>(center->statusPanel(Panel::Battery));
    ASSERT_EQ(3u, battery->rows().size());
    EXPECT_TRUE(battery->rows()[0]->isVisible());
    EXPECT_TRUE(battery->rows()[1]->isActive());
    EXPECT_FALSE(battery->rows()[2]->isVisible());
    clickView(Panel::Battery, battery->rows()[0]);
    test::runFor(std::chrono::milliseconds(5));
    EXPECT_EQ("power-saver", services->profile);
    EXPECT_TRUE(battery->rows()[0]->isActive());
}

TEST_F(ControlCenterTest, LanguageChangeKeepsTheOpenPanels)
{
    center->openPanel(Panel::ControlCenter, bar.get(), item(178));
    frame();
    shell::ControlCenterView *view = center->controlCenterView();
    ASSERT_NE(nullptr, view);
    const int height = center->popupWindow(Panel::ControlCenter)->drawnBounds().height();
    ui::Application::instance()->languageChanged.emit();
    test::runFor(std::chrono::milliseconds(5));
    frame();
    EXPECT_TRUE(center->isOpen(Panel::ControlCenter));
    EXPECT_EQ(view, center->controlCenterView());
    EXPECT_EQ(height, center->popupWindow(Panel::ControlCenter)->drawnBounds().height());
    EXPECT_TRUE(view->wirelessCard()->isChecked());
    center->closePanels();
    test::runFor(std::chrono::milliseconds(5));

    center->openPanel(Panel::Battery, bar.get(), item(247));
    frame();
    auto *battery = static_cast<shell::BatteryPanel *>(center->statusPanel(Panel::Battery));
    ASSERT_NE(nullptr, battery);
    ui::Application::instance()->languageChanged.emit();
    test::runFor(std::chrono::milliseconds(5));
    frame();
    EXPECT_TRUE(center->isOpen(Panel::Battery));
    EXPECT_EQ(battery, center->statusPanel(Panel::Battery));
    ASSERT_EQ(3u, battery->rows().size());
    EXPECT_TRUE(battery->rows()[1]->isActive());
    EXPECT_FALSE(battery->rows()[2]->isVisible());
}

TEST_F(ControlCenterTest, TheBarIndicatorsFollowTheServices)
{
    int changes = 0;
    base::Subscription subscription = center->StatusBarIndicators::changed.connect([&] { ++changes; });
    EXPECT_TRUE(center->audio().hasSink);
    EXPECT_NEAR(0.36, center->audio().volume, 1e-9);
    EXPECT_EQ("Office 5G", center->network().wirelessName);
    services->wireless = false;
    services->changed();
    test::runFor(std::chrono::milliseconds(5));
    EXPECT_EQ(1, changes);
    EXPECT_FALSE(center->network().wirelessEnabled);
    services->changed();
    test::runFor(std::chrono::milliseconds(5));
    EXPECT_EQ(1, changes);
}

TEST_F(ControlCenterTest, ThePowerDialogConfirmsWithReturnAndCancelsWithEscape)
{
    shell::PowerDialog dialog(shell.get());
    int accepted = 0;
    int rejected = 0;
    dialog.onAccepted = [&] { ++accepted; };
    dialog.onRejected = [&] { ++rejected; };
    dialog.open(shell::PowerDialog::Action::Logout);
    frame();
    ASSERT_TRUE(dialog.isOpen());
    wm::Window *window = dialog.window()->wmWindow();
    ASSERT_NE(nullptr, window);
    EXPECT_EQ(wm::WindowRole::Normal, window->role());
    EXPECT_TRUE(window->isAlwaysOnTop());
    EXPECT_EQ(336, window->bounds().width());
    // Asking again changes what it asks.
    dialog.open(shell::PowerDialog::Action::Reboot);
    EXPECT_EQ(shell::PowerDialog::Action::Reboot, dialog.action());
    key(KEY_ENTER);
    test::runFor(std::chrono::milliseconds(5));
    EXPECT_EQ(1, accepted);
    EXPECT_FALSE(dialog.isOpen());

    dialog.open(shell::PowerDialog::Action::Shutdown);
    frame();
    key(KEY_ESC);
    test::runFor(std::chrono::milliseconds(5));
    EXPECT_EQ(1, rejected);
    EXPECT_FALSE(dialog.isOpen());
}

TEST_F(ControlCenterTest, TheDeviceUsersTileShowsWhoUsesTheCamera)
{
    center->openPanel(Panel::ControlCenter, bar.get(), item(178));
    frame();
    shell::ControlCenterView *view = center->controlCenterView();
    const int before = center->popupWindow(Panel::ControlCenter)->drawnBounds().height();
    const ui::RectF cards = view->wirelessCard()->geometry();
    services->users = {{"Camera", "org.example.Camera", true, true, false}};
    services->changed();
    test::runFor(std::chrono::milliseconds(5));
    frame();
    // One row of 20 with 16 round it, and the gap below.
    const int after = center->popupWindow(Panel::ControlCenter)->drawnBounds().height();
    EXPECT_NEAR(before + 20 + 32 + 10, after, 1);
    EXPECT_NEAR(cards.y + 62, view->wirelessCard()->geometry().y, 0.01);
    EXPECT_EQ("is using the camera and microphone", shell::deviceUsageText(services->users.front()));
}

// The real bar opening the panels with clicks on its items, as live.
TEST_F(ControlCenterTest, TheBarsItemsOpenEveryPanel)
{
    // The fixture's stand-in bar gives way to the real one.
    bar->hide();
    directory.write("shell.conf", "");
    auto backend = std::make_unique<shell::StatusBar>(base::dbus::Connection::session(core::mainLoop()),
                                                      base::dbus::Connection::system(core::mainLoop()), &shell->tasks(),
                                                      directory.filePath("shell.conf"));
    auto *statusBar = shell->addComponent(std::make_unique<shell::StatusBarWindow>(shell.get(), std::move(backend)));
    statusBar->addPanels(center);
    statusBar->setIndicators(center);
    frame();
    for (const auto &[panel, icon] : {std::pair {Panel::Volume, statusBar->volumeIcon()},
                                      std::pair {Panel::Network, statusBar->networkIcon()},
                                      std::pair {Panel::ControlCenter, statusBar->controlIcon()}}) {
        ASSERT_TRUE(icon->isVisible());
        const ui::PointF origin = icon->mapToWindow({0, 0});
        click(origin.x + icon->geometry().width / 2, 16);
        test::runFor(std::chrono::milliseconds(50));
        frame();
        EXPECT_TRUE(center->isOpen(panel)) << int(panel);
        ASSERT_NE(nullptr, center->popupWindow(panel)) << int(panel);
        EXPECT_NE(nullptr, center->popupWindow(panel)->wmWindow()) << int(panel);
        EXPECT_TRUE(icon->isChecked());
    }
    statusBar->removePanels(center);
    statusBar->setIndicators(nullptr);
}

TEST_F(ControlCenterTest, EfficientTaskbarOpensControlCenterAboveItsBottomEdge)
{
    bar->setGeometry(base::Rect(900, kOutput.height() - 52, kOutput.width() - 900, 52));
    bar->setExclusiveZone(wm::ScreenEdge::Bottom, 52);
    center->openPanel(Panel::ControlCenter, bar.get(), {100, 0, 40, 52});
    test::runFor(std::chrono::milliseconds(50)); frame();
    ASSERT_NE(nullptr, center->popupWindow(Panel::ControlCenter));
    const auto geometry = center->popupWindow(Panel::ControlCenter)->geometry();
    EXPECT_LE(geometry.bottom(), bar->geometry().y());
    EXPECT_GE(geometry.y(), kOutput.y());
}
