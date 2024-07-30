/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 2007 Lubos Lunak <l.lunak@kde.org>
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kwindowsystem_p_x11.h"

#include "kx11extras.h"

void KWindowSystemPrivateX11::activateWindow(QWindow *win, long time)
{
    KX11Extras::activateWindow(win->winId(), time);
}

bool KWindowSystemPrivateX11::showingDesktop()
{
    return KX11Extras::showingDesktop();
}

void KWindowSystemPrivateX11::setShowingDesktop(bool showing)
{
    KX11Extras::setShowingDesktop(showing);
}
