/*
 * liblingmosdk-waylandhelper's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef WINDOWINFO_H
#define WINDOWINFO_H

#include <QVariant>
#include <QRect>
#include <QIcon>

namespace kdk {


class WindowInfo
{
using WindowId = QVariant;
public:
    WindowInfo():
        m_handle(nullptr)
      , m_isValid(false)
      , m_isActive(false)
      , m_isMinimized(false)
      , m_isMaxVert(false)
      , m_isMaxHoriz(false)
      , m_isFullscreen(false)
      , m_isShaded(false)
      , m_isPlasmaDesktop(false)
      , m_isKeepAbove(false)
      , m_hasSkipTaskbar(false)
      , m_hasSkipSwitcher(false)
      , m_isOnAllDesktops(false)
      , m_isOnAllActivities(false)
      , m_isClosable(false)
      , m_isFullScreenable(false)
      , m_isGroupable(false)
      , m_isMaximizable(false)
      , m_isMinimizable(false)
      , m_isMovable(false)
      , m_isResizable(false)
      , m_isShadeable(false)
      , m_isVirtualDesktopsChangeable(false)
      , m_isDemandAttention(false){}

    inline bool isPlasmaDesktop() const;
    inline void setIsPlasmaDesktop(bool isPlasmaDesktop);

    inline bool isValid() const;
    inline void setIsValid(bool isValid);

    inline bool isActive() const noexcept;
    inline void setIsActive(bool isActive) noexcept;

    inline bool isMaxVert() const;
    inline void setIsMaxVert(bool isMaxVert);

    inline bool isMaxHoriz() const;
    inline void setIsMaxHoriz(bool isMaxHoriz);

    inline WindowId wid() const;
    inline void setWid(const WindowId &wid);

    inline bool isKeepAbove() const;
    inline void setIsKeepAbove(bool isKeepAbove);

    inline bool isMinimized() const;
    inline void setIsMinimized(bool isMinimized);

    inline bool isMaximized() const;

    inline bool isFullscreen() const noexcept;
    inline void setIsFullscreen(bool isFullscreen) noexcept;

    inline bool isShaded() const noexcept;
    inline void setIsShaded(bool isShaded) noexcept;

    inline bool hasSkipTaskbar() const noexcept;
    inline void setHasSkipTaskbar(bool skipTaskbar) noexcept;

    inline bool hasSkipSwitcher() const noexcept;
    inline void setHasSkipSwitcher(bool skipSwitcher) noexcept;

    inline bool isOnAllDesktops() const noexcept;
    inline void setIsOnAllDesktops(bool alldesktops) noexcept;

    inline bool isOnAllActivities() const noexcept;
    inline void setIsOnAllActivities(bool allactivities) noexcept;

    //ability
    inline bool isCloseable() const noexcept;
    inline void setIsClosable(bool closable) noexcept;

    inline bool isFullScreenable() const noexcept;
    inline void setIsFullScreenable(bool fullscreenable) noexcept;

    inline bool isGroupable() const noexcept;
    inline void setIsGroupable(bool groupable) noexcept;

    inline bool isMaximizable() const;
    inline void setIsMaximizable(bool maximizable);

    inline bool isMinimizable() const;
    inline void setIsMinimizable(bool minimizable);

    inline bool isMovable() const noexcept;
    inline void setIsMovable(bool movable) noexcept;

    inline bool isResizable() const noexcept;
    inline void setIsResizable(bool resizable) noexcept;

    inline bool isShadeable() const noexcept;
    inline void setIsShadeable(bool shadeble) noexcept;

    inline bool isVirtualDesktopsChangeable() const noexcept;
    inline void setIsVirtualDesktopsChangeable(bool virtualdesktopchangeable) noexcept;

    inline QIcon icon() const noexcept;
    inline void setIcon(const QIcon &icon) noexcept;

    inline QStringList desktops() const noexcept;
    inline void setDesktops(const QStringList &desktops) noexcept;

    inline bool isOnDesktop(const QString &desktop) const noexcept;

    inline bool isDemandAttention() const noexcept;
    inline void setIsDemandAttention(bool demandAttention) noexcept;


private:
    void* m_handle;
    bool m_isWayland;

    WindowId m_wid{0};
    WindowId m_parentId{0};

    QRect m_geometry;

    bool m_isValid : 1;
    bool m_isActive : 1;
    bool m_isMinimized : 1;
    bool m_isMaxVert : 1;
    bool m_isMaxHoriz : 1;
    bool m_isFullscreen : 1;
    bool m_isShaded : 1;
    bool m_isPlasmaDesktop : 1;
    bool m_isKeepAbove: 1;
    bool m_hasSkipTaskbar: 1;
    bool m_isOnAllDesktops: 1;
    bool m_isOnAllActivities: 1;
    bool m_hasSkipSwitcher: 1;

    //!BEGIN: Window Abilities
    bool m_isClosable : 1;
    bool m_isFullScreenable : 1;
    bool m_isGroupable : 1;
    bool m_isMaximizable : 1;
    bool m_isMinimizable : 1;
    bool m_isMovable : 1;
    bool m_isResizable : 1;
    bool m_isShadeable : 1;
    bool m_isVirtualDesktopsChangeable : 1;
    bool m_isDemandAttention : 1;

    QString m_appName;
    QString m_display;

    QIcon m_icon;

    QStringList m_desktops;
    QStringList m_activities;
};

bool WindowInfo::isPlasmaDesktop() const
{
    return m_isPlasmaDesktop;
}

void WindowInfo::setIsPlasmaDesktop(bool isPlasmaDesktop)
{
    m_isPlasmaDesktop = isPlasmaDesktop;
}

bool WindowInfo::isValid() const
{
    return m_isValid;
}

void WindowInfo::setIsValid(bool isValid)
{
    m_isValid = isValid;
}

bool WindowInfo::isActive() const noexcept
{
    return m_isActive;
}

void WindowInfo::setIsActive(bool isActive) noexcept
{
    m_isActive = isActive;
}

bool WindowInfo::isMaxVert() const
{
    return m_isMaxVert;
}

void WindowInfo::setIsMaxVert(bool isMaxVert)
{
    m_isMaxVert = isMaxVert;
}

bool WindowInfo::isMaxHoriz() const
{
    return m_isMaxHoriz;
}

void WindowInfo::setIsMaxHoriz(bool isMaxHoriz)
{
    m_isMaxHoriz = isMaxHoriz;
}

WindowInfo::WindowId WindowInfo::wid() const
{
    return m_wid;
}

void WindowInfo::setWid(const WindowId &wid)
{
    m_wid = wid;
}

bool WindowInfo::isKeepAbove() const
{
    return m_isKeepAbove;
}

void WindowInfo::setIsKeepAbove(bool isKeepAbove)
{
    m_isKeepAbove = isKeepAbove;
}

bool WindowInfo::isMinimized() const
{
    return m_isMinimized;
}

void WindowInfo::setIsMinimized(bool isMinimized)
{
    m_isMinimized = isMinimized;
}

bool WindowInfo::isMaximized() const
{
    return m_isMaxVert && m_isMaxHoriz;
}

bool WindowInfo::isFullscreen() const noexcept
{
    return m_isFullscreen;
}

void WindowInfo::setIsFullscreen(bool isFullscreen) noexcept
{
    m_isFullscreen = isFullscreen;
}

bool WindowInfo::isShaded() const noexcept
{
    return m_isShaded;
}

void WindowInfo::setIsShaded(bool isShaded) noexcept
{
    m_isShaded = isShaded;
}

bool WindowInfo::hasSkipTaskbar() const noexcept
{
    return m_hasSkipTaskbar;
}

void WindowInfo::setHasSkipTaskbar(bool skipTaskbar) noexcept
{
    m_hasSkipTaskbar = skipTaskbar;
}

inline bool WindowInfo::hasSkipSwitcher() const noexcept
{
    return m_hasSkipSwitcher;
}

inline void WindowInfo::setHasSkipSwitcher(bool skipSwitcher) noexcept
{
    m_hasSkipSwitcher = skipSwitcher;
}

bool WindowInfo::isOnAllDesktops() const noexcept
{
    return m_isOnAllDesktops;
}

void WindowInfo::setIsOnAllDesktops(bool alldesktops) noexcept
{
    m_isOnAllDesktops = alldesktops;
}

bool WindowInfo::isOnAllActivities() const noexcept
{
    return m_isOnAllActivities;
}

void WindowInfo::setIsOnAllActivities(bool allactivities) noexcept
{
    m_isOnAllActivities = allactivities;
}

bool WindowInfo::isCloseable() const noexcept
{
    return m_isClosable;
}

void WindowInfo::setIsClosable(bool closable) noexcept
{
    m_isClosable = closable;
}

bool WindowInfo::isFullScreenable() const noexcept
{
    return m_isFullScreenable;
}

void WindowInfo::setIsFullScreenable(bool fullscreenable) noexcept
{
    m_isFullScreenable = fullscreenable;
}

bool WindowInfo::isGroupable() const noexcept
{
    return m_isGroupable;
}

void WindowInfo::setIsGroupable(bool groupable) noexcept
{
    m_isGroupable = groupable;
}

bool WindowInfo::isMaximizable() const
{
    return m_isMaximizable;
}

void WindowInfo::setIsMaximizable(bool maximizable)
{
    m_isMaximizable = maximizable;
}

bool WindowInfo::isMinimizable() const
{
    return m_isMinimizable;
}

void WindowInfo::setIsMinimizable(bool minimizable)
{
    m_isMinimizable = minimizable;
}

bool WindowInfo::isOnDesktop(const QString &desktop) const noexcept
{
    return m_isOnAllDesktops || m_desktops.contains(desktop);
}

bool WindowInfo::isDemandAttention() const noexcept
{
    return m_isDemandAttention;
}

void WindowInfo::setIsDemandAttention(bool demandAttention) noexcept
{
    m_isDemandAttention = demandAttention;
}

bool WindowInfo::isMovable() const noexcept
{
    return m_isMovable;
}

void WindowInfo::setIsMovable(bool movable) noexcept
{
    m_isMovable = movable;
}

bool WindowInfo::isResizable() const noexcept
{
    return m_isResizable;
}

void WindowInfo::setIsResizable(bool resizable) noexcept
{
    m_isResizable = resizable;
}

bool WindowInfo::isShadeable() const noexcept
{
    return m_isShadeable;
}

void WindowInfo::setIsShadeable(bool shadeble) noexcept
{
    m_isShadeable = shadeble;
}

bool WindowInfo::isVirtualDesktopsChangeable() const noexcept
{
    return m_isVirtualDesktopsChangeable;
}

void WindowInfo::setIsVirtualDesktopsChangeable(bool virtualdesktopchangeable) noexcept
{
    m_isVirtualDesktopsChangeable = virtualdesktopchangeable;
}

QIcon WindowInfo::icon() const noexcept
{
    return m_icon;
}

void WindowInfo::setIcon(const QIcon &icon) noexcept
{
    m_icon = icon;
}

QStringList WindowInfo::desktops() const noexcept
{
    return m_desktops;
}

void WindowInfo::setDesktops(const QStringList &desktops) noexcept
{
    m_desktops = desktops;
}

}
#endif // WINDOWINFO_H
