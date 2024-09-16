/* This file is part of the KDE libraries
    Copyright (c) 2003 Luboš Luňák <l.lunak@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kusertimestamp.h"

#include "config-kwindowsystem.h"
#include "kwindowsystem.h"
#include <QGuiApplication>

#if KWINDOWSYSTEM_HAVE_X11
#include <netwm.h>
#include <QX11Info>
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

    if (QX11Info::appUserTime() == 0
            || NET::timestampCompare(time, QX11Info::appUserTime()) > 0) { // time > appUserTime
        QX11Info::setAppUserTime(time);
    }
    if (QX11Info::appTime() == 0
            || NET::timestampCompare(time, QX11Info::appTime()) > 0) { // time > appTime
        QX11Info::setAppTime(time);
    }
#else
    Q_UNUSED(time)
#endif
}

