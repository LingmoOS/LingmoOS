/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#if defined __linux__
#include "processes_linux_p.cpp"
#elif defined __FreeBSD__ || defined __FreeBSD_kernel__
#include "processes_freebsd_p.cpp"
#elif defined __DragonFly__
#include "processes_dragonfly_p.cpp"
#elif defined __OpenBSD__
#include "processes_openbsd_p.cpp"
#elif defined __NetBSD__
#include "processes_netbsd_p.cpp"
#elif defined __GNU__ || defined __APPLE__
#include "processes_gnu_p.cpp"
#else
// Use Qt's OS detection
#include <qglobal.h>
#ifdef Q_OS_SOLARIS
#include "processes_solaris_p.cpp"
#endif
#endif
