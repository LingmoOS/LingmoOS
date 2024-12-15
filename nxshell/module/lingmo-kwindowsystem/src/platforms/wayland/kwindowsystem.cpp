/*
 *   Copyright 2015 Martin Gräßlin <mgraesslin@kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "kwindowsystem_p_wayland.h"
#include <QList>
#include <QMetaMethod>
#include <QPixmap>

QList<WId> KWindowSystemPrivateWayland::windows()
{
    return QList<WId>();
}

QList<WId> KWindowSystemPrivateWayland::stackingOrder()
{
    return QList<WId>();
}

WId KWindowSystemPrivateWayland::activeWindow()
{
    return 0;
}

void KWindowSystemPrivateWayland::activateWindow(WId win, long time)
{
    Q_UNUSED(win)
    Q_UNUSED(time)
}

void KWindowSystemPrivateWayland::forceActiveWindow(WId win, long time)
{
    Q_UNUSED(win)
    Q_UNUSED(time)
}

void KWindowSystemPrivateWayland::demandAttention(WId win, bool set)
{
    Q_UNUSED(win)
    Q_UNUSED(set)
}

bool KWindowSystemPrivateWayland::compositingActive()
{
    return true;
}

int KWindowSystemPrivateWayland::currentDesktop()
{
    return 0;
}

int KWindowSystemPrivateWayland::numberOfDesktops()
{
    return 0;
}

void KWindowSystemPrivateWayland::setCurrentDesktop(int desktop)
{
    Q_UNUSED(desktop)
}

void KWindowSystemPrivateWayland::setOnAllDesktops(WId win, bool b)
{
    Q_UNUSED(win)
    Q_UNUSED(b)
}

void KWindowSystemPrivateWayland::setOnDesktop(WId win, int desktop)
{
    Q_UNUSED(win)
    Q_UNUSED(desktop)
}

void KWindowSystemPrivateWayland::setOnActivities(WId win, const QStringList &activities)
{
    Q_UNUSED(win)
    Q_UNUSED(activities)
}

#ifndef KWINDOWSYSTEM_NO_DEPRECATED
WId KWindowSystemPrivateWayland::transientFor(WId window)
{
    Q_UNUSED(window)
    return 0;
}

WId KWindowSystemPrivateWayland::groupLeader(WId window)
{
    Q_UNUSED(window)
    return 0;
}
#endif

QPixmap KWindowSystemPrivateWayland::icon(WId win, int width, int height, bool scale, int flags)
{
    Q_UNUSED(win)
    Q_UNUSED(width)
    Q_UNUSED(height)
    Q_UNUSED(scale)
    Q_UNUSED(flags)
    return QPixmap();
}

void KWindowSystemPrivateWayland::setIcons(WId win, const QPixmap &icon, const QPixmap &miniIcon)
{
    Q_UNUSED(win)
    Q_UNUSED(icon)
    Q_UNUSED(miniIcon)
}

void KWindowSystemPrivateWayland::setType(WId win, NET::WindowType windowType)
{
    Q_UNUSED(win)
    Q_UNUSED(windowType)
}

void KWindowSystemPrivateWayland::setState(WId win, NET::States state)
{
    Q_UNUSED(win)
    Q_UNUSED(state)
}

void KWindowSystemPrivateWayland::clearState(WId win, NET::States state)
{
    Q_UNUSED(win)
    Q_UNUSED(state)
}

void KWindowSystemPrivateWayland::minimizeWindow(WId win)
{
    Q_UNUSED(win)
}

void KWindowSystemPrivateWayland::unminimizeWindow(WId win)
{
    Q_UNUSED(win)
}

void KWindowSystemPrivateWayland::raiseWindow(WId win)
{
    Q_UNUSED(win)
}

void KWindowSystemPrivateWayland::lowerWindow(WId win)
{
    Q_UNUSED(win)
}

bool KWindowSystemPrivateWayland::icccmCompliantMappingState()
{
    return false;
}

QRect KWindowSystemPrivateWayland::workArea(int desktop)
{
    Q_UNUSED(desktop)
    return QRect();
}

QRect KWindowSystemPrivateWayland::workArea(const QList<WId> &excludes, int desktop)
{
    Q_UNUSED(excludes)
    Q_UNUSED(desktop)
    return QRect();
}

QString KWindowSystemPrivateWayland::desktopName(int desktop)
{
    Q_UNUSED(desktop)
    return QString();
}

void KWindowSystemPrivateWayland::setDesktopName(int desktop, const QString &name)
{
    Q_UNUSED(desktop)
    Q_UNUSED(name)
}

bool KWindowSystemPrivateWayland::showingDesktop()
{
    return false;
}

void KWindowSystemPrivateWayland::setShowingDesktop(bool showing)
{
    Q_UNUSED(showing);
}

void KWindowSystemPrivateWayland::setUserTime(WId win, long time)
{
    Q_UNUSED(win)
    Q_UNUSED(time)
}

void KWindowSystemPrivateWayland::setExtendedStrut(WId win, int left_width, int left_start, int left_end,
                                                 int right_width, int right_start, int right_end, int top_width, int top_start, int top_end,
                                                 int bottom_width, int bottom_start, int bottom_end)
{
    Q_UNUSED(win)
    Q_UNUSED(left_width)
    Q_UNUSED(left_start)
    Q_UNUSED(left_end)
    Q_UNUSED(right_width)
    Q_UNUSED(right_start)
    Q_UNUSED(right_end)
    Q_UNUSED(top_width)
    Q_UNUSED(top_start)
    Q_UNUSED(top_end)
    Q_UNUSED(bottom_width)
    Q_UNUSED(bottom_start)
    Q_UNUSED(bottom_end)
}

void KWindowSystemPrivateWayland::setStrut(WId win, int left, int right, int top, int bottom)
{
    Q_UNUSED(win)
    Q_UNUSED(left)
    Q_UNUSED(right)
    Q_UNUSED(top)
    Q_UNUSED(bottom)
}

bool KWindowSystemPrivateWayland::allowedActionsSupported()
{
    return false;
}

QString KWindowSystemPrivateWayland::readNameProperty(WId window, unsigned long atom)
{
    Q_UNUSED(window)
    Q_UNUSED(atom)
    return QString();
}

void KWindowSystemPrivateWayland::allowExternalProcessWindowActivation(int pid)
{
    Q_UNUSED(pid)
}

void KWindowSystemPrivateWayland::setBlockingCompositing(WId window, bool active)
{
    Q_UNUSED(window)
    Q_UNUSED(active)
}

bool KWindowSystemPrivateWayland::mapViewport()
{
    return false;
}

int KWindowSystemPrivateWayland::viewportToDesktop(const QPoint &pos)
{
    Q_UNUSED(pos)
    return 0;
}

int KWindowSystemPrivateWayland::viewportWindowToDesktop(const QRect &r)
{
    Q_UNUSED(r)
    return 0;
}

QPoint KWindowSystemPrivateWayland::desktopToViewport(int desktop, bool absolute)
{
    Q_UNUSED(desktop)
    Q_UNUSED(absolute)
    return QPoint();
}

QPoint KWindowSystemPrivateWayland::constrainViewportRelativePosition(const QPoint &pos)
{
    Q_UNUSED(pos)
    return QPoint();
}

void KWindowSystemPrivateWayland::connectNotify(const QMetaMethod &signal)
{
    Q_UNUSED(signal)
}

