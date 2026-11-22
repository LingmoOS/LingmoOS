// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/launcher/launcher_window.h"

#include <algorithm>
#include <cmath>

#include "core/main_loop.h"
#include "shell/dock/dock.h"
#include "shell/launcher/launcher.h"
#include "shell/launcher/launcher_view.h"
#include "shell/launcher/launcher_search_field.h"
#include "shell/launcher/launcher_wallpaper.h"
#include "shell/shell_data.h"
#include "shell/shell_window.h"
#include "ui/embedded_window.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "wm/window.h"
#include "scene/window_animation.h"
#include "wm/overview/overview_controller.h"
#include "wm/show_desktop_controller.h"
#include "wm/window_manager.h"

namespace Lingmo::Shell {

LauncherWindow::LauncherWindow(Shell *shell, std::unique_ptr<Launcher> launcher, Dock *dock,
                               Wallpaper *wallpaper)
    : m_shell(shell)
    , m_launcher(std::move(launcher))
    , m_dock(dock)
    , m_wallpaper(std::make_unique<LauncherWallpaper>(wallpaper))
{
    m_window = std::make_unique<ShellWindow>(shell, wm::WindowRole::Launcher);
    auto view = std::make_unique<LauncherView>(*m_launcher, *m_wallpaper, shell->badges());
    m_view = view.get();
    m_view->folderBlurChanged = [this](ui::RectF panel, bool active) {
        // The folder is rendered by LauncherView on this surface.  Keep the
        // blur on that same surface so the desktop remains visible through
        // the transparent card instead of relying on a full-screen popup.
        if (!active || (m_dock && m_dock->settings().efficientMode())) {
            m_window->window()->setBackgroundBlur({}, 0);
            return;
        }
        m_window->window()->setBackgroundBlur(panel, 18);
    };
    m_window->setContentView(std::move(view));
    // Transparent: until the wallpaper decodes, the dimmer darkens what is
    // behind it.
    m_window->window()->setBackgroundColor(ui::Color {0, 0, 0, 0});
    m_window->window()->onActiveChanged = [this] { activeChanged(); };
    m_window->onCloseRequested = [this] { m_launcher->hide(); };

    m_launcher->showWindow = [this] { showWindow(); };
    m_launcher->hideWindow = [this] { hideWindow(); };
    m_launcher->focusSearchWindow = [this] { m_view->searchField()->focusField(); };
    LauncherModel &model = m_launcher->applications();
    // Applications opened before the view existed are no longer new.
    model.clearNewInstalled(m_launcher->takeOpenedApplications());
    m_subscriptions.push_back(model.applicationLaunched.connect([this] { m_launcher->hide(); }));
    m_subscriptions.push_back(m_view->dismissRequested.connect([this] { m_launcher->hide(); }));
    m_subscriptions.push_back(m_shell->outputsChanged.connect([this] { updateGeometry(); }));
    if (m_dock)
        m_subscriptions.push_back(m_dock->primaryGeometryChanged.connect([this] { updateGeometry(); }));

    // Laid out and decoded while hidden, so the first show has its pages.
    updateGeometry();
}

LauncherWindow::~LauncherWindow()
{
    *m_alive = false;
    m_gestureTimer.stop();
    m_subscriptions.clear();
    m_launcher->showWindow = nullptr;
    m_launcher->hideWindow = nullptr;
    m_launcher->focusSearchWindow = nullptr;
    m_view->folderBlurChanged = nullptr;
    m_window.reset();
    m_wallpaper.reset();
    m_launcher.reset();
}

void LauncherWindow::updateGeometry()
{
    const Shell::Output *output = m_shell->output({});
    if (!output)
        return;
    const bool compact = m_dock && m_dock->settings().efficientMode();
    m_window->setOutput(output->name);
    if (compact) {
        const int width = std::min(760, std::max(520, output->bounds.width() * 3 / 5));
        const int height = std::min(720, std::max(520, output->bounds.height() * 4 / 5));
        m_window->setGeometry(base::Rect(output->bounds.right() - width,
                                         output->bounds.bottom() - DockSettings::taskbarHeight - height,
                                         width, height));
        m_window->setBackgroundBlur(base::Rect(0, 0, width, height), 24,
                                    popupGlass(ui::Theme::current().darkMode ? .13 : .94,
                                               ui::Theme::current().darkMode ? .13 : .94,
                                               ui::Theme::current().darkMode ? .14 : .95,
                                               .86));
    } else {
        // The normal launcher covers the output, including the top bar.
        m_window->setGeometry(output->bounds);
        m_window->setBackgroundBlur(base::Rect(), 0);
    }
    m_view->setCompact(compact);
    // The wallpaper is decoded at the device pixel size.
    m_wallpaper->setSurfaceSize(base::Size(int(std::lround(output->bounds.width() * output->scale)),
                                           int(std::lround(output->bounds.height() * output->scale))));

    // Room for the dock on its edge, and a constant 8 at the top.
    LauncherMetrics::Margins margins;
    const base::Rect dock = m_dock && m_dock->primaryGeometry ? m_dock->primaryGeometry() : base::Rect();
    if (!compact && !dock.isEmpty()) {
        switch (m_dock->settings().direction()) {
        case DockSettings::Left:
            margins.left = dock.width();
            break;
        case DockSettings::Right:
            margins.right = dock.width();
            break;
        case DockSettings::Bottom:
            margins.bottom = dock.height();
            break;
        }
    }
    m_view->setMargins(margins);
    m_view->preloadIcons();

    base::Rect available = m_shell->workArea(*output);
    available.translate(-output->bounds.x(), -output->bounds.y());
    m_view->setAvailableGeometry(ui::RectF {double(available.x()), double(available.y()),
                                            double(available.width()), double(available.height())});
}

bool LauncherWindow::beginGesture(bool opening)
{
    if (m_gestureActive || m_launcher->isShown() == opening || m_shell->windowManager()->overview()->isActive()) return false;
    if (opening) m_shell->windowManager()->showDesktop()->exitImmediately();
    m_gestureTimer.stop();
    m_gestureActive = true; m_gestureOpening = opening;
    m_gestureProgress = opening ? 0 : 1;
    // Opening is rendered by gesture progress, not a second timed animation.
    m_view->setOpacity(m_gestureProgress);
    m_view->setEnabled(false);
    if (opening) m_launcher->show();
    if (auto *animation = dynamic_cast<scene::WindowAnimation *>(m_window->wmWindow() ? m_window->wmWindow()->node() : nullptr)) {
        animation->finish(); animation->setStyle(scene::WindowAnimation::Style::None);
    }
    presentGesture();
    return true;
}

void LauncherWindow::updateGesture(double progress)
{
    if (!m_gestureActive || !std::isfinite(progress)) return;
    m_gestureProgress = std::clamp(m_gestureOpening ? progress : 1 - progress, 0.0, 1.0);
    presentGesture();
}

void LauncherWindow::presentGesture()
{
    const double scale = 1.08 - 0.08 * m_gestureProgress;
    const auto rect = m_view->geometry();
    m_view->setTransform({{rect.width * (1 - scale) / 2, rect.height * (1 - scale) / 2}, scale});
    m_view->setOpacity(m_gestureProgress);
}

void LauncherWindow::endGesture(bool cancelled)
{
    if (!m_gestureActive) return;
    const bool shown = m_gestureOpening ? m_gestureProgress >= 0.5 : m_gestureProgress > 0.5;
    m_gestureTarget = cancelled ? (m_gestureOpening ? 0 : 1) : (shown ? 1 : 0);
    m_gestureFrom = m_gestureProgress;
    m_gestureTime = std::chrono::steady_clock::now();
    m_gestureTimer.setInterval(std::chrono::milliseconds(16));
    m_gestureTimer.onTimeout = [this] { settleGesture(); };
    m_gestureTimer.start();
}

void LauncherWindow::settleGesture()
{
    const double elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - m_gestureTime).count();
    const double t = std::clamp(elapsed / 180, 0.0, 1.0);
    m_gestureProgress = m_gestureFrom + (m_gestureTarget - m_gestureFrom) * (1 - std::pow(1 - t, 3));
    presentGesture();
    if (t < 1) return;
    m_gestureTimer.stop(); m_gestureActive = false;
    if (m_gestureTarget == 0) m_launcher->hide();
    else m_window->activate();
    m_view->setEnabled(true);
    m_view->setTransform({}); m_view->setOpacity(1);
    if (auto *window = m_window->wmWindow())
        if (auto *animation = dynamic_cast<scene::WindowAnimation *>(window->node())) animation->setStyle(scene::WindowAnimation::Style::Launcher);
}

void LauncherWindow::showWindow()
{
    updateGeometry();
    m_view->setShown(true);
    m_window->show();
    // The trusted shell needs no activation token: the window manager's own
    // activation, as lingmo_shell_surface_v1.activate.
    m_window->activate();
    if (!m_gestureActive) {
        m_view->setOpacity(1); m_view->setTransform({}); m_view->setEnabled(true);
        if (auto *window = m_window->wmWindow())
            if (auto *animation = dynamic_cast<scene::WindowAnimation *>(window->node())) animation->setStyle(scene::WindowAnimation::Style::Launcher);
    }
}

void LauncherWindow::hideWindow()
{
    m_gestureTimer.stop(); m_gestureActive = false;
    m_view->setShown(false);
    m_window->hide();
    m_view->setEnabled(true);
    m_view->setOpacity(1); m_view->setTransform({});
}

bool LauncherWindow::keepsShownWhileInactive() const
{
    if (m_gestureActive || m_launcher->isDragging() || m_view->isMenuOpen())
        return true;
    // The capture tools take the keyboard to picture the launcher, not to close it.
    const auto &windows = m_shell->windowManager()->windows();
    return std::any_of(windows.begin(), windows.end(), [](const wm::Window *window) {
        return window->role() == wm::WindowRole::Capture && window->isVisible();
    });
}

void LauncherWindow::activeChanged()
{
    ui::Window *window = m_window->window();
    if (!window || window->isActive() || !m_launcher->isShown() || keepsShownWhileInactive())
        return;
    // A turn later: activation can pass through other windows on its way to
    // one of the launcher's own.
    std::weak_ptr<bool> alive = m_alive;
    core::mainLoop()->post([this, alive] {
        if (alive.expired())
            return;
        ui::Window *window = m_window->window();
        if (!window || window->isActive() || !m_launcher->isShown() || keepsShownWhileInactive())
            return;
        m_launcher->deactivated();
    });
}

} // namespace Lingmo::Shell
