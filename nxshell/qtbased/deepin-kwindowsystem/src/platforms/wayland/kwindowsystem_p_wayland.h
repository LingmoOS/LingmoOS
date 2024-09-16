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
#ifndef KWINDOWSYSTEM_P_WAYLAND_H
#define KWINDOWSYSTEM_P_WAYLAND_H

#include "kwindowsystem_p.h"

class KWindowSystemPrivateWayland : public KWindowSystemPrivate
{
public:
    QList<WId> windows() Q_DECL_OVERRIDE;
    QList<WId> stackingOrder() Q_DECL_OVERRIDE;
    WId activeWindow() Q_DECL_OVERRIDE;
    void activateWindow(WId win, long time) Q_DECL_OVERRIDE;
    void forceActiveWindow(WId win, long time) Q_DECL_OVERRIDE;
    void demandAttention(WId win, bool set) Q_DECL_OVERRIDE;
    bool compositingActive() Q_DECL_OVERRIDE;
    int currentDesktop() Q_DECL_OVERRIDE;
    int numberOfDesktops() Q_DECL_OVERRIDE;
    void setCurrentDesktop(int desktop) Q_DECL_OVERRIDE;
    void setOnAllDesktops(WId win, bool b) Q_DECL_OVERRIDE;
    void setOnDesktop(WId win, int desktop) Q_DECL_OVERRIDE;
    void setOnActivities(WId win, const QStringList &activities) Q_DECL_OVERRIDE;
#ifndef KWINDOWSYSTEM_NO_DEPRECATED
    WId transientFor(WId window) Q_DECL_OVERRIDE;
    WId groupLeader(WId window) Q_DECL_OVERRIDE;
#endif
    QPixmap icon(WId win, int width, int height, bool scale, int flags) Q_DECL_OVERRIDE;
    void setIcons(WId win, const QPixmap &icon, const QPixmap &miniIcon) Q_DECL_OVERRIDE;
    void setType(WId win, NET::WindowType windowType) Q_DECL_OVERRIDE;
    void setState(WId win, NET::States state) Q_DECL_OVERRIDE;
    void clearState(WId win, NET::States state) Q_DECL_OVERRIDE;
    void minimizeWindow(WId win) Q_DECL_OVERRIDE;
    void unminimizeWindow(WId win) Q_DECL_OVERRIDE;
    void raiseWindow(WId win) Q_DECL_OVERRIDE;
    void lowerWindow(WId win) Q_DECL_OVERRIDE;
    bool icccmCompliantMappingState() Q_DECL_OVERRIDE;
    QRect workArea(int desktop) Q_DECL_OVERRIDE;
    QRect workArea(const QList<WId> &excludes, int desktop) Q_DECL_OVERRIDE;
    QString desktopName(int desktop) Q_DECL_OVERRIDE;
    void setDesktopName(int desktop, const QString &name) Q_DECL_OVERRIDE;
    bool showingDesktop() Q_DECL_OVERRIDE;
    void setShowingDesktop(bool showing) Q_DECL_OVERRIDE;
    void setUserTime(WId win, long time) Q_DECL_OVERRIDE;
    void setExtendedStrut(WId win, int left_width, int left_start, int left_end,
                          int right_width, int right_start, int right_end, int top_width, int top_start, int top_end,
                          int bottom_width, int bottom_start, int bottom_end) Q_DECL_OVERRIDE;
    void setStrut(WId win, int left, int right, int top, int bottom) Q_DECL_OVERRIDE;
    bool allowedActionsSupported() Q_DECL_OVERRIDE;
    QString readNameProperty(WId window, unsigned long atom) Q_DECL_OVERRIDE;
    void allowExternalProcessWindowActivation(int pid) Q_DECL_OVERRIDE;
    void setBlockingCompositing(WId window, bool active) Q_DECL_OVERRIDE;
    bool mapViewport() Q_DECL_OVERRIDE;
    int viewportToDesktop(const QPoint &pos) Q_DECL_OVERRIDE;
    int viewportWindowToDesktop(const QRect &r) Q_DECL_OVERRIDE;
    QPoint desktopToViewport(int desktop, bool absolute) Q_DECL_OVERRIDE;
    QPoint constrainViewportRelativePosition(const QPoint &pos) Q_DECL_OVERRIDE;

    void connectNotify(const QMetaMethod &signal) Q_DECL_OVERRIDE;
};

#endif
