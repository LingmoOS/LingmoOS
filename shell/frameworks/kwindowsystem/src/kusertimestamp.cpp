/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Luboš Luňák <l.lunak@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kusertimestamp.h"

#include "config-kwindowsystem.h"
#include "kwindowsystem.h"

#if KWINDOWSYSTEM_HAVE_X11

#include <private/qtx11extras_p.h>

#include <netwm.h>
#endif

unsigned long KUserTimestamp::userTimestamp()
{
    if (KWindowSystem::isPlatformX11()) {
#if KWINDOWSYSTEM_HAVE_X11
        return QX11Info::appUserTime();
#endif
    }
    return 0;
}

void KUserTimestamp::updateUserTimestamp(unsigned long time)
{
#if KWINDOWSYSTEM_HAVE_X11
    if (!KWindowSystem::isPlatformX11()) {
        return;
    }
    if (time == 0) { // get current X timestamp
        time = QX11Info::getTimestamp();
    }

    if (QX11Info::appUserTime() == 0 || NET::timestampCompare(time, QX11Info::appUserTime()) > 0) { // time > appUserTime
        QX11Info::setAppUserTime(time);
    }
    if (QX11Info::appTime() == 0 || NET::timestampCompare(time, QX11Info::appTime()) > 0) { // time > appTime
        QX11Info::setAppTime(time);
    }
#else
    Q_UNUSED(time)
#endif
}
