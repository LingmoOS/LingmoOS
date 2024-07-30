/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include <X11/Xlib.h>

#include <QGuiApplication>
#include <QString>

using namespace Qt::StringLiterals;

namespace X11Info
{
[[nodiscard]] inline bool isPlatformX11()
{
    return QGuiApplication::platformName() == "xcb"_L1;
}

[[nodiscard]] inline auto display()
{
    return qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->display();
}

[[nodiscard]] inline auto connection()
{
    return qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->connection();
}

[[nodiscard]] inline Window appRootWindow()
{
    return DefaultRootWindow(display());
}
}
