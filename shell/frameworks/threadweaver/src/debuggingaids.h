/* -*- C++ -*-
    This file declares debugging aids for multithreaded applications.

    SPDX-FileCopyrightText: 2004-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    $Id: DebuggingAids.h 30 2005-08-16 16:16:04Z mirko $
*/

// krazy:excludeall=inline

#ifndef DEBUGGINGAIDS_H
#define DEBUGGINGAIDS_H

#include <QtGlobal>

extern "C" {
#include <stdarg.h>
#ifndef Q_OS_WIN
#include <unistd.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
}

#include "threadweaver_export.h"
#include <QMutex>
#include <QString>

namespace ThreadWeaver
{
extern THREADWEAVER_EXPORT bool Debug;
extern THREADWEAVER_EXPORT int DebugLevel;
extern THREADWEAVER_EXPORT QMutex GlobalMutex;

/** Set the debug log level.
@see debug
*/
extern inline void setDebugLevel(bool TWDEBUG, int level);

/** This method prints a text message on the screen, if debugging is
enabled. Otherwise, it does nothing. The message is thread safe,
therefore providing that the messages appear in the order they where
issued by the different threads.
All messages are suppressed when Debug is false. All messages with a
lower importance (higher number) than DebugLevel will be suppressed,
too. Debug level 0 messages will always be printed as long as
Debug is true.
    We use our own debugging method, since debugging threads is a more
    complicated experience than debugging single threaded
    contexts. This might change in future in the way that debug
    prints its messages to another logging facility provided by
    the platform.
Use setDebugLevel () to integrate adapt debug () to your platform.
*/
inline void TWDEBUG(int severity, const char *cformat, ...)
#ifdef __GNUC__
    __attribute__((format(printf, 2, 3)))
#endif
    ;

/** Prints the message to the console if condition is true. */
inline void TWDEBUG(bool condition, int severity, const char *cformat, ...)
#ifdef __GNUC__
    __attribute__((format(printf, 3, 4)))
#endif
    ;

/** PROTECT executes x with GlobalMutex locked.
    Mostly used for debugging, as in P_ASSERT. */
#ifdef PROTECT
#undef PROTECT
#endif

/* clang-format off */
#define PROTECT(x) \
    do { \
        QMutexLocker l(&ThreadWeaver::GlobalMutex); \
        (x); \
    } while (0)
/* clang-format on */

/** P_ASSERT ensures that error messages occur in the correct order. */
#ifdef P_ASSERT
#undef P_ASSERT
#endif

/* clang-format off */
#define P_ASSERT(x) \
    do { \
        QMutexLocker l(&ThreadWeaver::GlobalMutex); \
        Q_ASSERT(x); \
    } while (0)
/* clang-format on */

inline void setDebugLevel(bool debug, int level)
{
    Debug = debug;
    DebugLevel = level;
}

#ifndef QT_NO_DEBUG

#define TWDEBUG(...) ThreadWeaver::threadweaver_debug(__VA_ARGS__)
inline void threadweaver_debug(int severity, const char *cformat, ...)
{
    if (Debug == true && (severity <= DebugLevel || severity == 0)) {
        QString text;

        va_list ap;
        va_start(ap, cformat);
        PROTECT(vprintf(cformat, ap));
        va_end(ap);
    }
}

inline void threadweaver_debug(bool condition, int severity, const char *cformat, ...)
{
    if (condition && Debug == true && (severity <= DebugLevel || severity == 0)) {
        QString text;

        va_list ap;
        va_start(ap, cformat);
        PROTECT(vprintf(cformat, ap));
        va_end(ap);
    }
}
#else
#define TWDEBUG(...)
#endif

// Macros to ensure that mutexes are locked or unlocked:
void THREADWEAVER_EXPORT mutexAssertUnlocked(QMutex *mutex, const char *where);
void THREADWEAVER_EXPORT mutexAssertLocked(QMutex *mutex, const char *where);

#ifndef QT_NO_DEBUG
#define MUTEX_ASSERT_UNLOCKED(x) mutexAssertUnlocked(x, Q_FUNC_INFO)
#define MUTEX_ASSERT_LOCKED(x) mutexAssertLocked(x, Q_FUNC_INFO)
#else
#define MUTEX_ASSERT_UNLOCKED(x)
#define MUTEX_ASSERT_LOCKED(x)
#endif

inline bool invariant()
{
    return true;
}

#define INVARIANT Q_ASSERT_X(invariant(), __FILE__, "class invariant failed");

/* clang-format off */
#define REQUIRE(x) \
    INVARIANT \
    Q_ASSERT_X(x, Q_FUNC_INFO, "unfulfilled requirement " #x);

#define ENSURE(x) \
    INVARIANT \
    Q_ASSERT_X(x, Q_FUNC_INFO, "broken guarantee " #x);
/* clang-format on */

#ifdef QT_NO_DEBUG
#define DEBUGONLY(x)
#else
#define DEBUGONLY(x) x
#endif

}

#endif // DEBUGGINGAIDS_H
