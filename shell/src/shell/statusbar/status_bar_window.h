// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/geometry.h"
#include "base/signal.h"
#include "render/glass.h"
#include "shell/shell.h"
#include "shell/statusbar/bar_item.h"
#include "shell/statusbar/status_bar_menu_bar.h"
#include "shell/statusbar/status_bar_panels.h"

namespace Lingmo::ui {
class Menu;
}

namespace Lingmo::Shell {

class ClockItem;
class DockSettings;
class DeviceUsage;
class VpnNotifier;
class ShellWindow;
class StatusBar;
class StatusBarBackdrop;
class StatusIcon;
class StatusItemsView;
class StopItem;
class SystemTrayView;
class Wallpaper;

// The status bar: a 32 px strip along the top of the primary output, reserved
// from the work area. Its background is the user's Gaussian blur from
// shell.conf (status_bar_settings.h), so what shows through is softened rather
// than hidden; with no blur it is clear. Left to right: the menu bar, the stop
// button of an agent's control, the items applications
// draw, the tray, the volume, Wi-Fi, battery and control centre icons, and the
// clock. Its contents are light or dark by what the wallpaper shows behind it.
class StatusBarWindow : public Component, public BarContext
{
public:
    static constexpr int kHeight = 32;

    // `statusBar` is the backend: tray, registrar, battery, power actions,
    // the clock's format and org.lingmo.Statusbar.
    StatusBarWindow(Shell *shell, std::unique_ptr<StatusBar> statusBar);
    ~StatusBarWindow() override;

    void followDesktopMode(DockSettings &settings);
    bool efficientMode() const { return m_efficientMode; }
    int taskbarWidth() const;
    base::Signal<int> taskbarWidthChanged;
    StatusBar &statusBar() { return *m_statusBar; }
    StatusBarBackdrop &backdrop() { return *m_backdrop; }
    // What the desktop shows decides the bar's contents; null leaves them to
    // the theme.
    void setWallpaper(Wallpaper *wallpaper);
    // Which applications use a camera, a microphone or record the system's
    // audio: the control icon's dot, and the control centre's tile.
    DeviceUsage *deviceUsage() const { return m_deviceUsage.get(); }
    // Tests give their own; the session's is made with the bar on a bus.
    void setDeviceUsage(std::unique_ptr<DeviceUsage> usage);
    // Tells a VPN that went down on its own (NetworkManager's
    // Handler::vpnFailed, from the control centre's client).
    VpnNotifier &vpnNotifier() { return *m_vpnNotifier; }
    ShellWindow *window() const { return m_window.get(); }

    // --- The menu bar, the panels and the indicators of other components ---
    // The menu bar goes in the first slot; null takes it out (before its
    // component goes).
    StatusBarMenuBarHost menuBarHost();
    void setMenuBar(std::unique_ptr<StatusBarMenuBar> menuBar);

    void addPanels(StatusBarPanels *panels);
    void removePanels(StatusBarPanels *panels);
    void setIndicators(StatusBarIndicators *indicators);
    // An item's click: every popup closes, then the panel opens unless it
    // was the one open.
    void togglePanel(StatusBarPanels::Panel panel);
    // Closes the panels and every other popup of the bar.
    void closePanels();
    void closePopups();

    // The stop buttons, shown while a recording or an agent's control runs.
    void setRecording(bool recording);
    std::function<void()> onStopRecording;
    void setAgentControl(bool active);
    std::function<void()> onStopAgentControl;

    // --- For tests and the reference render ---
    SystemTrayView *tray() const { return m_tray; }
    StatusItemsView *statusItems() const { return m_statusItems; }
    StatusIcon *volumeIcon() const { return m_volume; }
    StatusIcon *networkIcon() const { return m_network; }
    StatusIcon *batteryIcon() const { return m_battery; }
    StatusIcon *controlIcon() const { return m_control; }
    StatusIcon *notificationIcon() const { return m_notification; }
    ClockItem *clock() const { return m_clock; }
    // The bar's contents are light.
    bool lightContents() const override;
    bool lightContentsAt(const ui::RectF& windowRect) const override;
    // One of the panels is open.
    bool panelOpen() const;
    void platePainted() override;
    // Reads the clock now.
    void updateClock();

private:
    class Root;

    void updateGeometry();
    void updateLook();
    void updateIndicators();
    void updateBattery();
    void layoutRow();
    void armClock();
    StatusBarPanels *panelsFor(StatusBarPanels::Panel panel) const;
    BarItem *itemFor(StatusBarPanels::Panel panel) const;
    void updateChecked();
    // Puts the bar's blur, or the plate's when the bar has none, behind the
    // contents.
    void updateGlass();
    void updatePlateBlur();
    void updateStatusItemLimits();
    ui::RectF availableGeometry() const;

    bool m_efficientMode = false;
    DockSettings *m_dockSettings = nullptr;
    bool m_geometryUpdating = false;
    base::Subscription m_modeChanged;
    Shell *m_shell;
    Wallpaper *m_wallpaper = nullptr;
    std::unique_ptr<StatusBar> m_statusBar;
    std::unique_ptr<StatusBarBackdrop> m_backdrop;
    std::unique_ptr<DeviceUsage> m_deviceUsage;
    std::unique_ptr<VpnNotifier> m_vpnNotifier;
    base::Subscription m_deviceUsageChanged;
    std::unique_ptr<ShellWindow> m_window;
    Root *m_root = nullptr;
    // The slot's placeholder, or the menu bar.
    ui::View *m_menuSlot = nullptr;
    StatusBarMenuBar *m_menuBar = nullptr;
    StopItem *m_agentControl = nullptr;
    StatusItemsView *m_statusItems = nullptr;
    SystemTrayView *m_tray = nullptr;
    StatusIcon *m_volume = nullptr;
    StatusIcon *m_network = nullptr;
    StatusIcon *m_battery = nullptr;
    StatusIcon *m_control = nullptr;
    ClockItem *m_clock = nullptr;
    StatusIcon *m_notification = nullptr;
    StatusIcon *m_search = nullptr;
    std::vector<StatusBarPanels *> m_panels;
    std::vector<base::Subscription> m_panelSubscriptions;
    StatusBarIndicators *m_indicators = nullptr;
    base::Subscription m_indicatorsChanged;
    // A wall-clock timerfd: it fires on the minute (or second) the clock
    // shows next, after a suspend, and whenever the system time is set.
    int m_clockFd = -1;
    int m_clockWatch = 0;
    bool m_lightContents = false;
    bool m_recordingActive = false;
    // The plate the blur is behind, in window coordinates; empty for none.
    ui::RectF m_blurPlate;
    // The bar's own blur: applied while shell.conf gives it some, and what was
    // applied, so a repaint does not commit the surface again.
    bool m_barBlurApplied = false;
    base::Rect m_barBlurRect;
    render::GlassParams m_barBlurGlass;
    std::vector<base::Subscription> m_subscriptions;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace Lingmo::Shell

