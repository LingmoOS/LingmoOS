/*
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "window-manager.h"
#include <mutex>
#include "abstract-window-manager.h"
#include "xcb-window-manager.h"
#include "wayland-window-manager.h"
namespace LingmoUIQuick {
static WindowManager *g_windowManager = nullptr;
static AbstractWindowManager *g_abstractWindowManager = nullptr;

static std::once_flag flag_windowManager;
static std::once_flag flag_abstractWindowManager;

class WindowManagerPrivate
{
public:
    static AbstractWindowManager *interface();
};

AbstractWindowManager *WindowManagerPrivate::interface()
{
    std::call_once(flag_abstractWindowManager, [ & ] {
        if(QString(getenv("XDG_SESSION_TYPE")) == "wayland") {
            g_abstractWindowManager = new WaylandWindowManager();
        } else {
            g_abstractWindowManager = new XcbWindowManager();
        }
    });
    return g_abstractWindowManager;
}

WindowManager *WindowManager::self()
{
    std::call_once(flag_windowManager, [ & ] {
        g_windowManager = new WindowManager();
    });
    return g_windowManager;
}

WindowManager::WindowManager(QObject *parent) : QObject(parent)
{
    connect(WindowManagerPrivate::interface(), &AbstractWindowManager::windowAdded, this, &WindowManager::windowAdded, Qt::QueuedConnection);
    connect(WindowManagerPrivate::interface(), &AbstractWindowManager::windowRemoved, this, &WindowManager::windowRemoved, Qt::QueuedConnection);
    connect(WindowManagerPrivate::interface(), &AbstractWindowManager::currentDesktopChanged, this, &WindowManager::currentDesktopChanged, Qt::QueuedConnection);
    connect(WindowManagerPrivate::interface(), &AbstractWindowManager::titleChanged, this, &WindowManager::titleChanged, Qt::QueuedConnection);
    connect(WindowManagerPrivate::interface(), &AbstractWindowManager::iconChanged, this, &WindowManager::iconChanged, Qt::QueuedConnection);
    connect(WindowManagerPrivate::interface(), &AbstractWindowManager::skipTaskbarChanged, this, &WindowManager::skipTaskbarChanged, Qt::QueuedConnection);
    connect(WindowManagerPrivate::interface(), &AbstractWindowManager::onAllDesktopsChanged, this, &WindowManager::onAllDesktopsChanged, Qt::QueuedConnection);
    connect(WindowManagerPrivate::interface(), &AbstractWindowManager::windowDesktopChanged, this, &WindowManager::windowDesktopChanged, Qt::QueuedConnection);
    connect(WindowManagerPrivate::interface(), &AbstractWindowManager::demandsAttentionChanged, this, &WindowManager::demandsAttentionChanged, Qt::QueuedConnection);
    connect(WindowManagerPrivate::interface(), &AbstractWindowManager::geometryChanged, this, &WindowManager::geometryChanged, Qt::QueuedConnection);
    connect(WindowManagerPrivate::interface(), &AbstractWindowManager::activeWindowChanged, this, &WindowManager::activeWindowChanged, Qt::QueuedConnection);
    connect(WindowManagerPrivate::interface(), &AbstractWindowManager::windowStateChanged, this, &WindowManager::windowStateChanged, Qt::QueuedConnection);
    connect(WindowManagerPrivate::interface(), &AbstractWindowManager::maximizedChanged, this, &WindowManager::maximizedChanged, Qt::QueuedConnection);
}

QStringList WindowManager::windows()
{
    return WindowManagerPrivate::interface()->windows();
}

QIcon WindowManager::windowIcon(const QString &wid)
{
    return WindowManagerPrivate::interface()->windowIcon(wid);
}

QString WindowManager::windowTitle(const QString &wid)
{
    return WindowManagerPrivate::interface()->windowTitle(wid);
}

bool WindowManager::skipTaskBar(const QString &wid)
{
    return WindowManagerPrivate::interface()->skipTaskBar(wid);
}

QString WindowManager::windowGroup(const QString &wid)
{
    return WindowManagerPrivate::interface()->windowGroup(wid);
}

bool WindowManager::isMaximized(const QString &wid)
{
    return WindowManagerPrivate::interface()->isMaximized(wid);
}

void WindowManager::maximizeWindow(const QString &wid)
{
    WindowManagerPrivate::interface()->maximizeWindow(wid);
}

bool WindowManager::isMinimized(const QString &wid)
{
    return WindowManagerPrivate::interface()->isMinimized(wid);
}

void WindowManager::minimizeWindow(const QString &wid)
{
    WindowManagerPrivate::interface()->minimizeWindow(wid);
}

bool WindowManager::isKeepAbove(const QString &wid)
{
    return WindowManagerPrivate::interface()->isKeepAbove(wid);;
}

void WindowManager::keepAboveWindow(const QString &wid)
{
    WindowManagerPrivate::interface()->keepAboveWindow(wid);
}

bool WindowManager::isOnAllDesktops(const QString &wid)
{
    return WindowManagerPrivate::interface()->isOnAllDesktops(wid);
}

bool WindowManager::isOnCurrentDesktop(const QString &wid)
{
    return WindowManagerPrivate::interface()->isOnCurrentDesktop(wid);
}

void WindowManager::activateWindow(const QString &wid)
{
    WindowManagerPrivate::interface()->activateWindow(wid);
}

QString WindowManager::currentActiveWindow()
{
    return WindowManagerPrivate::interface()->currentActiveWindow();;
}

void WindowManager::closeWindow(const QString &wid)
{
    WindowManagerPrivate::interface()->closeWindow(wid);
}

void WindowManager::restoreWindow(const QString &wid)
{
    WindowManagerPrivate::interface()->restoreWindow(wid);
}

bool WindowManager::isDemandsAttention(const QString &wid)
{
    return WindowManagerPrivate::interface()->isDemandsAttention(wid);
}

quint32 WindowManager::pid(const QString &wid)
{
    return WindowManagerPrivate::interface()->pid(wid);;
}

QString WindowManager::appId(const QString &wid)
{
    return WindowManagerPrivate::interface()->appId(wid);
}

bool WindowManager::isWaylandSession()
{
    return QString(getenv("XDG_SESSION_TYPE")) == "wayland";
}

QRect WindowManager::geometry(const QString &wid)
{
    return WindowManagerPrivate::interface()->geometry(wid);
}

void WindowManager::setStartupGeometry(const QString &wid, QQuickItem *item)
{
    WindowManagerPrivate::interface()->setStartupGeometry(wid, item);
}

void WindowManager::setMinimizedGeometry(const QString &wid, QQuickItem *item)
{
    WindowManagerPrivate::interface()->setMinimizedGeometry(wid, item);
}

void WindowManager::unsetMinimizedGeometry(const QString &wid, QQuickItem *item)
{
    WindowManagerPrivate::interface()->unsetMinimizedGeometry(wid, item);
}

void WindowManager::setMinimizedGeometry(const QStringList &widList, QQuickItem *item)
{
    for (const auto &wid : widList) {
        WindowManagerPrivate::interface()->setMinimizedGeometry(wid, item);
    }
}

void WindowManager::activateWindowView(const QStringList &wids)
{
    WindowManagerPrivate::interface()->activateWindowView(wids);
}

bool WindowManager::isMaximizable(const QString &wid)
{
    return WindowManagerPrivate::interface()->isMaximizable(wid);
}

bool WindowManager::isMinimizable(const QString &wid)
{
    return WindowManagerPrivate::interface()->isMinimizable(wid);
}
}