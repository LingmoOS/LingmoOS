// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "waylanddockhelper.h"
#include "appletbridge.h"
#include "constants.h"
#include "dockhelper.h"
#include "dockpanel.h"
#include "layershell/dlayershellwindow.h"
#include "qwayland-treeland-ocean-shell-v1.h"
#include "wayland-treeland-ocean-shell-v1-client-protocol.h"

#include <QtWaylandClient/private/qwaylandscreen_p.h>
#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

namespace dock {
WaylandDockHelper::WaylandDockHelper(DockPanel *panel)
    : DockHelper(panel)
    , m_panel(panel)
    , m_isWindowOverlap(false)
    , m_isCurrentActiveWindowFullscreened(false)
{
    m_wallpaperColorManager.reset(new WallpaperColorManager(this));
    m_oceanShellManager.reset(new TreeLandOCEANShellManager());
    DS_NAMESPACE::DAppletBridge bridge("org.lingmo.ds.dock.taskmanager");
    if (auto applet = bridge.applet()) {
        connect(applet, SIGNAL(windowFullscreenChanged(bool)), this, SLOT(setCurrentActiveWindowFullscreened(bool)));
    }

    connect(m_panel, &DockPanel::rootObjectChanged, this, [this]() {
        m_wallpaperColorManager->watchScreen(dockScreenName());
    });

    connect(m_wallpaperColorManager.get(), &WallpaperColorManager::activeChanged, this, [this]() {
        if (m_panel->rootObject() != nullptr) {
            m_wallpaperColorManager->watchScreen(dockScreenName());
        }
    });

    connect(m_panel, &DockPanel::dockScreenChanged, this, [this]() {
        m_wallpaperColorManager->watchScreen(dockScreenName());
    });

    connect(m_panel, &DockPanel::positionChanged, this, &WaylandDockHelper::updateOverlapCheckerPos);
    connect(m_panel, &DockPanel::dockSizeChanged, this, &WaylandDockHelper::updateOverlapCheckerPos);
    connect(m_panel, &DockPanel::rootObjectChanged, this, &WaylandDockHelper::updateOverlapCheckerPos);

    connect(m_oceanShellManager.get(), &TreeLandOCEANShellManager::activeChanged, this, [this]() {
        if (m_panel->hideMode() == HideMode::SmartHide && m_oceanShellManager->isActive()) {
            m_overlapChecker.reset(new TreeLandWindowOverlapChecker(this, m_oceanShellManager->get_window_overlap_checker()));
            updateOverlapCheckerPos();
        } else {
            m_overlapChecker.reset(nullptr);
        }
    });

    connect(m_panel, &DockPanel::hideModeChanged, this, [this](HideMode mode) {
        if (mode == HideMode::SmartHide && m_oceanShellManager->isActive()) {
            m_overlapChecker.reset(new TreeLandWindowOverlapChecker(this, m_oceanShellManager->get_window_overlap_checker()));
            updateOverlapCheckerPos();
        } else {
            m_overlapChecker.reset(nullptr);
        }
    });

    if (m_panel->rootObject() != nullptr) {
        m_wallpaperColorManager->watchScreen(dockScreenName());
    }
}

void WaylandDockHelper::updateOverlapCheckerPos()
{
    if (!m_overlapChecker)
        return;

    if (!m_panel->rootObject())
        return;

    auto waylandScreen = dynamic_cast<QtWaylandClient::QWaylandScreen *>(m_panel->window()->screen()->handle());
    if (!waylandScreen)
        return;

    uint32_t anchor;
    switch (m_panel->position()) {
    case Top:
        anchor = QtWayland::treeland_window_overlap_checker::anchor_top;
        break;
    case Bottom:
        anchor = QtWayland::treeland_window_overlap_checker::anchor_bottom;
        break;
    case Left:
        anchor = QtWayland::treeland_window_overlap_checker::anchor_left;
        break;
    case Right:
        anchor = QtWayland::treeland_window_overlap_checker::anchor_right;
        break;
    }

    m_overlapChecker->update(m_panel->dockSize(), m_panel->dockSize(), anchor, waylandScreen->output());
}

DockWakeUpArea *WaylandDockHelper::createArea(QScreen *screen)
{
    return new TreeLandDockWakeUpArea(screen, this);
}

void WaylandDockHelper::destroyArea(DockWakeUpArea *area)
{
    auto waylandArea = static_cast<TreeLandDockWakeUpArea *>(area);
    if (waylandArea) {
        waylandArea->close();
        waylandArea->deleteLater();
    }
}

QString WaylandDockHelper::dockScreenName()
{
    if (m_panel->dockScreen() && m_panel->dockScreen()->handle())
        return m_panel->dockScreen()->name();

    return {};
}

bool WaylandDockHelper::currentActiveWindowFullscreened()
{
    return m_isCurrentActiveWindowFullscreened;
}

void WaylandDockHelper::setCurrentActiveWindowFullscreened(bool isFullscreen)
{
    m_isCurrentActiveWindowFullscreened = isFullscreen;
    emit currentActiveWindowFullscreenChanged(isFullscreen);
}

bool WaylandDockHelper::isWindowOverlap()
{
    return m_isWindowOverlap;
}

void WaylandDockHelper::setDockColorTheme(const ColorTheme &theme)
{
    m_panel->setColorTheme(theme);
}

WallpaperColorManager::WallpaperColorManager(WaylandDockHelper *helper)
    : QWaylandClientExtensionTemplate<WallpaperColorManager>(treeland_wallpaper_color_manager_v1_interface.version)
    , m_helper(helper)
{
}

void WallpaperColorManager::treeland_wallpaper_color_manager_v1_output_color(const QString &output, uint32_t isDark)
{
    if (output == m_helper->dockScreenName()) {
        m_helper->setDockColorTheme(isDark ? Dark : Light);
    }
}

void WallpaperColorManager::watchScreen(const QString &screeName)
{
    if (isActive() && !screeName.isEmpty()) {
        watch(screeName);
    }
}

TreeLandOCEANShellManager::TreeLandOCEANShellManager()
    : QWaylandClientExtensionTemplate<TreeLandOCEANShellManager>(treeland_ocean_shell_manager_v1_interface.version)
{
}

TreeLandWindowOverlapChecker::TreeLandWindowOverlapChecker(WaylandDockHelper *helper, struct ::treeland_window_overlap_checker *checker)
    : QWaylandClientExtensionTemplate<TreeLandWindowOverlapChecker>(treeland_ocean_shell_manager_v1_interface.version)
    , m_helper(helper)
{
    setParent(helper);
    init(checker);
}

TreeLandWindowOverlapChecker::~TreeLandWindowOverlapChecker()
{
    destroy();
}

void TreeLandWindowOverlapChecker::treeland_window_overlap_checker_enter()
{
    m_helper->m_isWindowOverlap = true;
    Q_EMIT m_helper->isWindowOverlapChanged(m_helper->m_isWindowOverlap);
}

void TreeLandWindowOverlapChecker::treeland_window_overlap_checker_leave()
{
    m_helper->m_isWindowOverlap = false;
    Q_EMIT m_helper->isWindowOverlapChanged(m_helper->m_isWindowOverlap);
}

TreeLandDockWakeUpArea::TreeLandDockWakeUpArea(QScreen *screen, WaylandDockHelper *helper)
    : QWidget()
    , DockWakeUpArea(screen, helper)
{
    // force create windowHandle for layershell.
    winId();
    window()->setScreen(screen);
    window()->resize(QSize(15, 15));
    setAttribute(Qt::WA_TranslucentBackground, true);

    auto window = ds::DLayerShellWindow::get(windowHandle());
    window->setLayer(ds::DLayerShellWindow::LayerOverlay);
    window->setAnchors({ds::DLayerShellWindow::AnchorBottom | ds::DLayerShellWindow::AnchorLeft | ds::DLayerShellWindow::AnchorRight});
    window->setExclusiveZone(-1);
}

void TreeLandDockWakeUpArea::open()
{
    show();
}

void TreeLandDockWakeUpArea::close()
{
    hide();
}

void TreeLandDockWakeUpArea::updateDockWakeArea(Position pos)
{
    m_pos = pos;
    ds::DLayerShellWindow::Anchors anchors = {0 | ds::DLayerShellWindow::AnchorNone};
    switch (pos) {
    case Top: {
        anchors = {ds::DLayerShellWindow::AnchorLeft | ds::DLayerShellWindow::AnchorRight | ds::DLayerShellWindow::AnchorTop};
        break;
    }
    case Right: {
        anchors = {ds::DLayerShellWindow::AnchorRight | ds::DLayerShellWindow::AnchorTop | ds::DLayerShellWindow::AnchorBottom};
        break;
    }
    case Bottom: {
        anchors = {ds::DLayerShellWindow::AnchorLeft | ds::DLayerShellWindow::AnchorRight | ds::DLayerShellWindow::AnchorBottom};
        break;
    }
    case Left: {
        anchors = {ds::DLayerShellWindow::AnchorLeft | ds::DLayerShellWindow::AnchorTop | ds::DLayerShellWindow::AnchorBottom};
        break;
    }
    }

    window()->resize(QSize(15, 15));
    auto window = ds::DLayerShellWindow::get(windowHandle());
    window->setAnchors(anchors);
}

void TreeLandDockWakeUpArea::enterEvent(QEnterEvent *event)
{
    m_helper->enterScreen(QWidget::screen());
}

void TreeLandDockWakeUpArea::leaveEvent(QEvent *event)
{
    m_helper->leaveScreen();
}

void TreeLandDockWakeUpArea::resizeEvent(QResizeEvent *event)
{
    auto size = event->size();
    if (m_pos == Left || m_pos == Right) {
        size.setHeight(m_screen->size().height());
        size.setWidth(15);
    } else {
        size.setHeight(15);
        size.setWidth(m_screen->size().width());
    }

    window()->resize(size);
}
}
