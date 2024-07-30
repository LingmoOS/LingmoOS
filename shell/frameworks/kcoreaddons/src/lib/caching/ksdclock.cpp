/*
 *    This file is part of the KDE project.
 *
 *    SPDX-FileCopyrightText: 2010 Michael Pyne <mpyne@kde.org>
 *    SPDX-License-Identifier: LGPL-2.0-only
 */

#include "ksdclock_p.h"

#include "kcoreaddons_debug.h"

#include <memory>

/**
 * This is a method to determine the best lock type to use for a
 * shared cache, based on local support. An identifier to the appropriate
 * SharedLockId is returned, which can be passed to createLockFromId().
 */
SharedLockId findBestSharedLock()
{
    // We would prefer a process-shared capability that also supports
    // timeouts. Failing that, process-shared is preferred over timeout
    // support. Failing that we'll go thread-local
    bool timeoutsSupported = false;
    bool pthreadsProcessShared = false;
    bool semaphoresProcessShared = false;

#ifdef KSDC_TIMEOUTS_SUPPORTED
    timeoutsSupported = ::sysconf(_SC_TIMEOUTS) >= 200112L;
#endif

// Now that we've queried timeouts, try actually creating real locks and
// seeing if there's issues with that.
#ifdef KSDC_THREAD_PROCESS_SHARED_SUPPORTED
    {
        pthread_mutex_t tempMutex;
        std::unique_ptr<KSDCLock> tempLock;
        if (timeoutsSupported) {
#ifdef KSDC_TIMEOUTS_SUPPORTED
            tempLock = std::make_unique<pthreadTimedLock>(tempMutex);
#endif
        } else {
            tempLock = std::make_unique<pthreadLock>(tempMutex);
        }

        tempLock->initialize(pthreadsProcessShared);
    }
#endif // KSDC_THREAD_PROCESS_SHARED_SUPPORTED

    // Our first choice is pthread_mutex_t for compatibility.
    if (timeoutsSupported && pthreadsProcessShared) {
        return LOCKTYPE_MUTEX;
    }

#ifdef KSDC_SEMAPHORES_SUPPORTED
    {
        sem_t tempSemaphore;
        std::unique_ptr<KSDCLock> tempLock;
        if (timeoutsSupported) {
            tempLock = std::make_unique<semaphoreTimedLock>(tempSemaphore);
        } else {
            tempLock = std::make_unique<semaphoreLock>(tempSemaphore);
        }

        tempLock->initialize(semaphoresProcessShared);
    }
#endif // KSDC_SEMAPHORES_SUPPORTED

    if (timeoutsSupported && semaphoresProcessShared) {
        return LOCKTYPE_SEMAPHORE;
    } else if (pthreadsProcessShared) {
        return LOCKTYPE_MUTEX;
    } else if (semaphoresProcessShared) {
        return LOCKTYPE_SEMAPHORE;
    }

    // Fallback to a dumb-simple but possibly-CPU-wasteful solution.
    return LOCKTYPE_SPINLOCK;
}

KSDCLock *createLockFromId(SharedLockId id, SharedLock &lock)
{
    switch (id) {
#ifdef KSDC_THREAD_PROCESS_SHARED_SUPPORTED
    case LOCKTYPE_MUTEX:
#ifdef KSDC_TIMEOUTS_SUPPORTED
        if (::sysconf(_SC_TIMEOUTS) >= 200112L) {
            return new pthreadTimedLock(lock.mutex);
        }
#endif
        return new pthreadLock(lock.mutex);

        break;
#endif // KSDC_THREAD_PROCESS_SHARED_SUPPORTED

#ifdef KSDC_SEMAPHORES_SUPPORTED
    case LOCKTYPE_SEMAPHORE:
#ifdef KSDC_TIMEOUTS_SUPPORTED
        if (::sysconf(_SC_SEMAPHORES) >= 200112L) {
            return new semaphoreTimedLock(lock.semaphore);
        }
#endif
        return new semaphoreLock(lock.semaphore);

        break;
#endif // KSDC_SEMAPHORES_SUPPORTED

    case LOCKTYPE_SPINLOCK:
        return new simpleSpinLock(lock.spinlock);
        break;

    default:
        qCCritical(KCOREADDONS_DEBUG) << "Creating shell of a lock!";
        return new KSDCLock;
    }
}
