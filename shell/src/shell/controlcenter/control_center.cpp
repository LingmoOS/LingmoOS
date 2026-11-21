// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/controlcenter/control_center.h"

#include <algorithm>
#include <cmath>

#include "base/event_loop.h"
#include "core/main_loop.h"
#include "shell/controlcenter/control_center_view.h"
#include "shell/controlcenter/panel_style.h"
#include "shell/controlcenter/power_dialog.h"
#include "shell/controlcenter/session_services.h"
#include "shell/controlcenter/status_panel.h"
#include "shell/shell_host.h"
#include "shell/shell_window.h"
#include "shell/statusbar/power_actions.h"
#include "shell/statusbar/status_bar.h"
#include "ui/application.h"
#include "ui/embedded_window.h"
#include "ui/keys.h"
#include "ui/theme.h"
#include "ui/window.h"

namespace Lingmo::Shell {

// The status bar's height: the control centre hangs this far below the
// bar's screen's top, whatever the item.
static constexpr int kBarHeight = 32;

// A popup's content: the panel's view filling it. A theme change lays every
// window out again, which is where the glass's tint follows dark mode.
class ControlCenter::PopupRoot final : public ui::View
{
public:
    explicit PopupRoot(std::unique_ptr<ui::View> content)
    {
        m_content = content.get();
        addChild(std::move(content));
    }

    std::function<void()> onLayout;
    std::function<void()> onEscape;

protected:
    void layout() override
    {
        m_content->setGeometry(localRect());
        if (onLayout)
            onLayout();
    }

    bool keyPress(const ui::KeyEvent &event) override
    {
        if (event.key != ui::key::Escape || !onEscape)
            return false;
        onEscape();
        return true;
    }

private:
    ui::View *m_content;
};

ControlCenter::ControlCenter(Shell *shell, std::unique_ptr<ControlCenterServices> services,
                             PowerActions *powerActions)
    : m_shell(shell)
    , m_services(std::move(services))
    , m_powerActions(powerActions)
{
    m_subscriptions.push_back(m_services->changed.connect([this] { refresh(); }));
    m_subscriptions.push_back(m_services->vpnFailed.connect(
            [this](const std::string &name, const std::string &message) { vpnFailed.emit(name, message); }));
    if (m_powerActions) {
        m_subscriptions.push_back(m_powerActions->pendingChanged.connect([this] { updatePower(); }));
        m_subscriptions.push_back(m_powerActions->requested.connect([this] { updatePower(); }));
    }
    // StatusPanel's adjustPosition on a work area change: the panel keeps
    // its place and takes the new height. Only the status panels follow it.
    m_subscriptions.push_back(m_shell->outputsChanged.connect([this] {
        for (Panel panel : {Panel::Network, Panel::Bluetooth, Panel::Volume, Panel::Battery})
            fitToWorkArea(panel);
    }));
    m_subscriptions.push_back(ui::Application::instance()->languageChanged.connect([this] { retranslate(); }));
    m_audio = audio();
    m_network = network();
}

ControlCenter::~ControlCenter()
{
    *m_alive = false;
    m_subscriptions.clear();
    for (Open &open : m_open)
        open.window.reset();
    m_powerDialog.reset();
}

std::unique_ptr<ControlCenter> ControlCenter::create(Shell *shell, StatusBar &statusBar,
                                                     NotificationSettings *notifications)
{
    return std::make_unique<ControlCenter>(shell, createSessionServices(shell, statusBar, notifications),
                                           &statusBar.powerActions());
}

size_t ControlCenter::index(Panel panel)
{
    switch (panel) {
    case Panel::ControlCenter:
        return 0;
    case Panel::Network:
        return 1;
    case Panel::Bluetooth:
        return 2;
    case Panel::Volume:
        return 3;
    case Panel::Battery:
        return 4;
    case Panel::NotificationCenter:
        break;
    }
    return 0;
}

bool ControlCenter::hasPanel(Panel panel) const
{
    return panel != Panel::NotificationCenter;
}

bool ControlCenter::isOpen(Panel panel) const
{
    if (!hasPanel(panel))
        return false;
    const Open &open = m_open[index(panel)];
    return open.window && !open.window->isHidden();
}

ControlCenterView *ControlCenter::controlCenterView() const
{
    return m_open[0].controlCenter;
}

StatusPanelView *ControlCenter::statusPanel(Panel panel) const
{
    return hasPanel(panel) ? m_open[index(panel)].panel : nullptr;
}

ui::Window *ControlCenter::popup(Panel panel) const
{
    return hasPanel(panel) ? m_open[index(panel)].window.get() : nullptr;
}

ShellWindow *ControlCenter::popupWindow(Panel panel) const
{
    ui::Window *window = popup(panel);
    return window ? m_shell->host()->windowFor(static_cast<ui::EmbeddedWindow *>(window)) : nullptr;
}

base::Rect ControlCenter::panelGeometry(Panel panel, ShellWindow *bar, const base::Rect &anchor) const
{
    const base::Point barOrigin = bar ? bar->geometry().topLeft() : base::Point();
    // The left edge of the item's plate, at the bar's bottom.
    const base::Point position(barOrigin.x() + anchor.x() + 3, barOrigin.y() + anchor.y() + anchor.height());
    const Shell::Output *output = m_shell->output(bar ? bar->outputName() : std::string());
    const base::Rect screen = output ? output->bounds : base::Rect(0, 0, 1440, 960);
    const int margin = int(::Lingmo::Shell::Panel::kPopupMargin);

    if (panel == Panel::ControlCenter) {
        const int width = int(ControlCenterView::kWidth);
        const double scale = bar ? bar->scale() : 1;
        const int height = m_open[0].controlCenter ? int(m_open[0].controlCenter->windowHeight(scale)) : 0;
        int x = position.x();
        if (x + width >= screen.x() + screen.width())
            x = screen.x() + screen.width() - width - margin;
        x = std::max(x, screen.x() + margin);
        // The bar sits at the top of its own screen, which need not be at y 0.
        const bool bottomBar = bar && bar->geometry().y() > screen.y() + screen.height() / 2;
        const int y = bottomBar ? std::max(screen.y() + margin, bar->geometry().y() - height - margin)
                                : screen.y() + kBarHeight + margin;
        return base::Rect(x, y, width, height);
    }

    const base::Rect area = output ? m_shell->workArea(*output) : screen;
    const int width = int(status_panel::kWidth);
    // As tall as the work area allows the panel to grow; the window is as
    // tall as the panel is drawn.
    const int height = std::max(0, area.height() - margin * 2);
    const int x = std::max(area.x() + margin, std::min(position.x(), area.x() + area.width() - width - margin));
    const int y = std::max(area.y() + margin, std::min(position.y() + margin, area.y() + area.height() - height - margin));
    return base::Rect(x, y, width, height);
}

void ControlCenter::openPanel(Panel panel, ShellWindow *bar, const base::Rect &anchor)
{
    if (!hasPanel(panel) || !bar || !bar->window())
        return;
    Open &open = m_open[index(panel)];
    if (open.window)
        close(panel);

    std::unique_ptr<ui::View> content;
    switch (panel) {
    case Panel::ControlCenter: {
        auto view = std::make_unique<ControlCenterView>(*m_services);
        open.controlCenter = view.get();
        view->onHideRequested = [this] { close(Panel::ControlCenter); };
        view->onWirelessDetailsRequested = [this] {
            Open &control = m_open[index(Panel::ControlCenter)];
            ShellWindow *bar = control.bar;
            const base::Rect anchor = control.anchor;
            close(Panel::ControlCenter);
            openPanel(Panel::Network, bar, anchor);
        };
        view->onBluetoothDetailsRequested = [this] {
            Open &control = m_open[index(Panel::ControlCenter)];
            ShellWindow *bar = control.bar;
            const base::Rect anchor = control.anchor;
            close(Panel::ControlCenter);
            openPanel(Panel::Bluetooth, bar, anchor);
        };
        view->onHeightChanged = [this] { resizeControlCenter(); };
        content = std::move(view);
        break;
    }
    case Panel::Network:
        open.panel = new NetworkPanel(*m_services);
        break;
    case Panel::Bluetooth:
        open.panel = new BluetoothPanel(*m_services);
        break;
    case Panel::Volume:
        open.panel = new VolumePanel(*m_services);
        break;
    case Panel::Battery:
        open.panel = new BatteryPanel(*m_services);
        break;
    case Panel::NotificationCenter:
        return;
    }
    if (open.panel) {
        content.reset(open.panel);
        open.panel->onCloseRequested = [this, panel] { close(panel); };
        open.panel->onShownBottomChanged = [this, panel] { updateWindow(panel); };
    }

    open.bar = bar;
    open.anchor = anchor;
    open.geometry = panelGeometry(panel, bar, anchor);
    if (open.panel)
        open.panel->setMaximumHeight(open.geometry.height());
    const base::Point origin = bar->geometry().topLeft();
    const bool bottomBar = bar->geometry().y() > m_shell->output(bar->outputName())->bounds.y()
            + m_shell->output(bar->outputName())->bounds.height() / 2;
    ui::PopupPlacement placement;
    placement.anchorRect = {double(open.geometry.x() - origin.x()), double(open.geometry.y() - origin.y()), 1, 1};
    placement.anchor = ui::PopupAnchor::TopLeft;
    placement.gravity = bottomBar ? ui::PopupAnchor::TopRight : ui::PopupAnchor::BottomRight;
    placement.flipX = false;
    placement.flipY = false;
    placement.slide = false;
    open.window = ui::Window::createPopup(bar->window(), placement, true);
    auto root = std::make_unique<PopupRoot>(std::move(content));
    open.root = root.get();
    open.root->onEscape = [this, panel] { close(panel); };
    open.root->onLayout = [this, panel] {
        Open &shown = m_open[index(panel)];
        if (shown.dark != ui::Theme::current().darkMode)
            updateWindow(panel);
    };
    open.window->setContentView(std::move(root));
    open.window->addShortcut(ui::NoModifier, ui::key::Escape, [this, panel] {
        const std::shared_ptr<bool> alive = m_alive;
        core::mainLoop()->post([this, alive, panel] {
            if (*alive)
                close(panel);
        });
    });
    // A press outside the bar and the panel, or the bar closing its popups.
    open.window->onCloseRequested = [this, panel] {
        const std::shared_ptr<bool> alive = m_alive;
        core::mainLoop()->post([this, alive, panel] {
            if (*alive)
                close(panel);
        });
    };
    if (panel == Panel::ControlCenter) {
        open.window->setScale(bar->scale());
        open.geometry.setHeight(int(open.controlCenter->windowHeight(bar->scale())));
    }
    // A status panel's window takes its height from the panel once shown.
    open.window->resize({double(open.geometry.width()),
                         open.panel ? std::max(1.0, open.panel->contentHeight()) : double(open.geometry.height())});
    open.window->show();
    if (open.panel) {
        // Measured at the output's scale now: no open animation.
        open.panel->setShown(false);
        open.panel->setShown(true);
        open.panel->opened();
    }
    updateWindow(panel);
    panelsChanged();
}

void ControlCenter::updateWindow(Panel panel)
{
    Open &open = m_open[index(panel)];
    ShellWindow *window = popupWindow(panel);
    if (!window)
        return;
    open.dark = ui::Theme::current().darkMode;
    int height = open.geometry.height();
    if (open.panel) {
        // The window follows the panel's height as it animates, and the
        // compositor's outline and shadow follow the window.
        height = std::max(1, int(std::lround(open.panel->shownBottom())));
        open.window->resize({double(open.geometry.width()), double(height)});
    }
    window->setBackgroundBlur(base::Rect(0, 0, open.geometry.width(), height), int(::Lingmo::Shell::Panel::kTileRadius),
                              ::Lingmo::Shell::Panel::panelGlass());
}

void ControlCenter::fitToWorkArea(Panel panel)
{
    Open &open = m_open[index(panel)];
    if (!open.window || !open.panel)
        return;
    const base::Rect geometry = panelGeometry(panel, open.bar, open.anchor);
    if (geometry.height() == open.geometry.height())
        return;
    open.geometry.setHeight(geometry.height());
    open.panel->setMaximumHeight(geometry.height());
    updateWindow(panel);
}

void ControlCenter::resizeControlCenter()
{
    Open &open = m_open[0];
    if (!open.window || !open.controlCenter)
        return;
    const double scale = open.bar ? open.bar->scale() : 1;
    const int height = int(open.controlCenter->windowHeight(scale));
    if (height == open.geometry.height())
        return;
    open.geometry = panelGeometry(Panel::ControlCenter, open.bar, open.anchor);
    open.window->resize({double(open.geometry.width()), double(height)});
    const auto origin = open.bar->geometry().topLeft();
    ui::PopupPlacement placement;
    placement.anchorRect = {double(open.geometry.x() - origin.x()), double(open.geometry.y() - origin.y()), 1, 1};
    placement.anchor = ui::PopupAnchor::TopLeft;
    placement.gravity = ui::PopupAnchor::BottomRight;
    placement.flipX = placement.flipY = placement.slide = false;
    open.window->setPopupPlacement(placement);
    updateWindow(Panel::ControlCenter);
}

void ControlCenter::close(Panel panel)
{
    Open &open = m_open[index(panel)];
    if (!open.window)
        return;
    if (open.panel) {
        open.panel->onShownBottomChanged = nullptr;
        open.panel->onCloseRequested = nullptr;
        open.panel->closed();
        if (auto *network = dynamic_cast<NetworkPanel *>(open.panel))
            network->setShownForScans(false);
    }
    if (open.controlCenter) {
        open.controlCenter->onHideRequested = nullptr;
        open.controlCenter->onHeightChanged = nullptr;
        open.controlCenter->onWirelessDetailsRequested = nullptr;
        open.controlCenter->onBluetoothDetailsRequested = nullptr;
    }
    std::shared_ptr<ui::Window> doomed(std::move(open.window));
    doomed->hide();
    open = Open();
    // Not from inside its own event handling.
    core::mainLoop()->post([doomed] {});
    panelsChanged();
}

// The open panels and the power dialog stay as they are, with their text
// taken again.
void ControlCenter::retranslate()
{
    if (m_open[0].controlCenter)
        m_open[0].controlCenter->retranslate();
    for (size_t i = 1; i < m_open.size(); ++i) {
        if (m_open[i].panel)
            m_open[i].panel->retranslate();
    }
    if (m_powerDialog)
        m_powerDialog->retranslate();
}

void ControlCenter::closePanels()
{
    for (Panel panel : {Panel::ControlCenter, Panel::Network, Panel::Bluetooth, Panel::Volume, Panel::Battery})
        close(panel);
}

void ControlCenter::refresh()
{
    if (m_open[0].controlCenter)
        m_open[0].controlCenter->refresh();
    for (size_t i = 1; i < m_open.size(); ++i) {
        if (m_open[i].panel)
            m_open[i].panel->refresh();
    }
    const Audio audio = this->audio();
    const Network network = this->network();
    if (audio != m_audio || network != m_network) {
        m_audio = audio;
        m_network = network;
        changed();
    }
}

StatusBarIndicators::Audio ControlCenter::audio() const
{
    Audio audio;
    audio.hasSink = m_services->hasSink();
    audio.muted = m_services->muted();
    audio.volume = m_services->volume();
    return audio;
}

StatusBarIndicators::Network ControlCenter::network() const
{
    Network network;
    network.wirelessHwEnabled = m_services->wirelessHardwareEnabled();
    network.wirelessEnabled = m_services->wirelessEnabled();
    network.hotspotActive = m_services->hotspotActive();
    network.wirelessName = m_services->wirelessName();
    network.wirelessIcon = m_services->wirelessIcon();
    return network;
}

void ControlCenter::setDeviceUsersSource(std::function<std::vector<DeviceUser>()> source)
{
    m_services->deviceUsersSource = std::move(source);
    refresh();
}

void ControlCenter::deviceUsersChanged()
{
    refresh();
}

void ControlCenter::updatePower()
{
    if (!m_powerActions)
        return;
    const PowerActions::Action pending = m_powerActions->pending();
    if (pending == PowerActions::None) {
        if (m_powerDialog)
            m_powerDialog->close();
        return;
    }
    if (!m_powerDialog) {
        m_powerDialog = std::make_unique<PowerDialog>(m_shell);
        m_powerDialog->onAccepted = [this] { m_powerActions->confirm(); };
        m_powerDialog->onRejected = [this] { m_powerActions->cancel(); };
    }
    m_powerDialog->open(pending == PowerActions::Reboot     ? PowerDialog::Action::Reboot
                        : pending == PowerActions::Shutdown ? PowerDialog::Action::Shutdown
                                                            : PowerDialog::Action::Logout);
}

} // namespace Lingmo::Shell
