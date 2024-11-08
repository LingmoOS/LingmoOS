/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "window-helper.h"

#include <QGuiApplication>
#include <QtX11Extras/QX11Info>
#include <KWindowSystem>
#include <QTimer>
#include <QDebug>
#include <QPlatformSurfaceEvent>

// lingmosdk
#include <lingmosdk/applications/windowmanager/windowmanager.h>
#include <lingmosdk/applications/lingmostylehelper/lingmo-decoration-manager.h>
#include <lingmostylehelper/xatom-helper.h>

#include <windowmanager/lingmoshell.h>
#include <KWayland/Client/surface.h>
#include <KWindowEffects>
#include "wayland-integration_p.h"

namespace LingmoUIQuick {

// ====== WindowProxyPrivate ====== //
class WindowProxyPrivate
{
public:
    bool setupWaylandIntegration();

    void removeHeaderBar(bool remove = true);
    void skipTaskBarOrSwitcher();
    void setWindowTypeX11();
    void setWindowTypeWayland();

    void updateGeometry();
    void updatePoint();
    void setWindowType();
    void setAutoHideState();

    void exec();

    // 代理操作
    bool m_isWayland = false;
    bool m_panelAutoHide = false;
    bool m_removeTitleBar = false;

    // 窗口类型
    WindowType::Type m_type = WindowType::Normal;
    WindowProxy::Operations m_operations;

    // old winId
    WId        m_winId {0};
    // 被代理的窗口
    QWindow   *m_window{nullptr};
    QSize      m_size;
    QPoint     m_point;

    QPointer<LingmoUIShellSurface> m_shellSurface;

    int  m_slideOffset = -1;
    WindowProxy::SlideFromEdge m_slideFromEdge = WindowProxy::NoEdge;
};

bool WindowProxyPrivate::setupWaylandIntegration()
{
    if (m_shellSurface) {
        // already setup
        return true;
    }

    LingmoUIShell *interface = WaylandIntegration::self()->waylandLingmoUIShell();
    if (!interface) {
        return false;
    }

    KWayland::Client::Surface *s = KWayland::Client::Surface::fromWindow(m_window);
    if (!s) {
        return false;
    }

    m_shellSurface = interface->createSurface(s, m_window);
    return m_shellSurface != nullptr;
}

void WindowProxyPrivate::removeHeaderBar(bool remove)
{
    // 无边框窗口不需要去除标题栏
    if (m_window->flags().testFlag(Qt::FramelessWindowHint)) {
        return;
    }

    if (m_isWayland) {
        if (m_shellSurface) {
            m_shellSurface->setSurfaceProperty(LingmoUIShellSurface::SurfaceProperty::NoTitleBar, remove ? 1 : 0);
        }
    } else {
        //x下暂不能取消
        if (remove && m_winId != m_window->winId()) {
            m_winId = m_window->winId();

            MotifWmHints hints;
            hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
            hints.functions = MWM_FUNC_ALL;
            hints.decorations = MWM_DECOR_BORDER;
            XAtomHelper::getInstance()->setWindowMotifHint(static_cast<int>(m_winId), hints);
        }
    }
}

void WindowProxyPrivate::setWindowTypeX11()
{
    NET::WindowType t;
    switch (m_type) {
        default:
            t = NET::Unknown;
            break;
        case WindowType::Normal:
            t = NET::Normal;
            break;
        case WindowType::Dock:
        case WindowType::Panel:
            t = NET::Dock;
            break;
        case WindowType::Desktop:
            t = NET::Desktop;
            break;
        case WindowType::Menu:
            t = NET::Menu;
            break;
        case WindowType::Dialog:
            t = NET::Dialog;
            m_window->setFlags(m_window->flags() | Qt::Dialog);
            break;
        case WindowType::PopupMenu:
            t = NET::PopupMenu;
            m_window->setFlags(m_window->flags() | Qt::Popup);
            break;
        case WindowType::ToolTip:
            t = NET::Tooltip;
            m_window->setFlags(m_window->flags() | Qt::ToolTip);
            break;
        case WindowType::Notification:
            t = NET::Notification;
            break;
        case WindowType::CriticalNotification:
            t = NET::CriticalNotification;
            break;
        case WindowType::SystemWindow:
            t = NET::SystemWindow;
            break;
        case WindowType::OnScreenDisplay:
            t = NET::OnScreenDisplay;
            break;
        case WindowType::AppletPopup:
            t = NET::AppletPopup;
            break;
    }

    KWindowSystem::setType(m_window->winId(), t);
}

void WindowProxyPrivate::setWindowTypeWayland()
{
   if (!m_shellSurface) {
       return;
   }

    switch (m_type) {
        default:
        case WindowType::Normal:
            m_shellSurface->setRole(LingmoUIShellSurface::Role::Normal);
            break;
        case WindowType::Desktop:
            m_shellSurface->setRole(LingmoUIShellSurface::Role::Desktop);
            break;
        case WindowType::Dock:
        case WindowType::Panel:
            m_shellSurface->setRole(LingmoUIShellSurface::Role::Panel);
            break;
        case WindowType::SystemWindow:
            m_shellSurface->setRole(LingmoUIShellSurface::Role::SystemWindow);
            break;
        case WindowType::Notification:
            m_shellSurface->setRole(LingmoUIShellSurface::Role::Notification);
            break;
        case WindowType::CriticalNotification:
            m_shellSurface->setRole(LingmoUIShellSurface::Role::CriticalNotification);
            break;
        case WindowType::ScreenLockNotification:
            m_shellSurface->setRole(LingmoUIShellSurface::Role::ScreenLockNotification);
            break;
        case WindowType::OnScreenDisplay:
            m_shellSurface->setRole(LingmoUIShellSurface::Role::OnScreenDisplay);
            break;
        case WindowType::Menu:
        case WindowType::Dialog:
            m_window->setFlags(m_window->flags() | Qt::Dialog);
        case WindowType::ToolTip:
            m_shellSurface->setRole(LingmoUIShellSurface::Role::ToolTip);
            break;
        case WindowType::PopupMenu:
            m_window->setFlags(m_window->flags() | Qt::Popup);
            break;
        case WindowType::AppletPopup:
            m_shellSurface->setRole(LingmoUIShellSurface::Role::AppletPop);
            break;
        case WindowType::Switcher:
            m_shellSurface->setRole(LingmoUIShellSurface::Role::Switcher);
            break;
    }
}

void WindowProxyPrivate::updateGeometry()
{
    if (m_size.isValid()) {
        m_window->resize(m_size);
    }
    updatePoint();
}

void WindowProxyPrivate::updatePoint()
{
    m_window->setPosition(m_point);
    if (m_shellSurface) {
        m_shellSurface->setPosition(m_point);
    }
}

void WindowProxyPrivate::setWindowType()
{
    if (m_isWayland) {
        setWindowTypeWayland();
    } else {
        setWindowTypeX11();
    }

    // thank kde.
    if (m_type == WindowType::OnScreenDisplay) {
        m_window->setFlags((m_window->flags() & ~Qt::Dialog) | Qt::Window);
    }

    if (m_type == WindowType::Dock || m_type == WindowType::Notification || m_type == WindowType::OnScreenDisplay || m_type == WindowType::CriticalNotification) {
        KWindowSystem::setOnAllDesktops(m_window->winId(), true);
    } else {
        KWindowSystem::setOnAllDesktops(m_window->winId(), false);
    }
}

void WindowProxyPrivate::skipTaskBarOrSwitcher()
{
    if (m_isWayland) {
        if (m_shellSurface) {
            m_shellSurface->setSkipTaskbar(m_operations.testFlag(WindowProxy::SkipTaskBar));
            m_shellSurface->setSkipSwitcher(m_operations.testFlag(WindowProxy::SkipSwitcher));
        }
    } else {
        NET::States states;
        if (m_operations.testFlag(WindowProxy::SkipTaskBar)) {
            states |= NET::SkipTaskbar;
        }
        if (m_operations.testFlag(WindowProxy::SkipSwitcher)) {
            states |= (NET::SkipPager | NET::SkipSwitcher);
        }

        KWindowSystem::setState(m_window->winId(), states);
    }
}

void WindowProxyPrivate::setAutoHideState()
{
    if (m_shellSurface) {
        m_shellSurface->setPanelAutoHide(m_panelAutoHide);
    }
}

void WindowProxyPrivate::exec()
{
    if (m_isWayland) {
        if (m_shellSurface) {
            return;
        }

        if (!setupWaylandIntegration()) {
            return;
        }
    }

    setWindowType();
    setAutoHideState();
    removeHeaderBar(m_operations.testFlag(WindowProxy::RemoveTitleBar));
    skipTaskBarOrSwitcher();
    updateGeometry();
}

// ====== WindowProxy ====== //
WindowProxy::WindowProxy(QWindow *window, WindowProxy::Operations operations)
    : QObject(window),
    d(new WindowProxyPrivate)
{
    Q_ASSERT(window);

    d->m_window = window;
    d->m_operations = operations;
    d->m_isWayland = WindowProxy::isWayland();
    d->m_window->installEventFilter(this);
}

WindowProxy::~WindowProxy()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

bool WindowProxy::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->m_window) {
        switch (event->type()) {
            case QEvent::PlatformSurface: {
                const QPlatformSurfaceEvent *pSEvent = static_cast<QPlatformSurfaceEvent *>(event);
                if (pSEvent->surfaceEventType() != QPlatformSurfaceEvent::SurfaceCreated) {
                    break;
                }
                // 继续走Show事件的操作
            }
            case QEvent::Show:
                d->exec();
                WindowProxy::slideWindow(d->m_window, d->m_slideFromEdge, d->m_slideOffset);
                break;
            case QEvent::Expose:
                d->exec();
                break;
            case QEvent::Hide:
                delete d->m_shellSurface;
                d->m_shellSurface = nullptr;
            default:
                break;
        }
    }

    return QObject::eventFilter(watched, event);
}

bool WindowProxy::isWayland()
{
    return QGuiApplication::platformName().startsWith(QStringLiteral("wayland"));
}

// ==APIS==
void WindowProxy::setRemoveTitleBar(bool remove)
{
    if (remove) {
        d->m_operations |= RemoveTitleBar;
    } else {
        d->m_operations &= ~RemoveTitleBar;
    }
}

void WindowProxy::setSkipTaskBar(bool skip)
{
    if (skip) {
        d->m_operations |= SkipTaskBar;
    } else {
        d->m_operations &= ~SkipTaskBar;
    }
}

void WindowProxy::setSkipSwitcher(bool skip)
{
    if (skip) {
        d->m_operations |= SkipSwitcher;
    } else {
        d->m_operations &= ~SkipSwitcher;
    }
}

void WindowProxy::setPosition(const QPoint &point)
{
    d->m_point = point;
    d->updateGeometry();
}

void WindowProxy::setGeometry(const QRect &rect)
{
    d->m_size = rect.size();
    d->m_point = rect.topLeft();
    d->updateGeometry();
}

void WindowProxy::setWindowType(WindowType::Type type)
{
    d->m_type = type;
    d->setWindowType();
}

void WindowProxy::setPanelAutoHide(bool autoHide)
{
    d->m_panelAutoHide = autoHide;
    d->setAutoHideState();
}

void WindowProxy::slideWindow(QWindow *window, WindowProxy::SlideFromEdge fromEdge, int offset)
{
    KWindowEffects::SlideFromLocation fromLocation;
    switch (fromEdge) {
        case NoEdge:
            fromLocation = KWindowEffects::SlideFromLocation::NoEdge;
            break;
        case TopEdge:
            fromLocation = KWindowEffects::SlideFromLocation::TopEdge;
            break;
        case RightEdge:
            fromLocation = KWindowEffects::SlideFromLocation::RightEdge;
            break;
        case BottomEdge:
            fromLocation = KWindowEffects::SlideFromLocation::BottomEdge;
            break;
        case LeftEdge:
            fromLocation = KWindowEffects::SlideFromLocation::LeftEdge;
            break;
    }

    KWindowEffects::slideWindow(window, fromLocation, offset);
}

void WindowProxy::slideWindow(WindowProxy::SlideFromEdge fromEdge, int offset)
{
    d->m_slideFromEdge = fromEdge;
    d->m_slideOffset = offset;
    WindowProxy::slideWindow(d->m_window, fromEdge, offset);
}

void WindowProxy::setBlurRegion(QWindow *window, bool enable, const QRegion &region)
{
    KWindowEffects::enableBlurBehind(window, enable, region);
}

void WindowProxy::setBlurRegion(bool enable, const QRegion &region)
{
    KWindowEffects::enableBlurBehind(d->m_window, enable, region);
}

QScreen* WindowProxy::currentScreen()
{
    if(isWayland()) {
        auto shell = WaylandIntegration::self()->waylandLingmoUIShell();
        shell->updateCurrentOutput();
        WaylandIntegration::self()->sync();
        if(shell->isCurrentOutputReady()) {
            QString name = shell->outputName();
            for (auto screen: qApp->screens()) {
                if (screen->name() == name) {
                    return screen;
                }
            }
        }
    } else {
        QScreen * screen =  qApp->screenAt(QCursor::pos());
        if(screen) {
            return screen;
        }
    }
    return qApp->primaryScreen();
}

// ======== WindowProxy2 =========== //
class WindowProxy2::Private
{
public:
    WMInterface *m_wm {nullptr};

    bool m_needResetProp = true;
    bool m_panelAutoHide = false;
    bool m_panelTakeFocus = false;
    WindowProxy::Operations m_operations;
    // 窗口类型
    WindowType::Type m_type = WindowType::Normal;
    QPoint m_pos;
    KWindowEffects::SlideFromLocation m_fromLocation;
    int m_offset;

    void setupProp();
};

void WindowProxy2::Private::setupProp()
{
    m_wm->setWindowType(m_type);

    m_wm->setSkipTaskBar(m_operations.testFlag(WindowProxy::SkipTaskBar));
    m_wm->setSkipSwitcher(m_operations.testFlag(WindowProxy::SkipSwitcher));
    m_wm->setRemoveTitleBar(m_operations.testFlag(WindowProxy::RemoveTitleBar));

    m_wm->setPanelTakesFocus(m_panelTakeFocus);
    m_wm->setPanelAutoHide(m_panelAutoHide);
}

WindowProxy2::WindowProxy2(QWindow *window, WindowProxy::Operations operations)
    : WMInterface(window), d(new WindowProxy2::Private)
{
    window->installEventFilter(this);

    d->m_operations = operations;
    d->m_wm = WManager::getWM(window);
}

WindowProxy2::~WindowProxy2()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

bool WindowProxy2::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == window()) {
        switch (event->type()) {
            case QEvent::Expose:
                if (!d->m_needResetProp || !window()->isVisible()) {
                    break;
                }
                d->m_needResetProp = false;
            case QEvent::Show:
                d->setupProp();
                KWindowEffects::slideWindow(window(), d->m_fromLocation, d->m_offset);
                break;
            case QEvent::Hide:
                d->m_needResetProp = true;
                break;
            default:
                break;
        }
    }

    return QObject::eventFilter(watched, event);
}

QPoint WindowProxy2::position() const
{
    return d->m_wm->position();
}

void WindowProxy2::setPosition(const QPoint &point)
{
    d->m_wm->setPosition(point);
}

void WindowProxy2::setWindowType(WindowType::Type type)
{
    if (d->m_type == type) {
        return;
    }

    d->m_type = type;
    d->m_wm->setWindowType(type);
}

void WindowProxy2::setSkipTaskBar(bool skip)
{
    if (skip) {
        d->m_operations |= WindowProxy::SkipTaskBar;
    } else {
        d->m_operations &= ~WindowProxy::SkipTaskBar;
    }

    d->m_wm->setSkipTaskBar( d->m_operations.testFlag(WindowProxy::SkipTaskBar));
}

void WindowProxy2::setSkipSwitcher(bool skip)
{
    if (skip) {
        d->m_operations |= WindowProxy::SkipSwitcher;
    } else {
        d->m_operations &= ~WindowProxy::SkipSwitcher;
    }

    d->m_wm->setSkipSwitcher( d->m_operations.testFlag(WindowProxy::SkipSwitcher));
}

void WindowProxy2::setRemoveTitleBar(bool remove)
{
    if (remove) {
        d->m_operations |= WindowProxy::RemoveTitleBar;
    } else {
        d->m_operations &= ~WindowProxy::RemoveTitleBar;
    }

    d->m_wm->setRemoveTitleBar( d->m_operations.testFlag(WindowProxy::RemoveTitleBar));
}

void WindowProxy2::setPanelTakesFocus(bool takesFocus)
{
    if (d->m_panelTakeFocus == takesFocus) {
        return;
    }

    d->m_panelTakeFocus = takesFocus;
    d->m_wm->setPanelTakesFocus(takesFocus);
}

void WindowProxy2::setPanelAutoHide(bool autoHide)
{
    if (d->m_panelAutoHide == autoHide) {
        return;
    }

    d->m_panelAutoHide = autoHide;
    d->m_wm->setPanelTakesFocus(autoHide);
}

void WindowProxy2::setSlideWindowArgs(WindowProxy::SlideFromEdge fromEdge, int offset)
{
    KWindowEffects::SlideFromLocation fromLocation;
    switch (fromEdge) {
        case WindowProxy::NoEdge:
            fromLocation = KWindowEffects::SlideFromLocation::NoEdge;
            break;
        case WindowProxy::TopEdge:
            fromLocation = KWindowEffects::SlideFromLocation::TopEdge;
            break;
        case WindowProxy::RightEdge:
            fromLocation = KWindowEffects::SlideFromLocation::RightEdge;
            break;
        case WindowProxy::BottomEdge:
            fromLocation = KWindowEffects::SlideFromLocation::BottomEdge;
            break;
        case WindowProxy::LeftEdge:
            fromLocation = KWindowEffects::SlideFromLocation::LeftEdge;
            break;
    }
    d->m_fromLocation = fromLocation;
    d->m_offset = offset;
    KWindowEffects::slideWindow(window(), d->m_fromLocation, d->m_offset);
}

QScreen* WindowProxy2::currentScreen()
{
    return d->m_wm->currentScreen();
}
} // LingmoUIQuick
