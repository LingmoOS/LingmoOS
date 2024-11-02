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

#include "wm-impl-x11.h"

#include <netwm.h>
#include <KWindowSystem>

#include <lingmostylehelper/xatom-helper.h>
#include <KWindowEffects>
#include <QDebug>
#include <QGuiApplication>

namespace LingmoUIQuick {

WMImplX11::WMImplX11(QWindow *window) : WMInterface(window)
{

}

void WMImplX11::setWindowType(WindowType::Type type)
{
    NET::WindowType t;
    switch (type) {
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
            window()->setFlags(window()->flags() | Qt::Dialog);
            break;
        case WindowType::PopupMenu:
            t = NET::PopupMenu;
            window()->setFlags(window()->flags() | Qt::Popup);
            break;
        case WindowType::ToolTip:
            t = NET::Tooltip;
            window()->setFlags(window()->flags() | Qt::ToolTip);
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

    KWindowSystem::setType(window()->winId(), t);

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

    KWindowSystem::setOnAllDesktops(window()->winId(), onAllDesktop);
}

void WMImplX11::setSkipTaskBar(bool skip)
{
    NET::States states;
    KWindowInfo windowInfo(window()->winId(), NET::WMState);
    if (windowInfo.valid()) {
        states = windowInfo.state();
    }

    if (skip) {
        states |= NET::SkipTaskbar;
    } else {
        states &= ~NET::SkipTaskbar;
    }

    KWindowSystem::setState(window()->winId(), states);
}

void WMImplX11::setSkipSwitcher(bool skip)
{
    NET::States states;
    KWindowInfo windowInfo(window()->winId(), NET::WMState);
    if (windowInfo.valid()) {
        states = windowInfo.state();
    }

    if (skip) {
        states |= NET::SkipSwitcher | NET::SkipPager;
    } else {
        states &= ~(NET::SkipSwitcher | NET::SkipPager);
    }

    KWindowSystem::setState(window()->winId(), states);
}

void WMImplX11::setRemoveTitleBar(bool remove)
{
    if (window()->flags().testFlag(Qt::FramelessWindowHint)) {
        return;
    }

    //x下暂不能取消
    if (remove) {
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(static_cast<int>(window()->winId()), hints);
    }
}

void WMImplX11::setPanelAutoHide(bool autoHide)
{
    Q_UNUSED(autoHide)
}

void WMImplX11::setPanelTakesFocus(bool takesFocus)
{
    Q_UNUSED(takesFocus)

//    Qt::WindowFlags flags = window()->flags();
//    if (takesFocus) {
//        flags &= ~Qt::WindowDoesNotAcceptFocus;
//    } else {
//        flags |= Qt::WindowDoesNotAcceptFocus;
//    }
//
//    window()->setFlags(flags);
}

QScreen* WMImplX11::currentScreen()
{
    QScreen* screen = qApp->screenAt(QCursor::pos());
    if (screen) {
        return screen;
    }
    return qApp->primaryScreen();
}
}
