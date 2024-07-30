/*
 *    This file is part of the KDE project.
 *
 *    SPDX-License-Identifier: LGPL-2.0-only
 */

#ifndef KSDCMAPPING_P_H
#define KSDCMAPPING_P_H

#include "kcoreaddons_debug.h"
#include "ksdcmemory_p.h"
#include "kshareddatacache.h"

#include <config-caching.h> // HAVE_SYS_MMAN_H

#include <QFile>
#include <QtGlobal>
#include <qplatformdefs.h>

#include <sys/resource.h>

#if defined(_POSIX_MAPPED_FILES) && ((_POSIX_MAPPED_FILES == 0) || (_POSIX_MAPPED_FILES >= 200112L))
#define KSDC_MAPPED_FILES_SUPPORTED 1
#endif

#if defined(_POSIX_SYNCHRONIZED_IO) && ((_POSIX_SYNCHRONIZED_IO == 0) || (_POSIX_SYNCHRONIZED_IO >= 200112L))
#define KSDC_SYNCHRONIZED_IO_SUPPORTED 1
#endif

// msync(2) requires both MAPPED_FILES and SYNCHRONIZED_IO POSIX options
#if defined(KSDC_MAPPED_FILES_SUPPORTED) && defined(KSDC_SYNCHRONIZED_IO_SUPPORTED)
#define KSDC_MSYNC_SUPPORTED
#endif

// BSD/Mac OS X compat
#if HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif

class Q_DECL_HIDDEN KSDCMapping
{
public:
    KSDCMapping(const QFile *file, const uint size, const uint cacheSize, const uint pageSize)
        : m_mapped(nullptr)
        , m_lock()
        , m_mapSize(size)
        , m_expectedType(LOCKTYPE_INVALID)
    {
        mapSharedMemory(file, size, cacheSize, pageSize);
    }

    ~KSDCMapping()
    {
        detachFromSharedMemory(true);
    }

    bool isValid()
    {
        return !!m_mapped;
    }

    bool lock() const
    {
        if (Q_UNLIKELY(!m_mapped)) {
            return false;
        }
        if (Q_LIKELY(m_mapped->shmLock.type == m_expectedType)) {
            return m_lock->lock();
        }

        // Wrong type --> corrupt!
        throw KSDCCorrupted("Invalid cache lock type!");
    }

    void unlock() const
    {
        if (Q_LIKELY(m_lock)) {
            m_lock->unlock();
        }
    }

    // This should be called for any memory access to shared memory. This
    // function will verify that the bytes [base, base+accessLength) are
    // actually mapped to m_mapped. The cache itself may have incorrect cache
    // page sizes, incorrect cache size, etc. so this function should be called
    // despite the cache data indicating it should be safe.
    //
    // If the access is /not/ safe then a KSDCCorrupted exception will be
    // thrown, so be ready to catch that.
    void verifyProposedMemoryAccess(const void *base, unsigned accessLength) const
    {
        quintptr startOfAccess = reinterpret_cast<quintptr>(base);
        quintptr startOfShm = reinterpret_cast<quintptr>(m_mapped);

        if (Q_UNLIKELY(startOfAccess < startOfShm)) {
            throw KSDCCorrupted();
        }

        quintptr endOfShm = startOfShm + m_mapSize;
        quintptr endOfAccess = startOfAccess + accessLength;

        // Check for unsigned integer wraparound, and then
        // bounds access
        if (Q_UNLIKELY((endOfShm < startOfShm) || (endOfAccess < startOfAccess) || (endOfAccess > endOfShm))) {
            throw KSDCCorrupted();
        }
    }

    // Runs a quick battery of tests on an already-locked cache and returns
    // false as soon as a sanity check fails. The cache remains locked in this
    // situation.
    bool isLockedCacheSafe() const
    {
        if (Q_UNLIKELY(!m_mapped)) {
            return false;
        }

        // Note that cachePageSize() itself runs a check that can throw.
        uint testSize = SharedMemory::totalSize(m_mapped->cacheSize, m_mapped->cachePageSize());

        if (Q_UNLIKELY(m_mapSize != testSize)) {
            return false;
        }
        if (Q_UNLIKELY(m_mapped->version != SharedMemory::PIXMAP_CACHE_VERSION)) {
            return false;
        }
        switch (m_mapped->evictionPolicy.loadRelaxed()) {
        case KSharedDataCache::NoEvictionPreference: // fallthrough
        case KSharedDataCache::EvictLeastRecentlyUsed: // fallthrough
        case KSharedDataCache::EvictLeastOftenUsed: // fallthrough
        case KSharedDataCache::EvictOldest:
            break;
        default:
            return false;
        }

        return true;
    }

    SharedMemory *m_mapped;

private:
    // Put the cache in a condition to be able to call mapSharedMemory() by
    // completely detaching from shared memory (such as to respond to an
    // unrecoverable error).
    // m_mapSize must already be set to the amount of memory mapped to m_mapped.
    void detachFromSharedMemory(const bool flush = false)
    {
        // The lock holds a reference into shared memory, so this must be
        // cleared before m_mapped is removed.
        m_lock.reset();

        // Note that there is no other actions required to separate from the
        // shared memory segment, simply unmapping is enough. This makes things
        // *much* easier so I'd recommend maintaining this ideal.
        if (m_mapped) {
#ifdef KSDC_MSYNC_SUPPORTED
            if (flush) {
                ::msync(m_mapped, m_mapSize, MS_INVALIDATE | MS_ASYNC);
            }
#endif
            ::munmap(m_mapped, m_mapSize);
            if (0 != ::munmap(m_mapped, m_mapSize)) {
                qCCritical(KCOREADDONS_DEBUG) << "Unable to unmap shared memory segment" << static_cast<void *>(m_mapped) << ":" << ::strerror(errno);
            }
        }

        // Do not delete m_mapped, it was never constructed, it's just an alias.
        m_mapped = nullptr;
        m_mapSize = 0;
    }

    // This function does a lot of the important work, attempting to connect to shared
    // memory, a private anonymous mapping if that fails, and failing that, nothing (but
    // the cache remains "valid", we just don't actually do anything).
    void mapSharedMemory(const QFile *file, uint size, uint cacheSize, uint pageSize)
    {
        void *mapAddress = MAP_FAILED;

        if (file) {
            // Use mmap directly instead of QFile::map since the QFile (and its
            // shared mapping) will disappear unless we hang onto the QFile for no
            // reason (see the note below, we don't care about the file per se...)
            mapAddress = QT_MMAP(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, file->handle(), 0);

            // So... it is possible that someone else has mapped this cache already
            // with a larger size. If that's the case we need to at least match
            // the size to be able to access every entry, so fixup the mapping.
            if (mapAddress != MAP_FAILED) {
                // Successful mmap doesn't actually mean that whole range is readable so ensure it is
                struct rlimit memlock;
                if (getrlimit(RLIMIT_MEMLOCK, &memlock) == 0 && memlock.rlim_cur >= 2) {
                    // Half of limit in case something else has already locked some mem
                    uint lockSize = qMin(memlock.rlim_cur / 2, (rlim_t)size);
                    // Note that lockSize might be less than what we need to mmap
                    // and so this doesn't guarantee that later parts will be readable
                    // but that's fine, at least we know we will succeed here
                    if (mlock(mapAddress, lockSize)) {
                        throw KSDCCorrupted(QLatin1String("Cache is inaccessible ") + file->fileName());
                    }
                    if (munlock(mapAddress, lockSize) != 0) {
                        qCDebug(KCOREADDONS_DEBUG) << "Failed to munlock!";
                    }
                } else {
                    qCWarning(KCOREADDONS_DEBUG) << "Failed to get RLIMIT_MEMLOCK!";
                }

                SharedMemory *mapped = reinterpret_cast<SharedMemory *>(mapAddress);

                // First make sure that the version of the cache on disk is
                // valid.  We also need to check that version != 0 to
                // disambiguate against an uninitialized cache.
                if (mapped->version != SharedMemory::PIXMAP_CACHE_VERSION && mapped->version > 0) {
                    detachFromSharedMemory(false);
                    throw KSDCCorrupted(QLatin1String("Wrong version of cache ") + file->fileName());
                } else if (mapped->cacheSize > cacheSize) {
                    // This order is very important. We must save the cache size
                    // before we remove the mapping, but unmap before overwriting
                    // the previous mapping size...
                    auto actualCacheSize = mapped->cacheSize;
                    auto actualPageSize = mapped->cachePageSize();
                    ::munmap(mapAddress, size);
                    size = SharedMemory::totalSize(cacheSize, pageSize);
                    mapAddress = QT_MMAP(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, file->handle(), 0);
                    if (mapAddress != MAP_FAILED) {
                        cacheSize = actualCacheSize;
                        pageSize = actualPageSize;
                    }
                }
            }
        }

        // We could be here without the mapping established if:
        // 1) Process-shared synchronization is not supported, either at compile or run time,
        // 2) Unable to open the required file.
        // 3) Unable to resize the file to be large enough.
        // 4) Establishing the mapping failed.
        // 5) The mapping succeeded, but the size was wrong and we were unable to map when
        //    we tried again.
        // 6) The incorrect version of the cache was detected.
        // 7) The file could be created, but posix_fallocate failed to commit it fully to disk.
        // In any of these cases, attempt to fallback to the
        // better-supported anonymous page style of mmap.
        // NOTE: We never use the on-disk representation independently of the
        // shared memory. If we don't get shared memory the disk info is ignored,
        // if we do get shared memory we never look at disk again.
        if (!file || mapAddress == MAP_FAILED) {
            qCWarning(KCOREADDONS_DEBUG) << "Couldn't establish file backed memory mapping, will fallback"
                                         << "to anonymous memory";
            mapAddress = QT_MMAP(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        }

        // Well now we're really hosed. We can still work, but we can't even cache
        // data.
        if (mapAddress == MAP_FAILED) {
            qCCritical(KCOREADDONS_DEBUG) << "Unable to allocate shared memory segment for shared data cache" << file->fileName() << "of size" << m_mapSize;
            m_mapped = nullptr;
            m_mapSize = 0;
            return;
        }

        m_mapSize = size;

        // We never actually construct m_mapped, but we assign it the same address as the
        // shared memory we just mapped, so effectively m_mapped is now a SharedMemory that
        // happens to be located at mapAddress.
        m_mapped = reinterpret_cast<SharedMemory *>(mapAddress);

        // If we were first to create this memory map, all data will be 0.
        // Therefore if ready == 0 we're not initialized.  A fully initialized
        // header will have ready == 2.  Why?
        // Because 0 means "safe to initialize"
        //         1 means "in progress of initing"
        //         2 means "ready"
        uint usecSleepTime = 8; // Start by sleeping for 8 microseconds
        while (m_mapped->ready.loadRelaxed() != 2) {
            if (Q_UNLIKELY(usecSleepTime >= (1 << 21))) {
                // Didn't acquire within ~8 seconds?  Assume an issue exists
                detachFromSharedMemory(false);
                throw KSDCCorrupted("Unable to acquire shared lock, is the cache corrupt?");
            }

            if (m_mapped->ready.testAndSetAcquire(0, 1)) {
                if (!m_mapped->performInitialSetup(cacheSize, pageSize)) {
                    qCCritical(KCOREADDONS_DEBUG) << "Unable to perform initial setup, this system probably "
                                                     "does not really support process-shared pthreads or "
                                                     "semaphores, even though it claims otherwise.";

                    detachFromSharedMemory(false);
                    return;
                }
            } else {
                usleep(usecSleepTime); // spin

                // Exponential fallback as in Ethernet and similar collision resolution methods
                usecSleepTime *= 2;
            }
        }

        m_expectedType = m_mapped->shmLock.type;
        m_lock.reset(createLockFromId(m_expectedType, m_mapped->shmLock));
        bool isProcessSharingSupported = false;

        if (!m_lock->initialize(isProcessSharingSupported)) {
            qCCritical(KCOREADDONS_DEBUG) << "Unable to setup shared cache lock, although it worked when created.";
            detachFromSharedMemory(false);
            return;
        }
    }

    std::unique_ptr<KSDCLock> m_lock;
    uint m_mapSize;
    SharedLockId m_expectedType;
};

#endif /* KSDCMEMORY_P_H */
