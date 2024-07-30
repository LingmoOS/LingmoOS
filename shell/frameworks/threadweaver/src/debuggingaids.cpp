/* -*- C++ -*-
    This file implements debugging aids for multithreaded applications.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: DebuggingAids.cpp 20 2005-08-08 21:02:51Z mirko $
*/

#include "debuggingaids.h"

#include <threadweaver_export.h>

/** A global mutex for the ThreadWeaver objects.
    Generally, you should not use it in your own code. */
THREADWEAVER_EXPORT QMutex ThreadWeaver::GlobalMutex;
THREADWEAVER_EXPORT bool ThreadWeaver::Debug = true;
THREADWEAVER_EXPORT int ThreadWeaver::DebugLevel = 1;

namespace ThreadWeaver
{
void mutexAssertUnlocked(QMutex *mutex, const char *where)
{
    if (mutex->tryLock()) {
        mutex->unlock();
    } else {
        Q_ASSERT_X(false, where, "mutexAssertUnlocked: mutex was locked!");
    }
}

void mutexAssertLocked(QMutex *mutex, const char *where)
{
    if (mutex->tryLock()) {
        mutex->unlock();
        Q_ASSERT_X(false, where, "mutexAssertUnlocked: mutex was locked!");
    }
}

}
