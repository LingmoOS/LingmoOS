/*
 *    This file is part of the KDE project.
 *
 *    SPDX-FileCopyrightText: 2010 Michael Pyne <mpyne@kde.org>
 *    SPDX-License-Identifier: LGPL-2.0-only
 */

#ifndef KSDCLOCK_P_H
#define KSDCLOCK_P_H

#include <qbasicatomic.h>

#include <sched.h> // sched_yield
#include <unistd.h> // Check for sched_yield

// Mac OS X, for all its POSIX compliance, does not support timeouts on its
// mutexes, which is kind of a disaster for cross-process support. However
// synchronization primitives still work, they just might hang if the cache is
// corrupted, so keep going.
#if defined(_POSIX_TIMEOUTS) && ((_POSIX_TIMEOUTS == 0) || (_POSIX_TIMEOUTS >= 200112L))
#define KSDC_TIMEOUTS_SUPPORTED 1
#endif

#if defined(__GNUC__) && !defined(KSDC_TIMEOUTS_SUPPORTED)
#warning "No support for POSIX timeouts -- application hangs are possible if the cache is corrupt"
#endif

#if defined(_POSIX_THREAD_PROCESS_SHARED) && ((_POSIX_THREAD_PROCESS_SHARED == 0) || (_POSIX_THREAD_PROCESS_SHARED >= 200112L)) && !defined(__APPLE__)
#include <pthread.h>
#define KSDC_THREAD_PROCESS_SHARED_SUPPORTED 1
#endif

#if defined(_POSIX_SEMAPHORES) && ((_POSIX_SEMAPHORES == 0) || (_POSIX_SEMAPHORES >= 200112L))
#include <semaphore.h>
#define KSDC_SEMAPHORES_SUPPORTED 1
#endif

#if defined(__GNUC__) && !defined(KSDC_SEMAPHORES_SUPPORTED) && !defined(KSDC_THREAD_PROCESS_SHARED_SUPPORTED)
#warning "No system support claimed for process-shared synchronization, KSharedDataCache will be mostly useless."
#endif

/**
 * This class defines an interface used by KSharedDataCache::Private to offload
 * proper locking and unlocking depending on what the platform supports at
 * runtime and compile-time.
 */
class KSDCLock
{
public:
    virtual ~KSDCLock()
    {
    }

    // Return value indicates if the mutex was properly initialized (including
    // threads-only as a fallback).
    virtual bool initialize(bool &processSharingSupported)
    {
        processSharingSupported = false;
        return false;
    }

    virtual bool lock()
    {
        return false;
    }

    virtual void unlock()
    {
    }
};

/**
 * This is a very basic lock that should work on any system where GCC atomic
 * intrinsics are supported. It can waste CPU so better primitives should be
 * used if available on the system.
 */
class simpleSpinLock : public KSDCLock
{
public:
    simpleSpinLock(QBasicAtomicInt &spinlock)
        : m_spinlock(spinlock)
    {
    }

    bool initialize(bool &processSharingSupported) override
    {
        // Clear the spinlock
        m_spinlock.storeRelaxed(0);
        processSharingSupported = true;
        return true;
    }

    bool lock() override
    {
        // Spin a few times attempting to gain the lock, as upper-level code won't
        // attempt again without assuming the cache is corrupt.
        for (unsigned i = 50; i > 0; --i) {
            if (m_spinlock.testAndSetAcquire(0, 1)) {
                return true;
            }

            // Don't steal the processor and starve the thread we're waiting
            // on.
            loopSpinPause();
        }

        return false;
    }

    void unlock() override
    {
        m_spinlock.testAndSetRelease(1, 0);
    }

private:
#ifdef Q_CC_GNU
    __attribute__((always_inline,
                   gnu_inline
#if !defined(Q_CC_INTEL) && !defined(Q_CC_CLANG)
                   ,
                   artificial
#endif
                   ))
#endif
    static inline void
    loopSpinPause()
    {
// TODO: Spinning might be better in multi-core systems... but that means
// figuring how to find numbers of CPUs in a cross-platform way.
#ifdef _POSIX_PRIORITY_SCHEDULING
        sched_yield();
#else
        // Sleep for shortest possible time (nanosleep should round-up).
        struct timespec wait_time = {0 /* sec */, 100 /* ns */};
        ::nanosleep(&wait_time, static_cast<struct timespec *>(0));
#endif
    }

    QBasicAtomicInt &m_spinlock;
};

#ifdef KSDC_THREAD_PROCESS_SHARED_SUPPORTED
class pthreadLock : public KSDCLock
{
public:
    pthreadLock(pthread_mutex_t &mutex)
        : m_mutex(mutex)
    {
    }

    bool initialize(bool &processSharingSupported) override
    {
        // Setup process-sharing.
        pthread_mutexattr_t mutexAttr;
        processSharingSupported = false;

        // Initialize attributes, enable process-shared primitives, and setup
        // the mutex.
        if (::sysconf(_SC_THREAD_PROCESS_SHARED) >= 200112L && pthread_mutexattr_init(&mutexAttr) == 0) {
            if (pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED) == 0 && pthread_mutex_init(&m_mutex, &mutexAttr) == 0) {
                processSharingSupported = true;
            }
            pthread_mutexattr_destroy(&mutexAttr);
        }

        // Attempt to setup for thread-only synchronization.
        if (!processSharingSupported && pthread_mutex_init(&m_mutex, nullptr) != 0) {
            return false;
        }

        return true;
    }

    bool lock() override
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    void unlock() override
    {
        pthread_mutex_unlock(&m_mutex);
    }

protected:
    pthread_mutex_t &m_mutex;
};
#endif // KSDC_THREAD_PROCESS_SHARED_SUPPORTED

#if defined(KSDC_THREAD_PROCESS_SHARED_SUPPORTED) && defined(KSDC_TIMEOUTS_SUPPORTED)
class pthreadTimedLock : public pthreadLock
{
public:
    pthreadTimedLock(pthread_mutex_t &mutex)
        : pthreadLock(mutex)
    {
    }

    bool lock() override
    {
        struct timespec timeout;

        // Long timeout, but if we fail to meet this timeout it's probably a cache
        // corruption (and if we take 8 seconds then it should be much much quicker
        // the next time anyways since we'd be paged back in from disk)
        timeout.tv_sec = 10 + ::time(nullptr); // Absolute time, so 10 seconds from now
        timeout.tv_nsec = 0;

        return pthread_mutex_timedlock(&m_mutex, &timeout) == 0;
    }
};
#endif // defined(KSDC_THREAD_PROCESS_SHARED_SUPPORTED) && defined(KSDC_TIMEOUTS_SUPPORTED)

#ifdef KSDC_SEMAPHORES_SUPPORTED
class semaphoreLock : public KSDCLock
{
public:
    semaphoreLock(sem_t &semaphore)
        : m_semaphore(semaphore)
    {
    }

    bool initialize(bool &processSharingSupported) override
    {
        processSharingSupported = false;
        if (::sysconf(_SC_SEMAPHORES) < 200112L) {
            return false;
        }

        // sem_init sets up process-sharing for us.
        if (sem_init(&m_semaphore, 1, 1) == 0) {
            processSharingSupported = true;
        }
        // If not successful try falling back to thread-shared.
        else if (sem_init(&m_semaphore, 0, 1) != 0) {
            return false;
        }

        return true;
    }

    bool lock() override
    {
        return sem_wait(&m_semaphore) == 0;
    }

    void unlock() override
    {
        sem_post(&m_semaphore);
    }

protected:
    sem_t &m_semaphore;
};
#endif // KSDC_SEMAPHORES_SUPPORTED

#if defined(KSDC_SEMAPHORES_SUPPORTED) && defined(KSDC_TIMEOUTS_SUPPORTED)
class semaphoreTimedLock : public semaphoreLock
{
public:
    semaphoreTimedLock(sem_t &semaphore)
        : semaphoreLock(semaphore)
    {
    }

    bool lock() override
    {
        struct timespec timeout;

        // Long timeout, but if we fail to meet this timeout it's probably a cache
        // corruption (and if we take 8 seconds then it should be much much quicker
        // the next time anyways since we'd be paged back in from disk)
        timeout.tv_sec = 10 + ::time(nullptr); // Absolute time, so 10 seconds from now
        timeout.tv_nsec = 0;

        return sem_timedwait(&m_semaphore, &timeout) == 0;
    }
};
#endif // defined(KSDC_SEMAPHORES_SUPPORTED) && defined(KSDC_TIMEOUTS_SUPPORTED)

// This enum controls the type of the locking used for the cache to allow
// for as much portability as possible. This value will be stored in the
// cache and used by multiple processes, therefore you should consider this
// a versioned field, do not re-arrange.
enum SharedLockId {
    LOCKTYPE_INVALID = 0,
    LOCKTYPE_MUTEX = 1, // pthread_mutex
    LOCKTYPE_SEMAPHORE = 2, // sem_t
    LOCKTYPE_SPINLOCK = 3, // atomic int in shared memory
};

// This type is a union of all possible lock types, with a SharedLockId used
// to choose which one is actually in use.
struct SharedLock {
    union {
#if defined(KSDC_THREAD_PROCESS_SHARED_SUPPORTED)
        pthread_mutex_t mutex;
#endif
#if defined(KSDC_SEMAPHORES_SUPPORTED)
        sem_t semaphore;
#endif
        QBasicAtomicInt spinlock;

        // It would be highly unfortunate if a simple glibc upgrade or kernel
        // addition caused this structure to change size when an existing
        // lock was thought present, so reserve enough size to cover any
        // reasonable locking structure
        char unused[64];
    };

    SharedLockId type;
};

/**
 * This is a method to determine the best lock type to use for a
 * shared cache, based on local support. An identifier to the appropriate
 * SharedLockId is returned, which can be passed to createLockFromId().
 */
SharedLockId findBestSharedLock();

KSDCLock *createLockFromId(SharedLockId id, SharedLock &lock);

#endif /* KSDCLOCK_P_H */
