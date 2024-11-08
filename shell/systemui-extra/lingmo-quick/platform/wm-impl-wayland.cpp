/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

#include "wm-impl-wayland.h"

#include <KWayland/Client/surface.h>
#include <QPlatformSurfaceEvent>
#include <QDebug>
#include <KWindowSystem>
#include <QGuiApplication>

namespace LingmoUIQuick {

WMImplWayland::WMImplWayland(QWindow *window) : WMInterface(window)
{
    m_pos = window->position();
    window->installEventFilter(this);
    initSurface();
}

QPoint WMImplWayland::position() const
{
    return m_pos;
}

void WMImplWayland::setPosition(const QPoint &point)
{
    m_pos = point;
    window()->setPosition(m_pos);
    if (m_shellSurface) {
        m_shellSurface->setPosition(m_pos);
    }
}

void WMImplWayland::setWindowType(WindowType::Type type)
{
    if (!m_shellSurface) {
        return;
    }

    switch (type) {
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
        case WindowType::Dialog:
            window()->setFlags(window()->flags() | Qt::Dialog);
        case WindowType::Menu:
        case WindowType::ToolTip:
            m_shellSurface->setRole(LingmoUIShellSurface::Role::ToolTip);
            break;
        case WindowType::PopupMenu:
            window()->setFlags(window()->flags() | Qt::Popup);
            m_shellSurface->setRole(LingmoUIShellSurface::Role::ToolTip);
            break;
        case WindowType::AppletPopup:
            m_shellSurface->setRole(LingmoUIShellSurface::Role::AppletPop);
            break;
    }

    // thank kde.
    if (type == WindowType::OnScreenDisplay) {
        window()->setFlags((window()->flags() & ~Qt::Dialog) | Qt::Window);
    }

    bool onAllDesktop = (type == WindowType::Desktop
                         || type == WindowType::Dock || type == WindowType::Panel
                         || type == WindowType::SystemWindow
                         || type == WindowType::Notification
                         || type == WindowType::CriticalNotification
                         || type == WindowType::OnScreenDisplay);

    // TODO: wlcom ?
    KWindowSystem::setOnAllDesktops(window()->winId(), onAllDesktop);
}

void WMImplWayland::setSkipTaskBar(bool skip)
{
    if (m_shellSurface) {
        m_shellSurface->setSkipTaskbar(skip);
    }
}

void WMImplWayland::setSkipSwitcher(bool skip)
{
    if (m_shellSurface) {
        m_shellSurface->setSkipSwitcher(skip);
    }
}

void WMImplWayland::setRemoveTitleBar(bool remove)
{
    if (window()->flags().testFlag(Qt::FramelessWindowHint)) {
        return;
    }

    if (m_shellSurface) {
        m_shellSurface->setSurfaceProperty(LingmoUIShellSurface::SurfaceProperty::NoTitleBar, remove ? 1 : 0);
    }
}

void WMImplWayland::setPanelAutoHide(bool autoHide)
{
    if (m_shellSurface) {
        m_shellSurface->setPanelAutoHide(autoHide);
    }
}

void WMImplWayland::setPanelTakesFocus(bool takesFocus)
{
    if (m_shellSurface) {
        m_shellSurface->setPanelAutoHide(takesFocus);
    }
}

QScreen* WMImplWayland::currentScreen()
{
    auto shell = WaylandIntegration::self()->waylandLingmoUIShell();
    shell->updateCurrentOutput();
    WaylandIntegration::self()->sync();
    if (shell->isCurrentOutputReady()) {
        QString name = shell->outputName();
        for (auto screen: qApp->screens()) {
            if (screen->name() == name) {
                return screen;
            }
        }
    }

}

bool WMImplWayland::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == window()) {
        switch (event->type()) {
            case QEvent::PlatformSurface: {
                const auto surfaceEvent = static_cast<QPlatformSurfaceEvent *>(event);
                if (surfaceEvent->surfaceEventType() != QPlatformSurfaceEvent::SurfaceCreated) {
                    destroySurface();
                }
                break;
            }
            case QEvent::Expose:
                if (!m_shellSurface) {
                    initSurface();

                    // 初始化位置
                    if (m_shellSurface) {
                        m_shellSurface->setPosition(m_pos);
                    }
                }
                break;
            case QEvent::Hide:
                destroySurface();
                break;
            default:
                break;
        }
    }

    return QObject::eventFilter(watched, event);
}

void WMImplWayland::initSurface()
{
    auto shell = WaylandIntegration::self()->waylandLingmoUIShell();
    if (!shell) {
        return;
    }

    KWayland::Client::Surface *surface = KWayland::Client::Surface::fromWindow(window());
    if (!surface) {
        return;
    }

    m_shellSurface.reset(shell->createSurface(surface));
}

inline void WMImplWayland::destroySurface()
{
    m_shellSurface.reset();
}

} // LingmoUIQuick
