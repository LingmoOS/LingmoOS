// SPDX-License-Identifier: GPL-3.0-or-later
//
// Qt 6 replacement for the subset of QX11Info used by Lingmo core services.
#pragma once

#include <QGuiApplication>
#include <QString>

#include <X11/Xlib.h>
// Undefine X11 macros that conflict with Qt6 enum values
#undef None
#undef Bool
#undef Status
#undef Success
#undef Above
#undef Always
#undef Below
#undef Bottom
#undef Top
#undef Unsorted
#undef Normal
#undef Index

#include <xcb/xcb.h>

namespace Lingmo::X11
{
inline QNativeInterface::QX11Application *nativeInterface()
{
    if (!qGuiApp || qGuiApp->platformName() != QLatin1String("xcb")) {
        return nullptr;
    }

    return qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
}

inline bool isPlatformX11()
{
    return nativeInterface() != nullptr;
}

inline Display *display()
{
    const auto *interface = nativeInterface();
    return interface ? interface->display() : nullptr;
}

inline xcb_connection_t *connection()
{
    const auto *interface = nativeInterface();
    return interface ? interface->connection() : nullptr;
}

inline xcb_window_t appRootWindow()
{
    if (const auto *x11Display = display()) {
        return DefaultRootWindow(x11Display);
    }

    return XCB_NONE;
}

inline int appScreen()
{
    if (const auto *x11Display = display()) {
        return DefaultScreen(x11Display);
    }

    return 0;
}

// X11 protocol accepts CurrentTime (0) when a server timestamp is not needed.
inline xcb_timestamp_t getTimestamp()
{
    return XCB_CURRENT_TIME;
}

// Kept only for source compatibility with old QX11Info callers. Lingmo does not
// persist an application timestamp in Qt 6.
inline xcb_timestamp_t appTime()
{
    return XCB_CURRENT_TIME;
}

inline void setAppTime(xcb_timestamp_t)
{
}
}
