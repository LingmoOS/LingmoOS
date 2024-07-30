/*
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2010, 2012 Michael Pyne <mpyne@kde.org>
    SPDX-FileCopyrightText: 2012 Ralf Jung <ralfjung-e@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kshareddatacache.h"
#include "kcoreaddons_debug.h"
#include "ksdcmapping_p.h"
#include "ksdcmemory_p.h"

#include "kshareddatacache_p.h" // Various auxiliary support code

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QRandomGenerator>
#include <QStandardPaths>

// The per-instance private data, such as map size, whether
// attached or not, pointer to shared memory, etc.
class Q_DECL_HIDDEN KSharedDataCache::Private
{
public:
    Private(const QString &name, unsigned defaultCacheSize, unsigned expectedItemSize)
        : m_cacheName(name)
        , shm(nullptr)
        , m_mapping(nullptr)
        , m_defaultCacheSize(defaultCacheSize)
        , m_expectedItemSize(expectedItemSize)
    {
        createMemoryMapping();
    }

    void createMemoryMapping()
    {
        shm = nullptr;
        m_mapping.reset();

        // 0-sized caches are fairly useless.
        unsigned cacheSize = qMax(m_defaultCacheSize, uint(SharedMemory::MINIMUM_CACHE_SIZE));
        unsigned pageSize = SharedMemory::equivalentPageSize(m_expectedItemSize);

        // Ensure that the cache is sized such that there is a minimum number of
        // pages available. (i.e. a cache consisting of only 1 page is fairly
        // useless and probably crash-prone).
        cacheSize = qMax(pageSize * 256, cacheSize);

        // The m_cacheName is used to find the file to store the cache in.
        const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
        QString cacheName = cacheDir + QLatin1String("/") + m_cacheName + QLatin1String(".kcache");
        QFile file(cacheName);
        QFileInfo fileInfo(file);
        if (!QDir().mkpath(fileInfo.absolutePath())) {
            qCWarning(KCOREADDONS_DEBUG) << "Failed to create cache dir" << fileInfo.absolutePath();
        }

        // The basic idea is to open the file that we want to map into shared
        // memory, and then actually establish the mapping. Once we have mapped the
        // file into shared memory we can close the file handle, the mapping will
        // still be maintained (unless the file is resized to be shorter than
        // expected, which we don't handle yet :-( )

        // size accounts for the overhead over the desired cacheSize
        uint size = SharedMemory::totalSize(cacheSize, pageSize);
        Q_ASSERT(size >= cacheSize);

        // Open the file and resize to some sane value if the file is too small.
        if (file.open(QIODevice::ReadWrite) && (file.size() >= size || (ensureFileAllocated(file.handle(), size) && file.resize(size)))) {
            try {
                m_mapping.reset(new KSDCMapping(&file, size, cacheSize, pageSize));
                shm = m_mapping->m_mapped;
            } catch (KSDCCorrupted) {
                shm = nullptr;
                m_mapping.reset();

                qCWarning(KCOREADDONS_DEBUG) << "Deleting corrupted cache" << cacheName;
                file.remove();
                QFile file(cacheName);
                if (file.open(QIODevice::ReadWrite) && ensureFileAllocated(file.handle(), size) && file.resize(size)) {
                    try {
                        m_mapping.reset(new KSDCMapping(&file, size, cacheSize, pageSize));
                    } catch (KSDCCorrupted) {
                        m_mapping.reset();
                        qCCritical(KCOREADDONS_DEBUG) << "Even a brand-new cache starts off corrupted, something is"
                                                      << "seriously wrong. :-(";
                    }
                }
            }
        }

        if (!m_mapping) {
            m_mapping.reset(new KSDCMapping(nullptr, size, cacheSize, pageSize));
            shm = m_mapping->m_mapped;
        }
    }

    // Called whenever the cache is apparently corrupt (for instance, a timeout trying to
    // lock the cache). In this situation it is safer just to destroy it all and try again.
    void recoverCorruptedCache()
    {
        qCWarning(KCOREADDONS_DEBUG) << "Deleting corrupted cache" << m_cacheName;

        KSharedDataCache::deleteCache(m_cacheName);

        createMemoryMapping();
    }

    class CacheLocker
    {
        mutable Private *d;

        bool cautiousLock()
        {
            int lockCount = 0;

            // Locking can fail due to a timeout. If it happens too often even though
            // we're taking corrective action assume there's some disastrous problem
            // and give up.
            while (!d->m_mapping->lock() && !d->m_mapping->isLockedCacheSafe()) {
                d->recoverCorruptedCache();

                if (!d->m_mapping->isValid()) {
                    qCWarning(KCOREADDONS_DEBUG) << "Lost the connection to shared memory for cache" << d->m_cacheName;
                    return false;
                }

                if (lockCount++ > 4) {
                    qCCritical(KCOREADDONS_DEBUG) << "There is a very serious problem with the KDE data cache" << d->m_cacheName
                                                  << "giving up trying to access cache.";
                    return false;
                }
            }

            return true;
        }

    public:
        CacheLocker(const Private *_d)
            : d(const_cast<Private *>(_d))
        {
            if (Q_UNLIKELY(!d || !cautiousLock())) {
                d = nullptr;
            }
        }

        ~CacheLocker()
        {
            if (d) {
                d->m_mapping->unlock();
            }
        }

        CacheLocker(const CacheLocker &) = delete;
        CacheLocker &operator=(const CacheLocker &) = delete;

        bool failed() const
        {
            return !d;
        }
    };

    QString m_cacheName;
    SharedMemory *shm;
    std::unique_ptr<KSDCMapping> m_mapping;
    uint m_defaultCacheSize;
    uint m_expectedItemSize;
};

KSharedDataCache::KSharedDataCache(const QString &cacheName, unsigned defaultCacheSize, unsigned expectedItemSize)
    : d(nullptr)
{
    try {
        d = new Private(cacheName, defaultCacheSize, expectedItemSize);
    } catch (KSDCCorrupted) {
        qCCritical(KCOREADDONS_DEBUG) << "Failed to initialize KSharedDataCache!";
        d = nullptr; // Just in case
    }
}

KSharedDataCache::~KSharedDataCache()
{
    if (!d) {
        return;
    }

    delete d;
}

bool KSharedDataCache::insert(const QString &key, const QByteArray &data)
{
    try {
        Private::CacheLocker lock(d);
        if (lock.failed()) {
            return false;
        }

        QByteArray encodedKey = key.toUtf8();
        uint keyHash = SharedMemory::generateHash(encodedKey);
        uint position = keyHash % d->shm->indexTableSize();

        // See if we're overwriting an existing entry.
        IndexTableEntry *indices = d->shm->indexTable();

        // In order to avoid the issue of a very long-lived cache having items
        // with a use count of 1 near-permanently, we attempt to artifically
        // reduce the use count of long-lived items when there is high load on
        // the cache. We do this randomly, with a weighting that makes the event
        // impossible if load < 0.5, and guaranteed if load >= 0.96.
        const static double startCullPoint = 0.5l;
        const static double mustCullPoint = 0.96l;

        // cacheAvail is in pages, cacheSize is in bytes.
        double loadFactor = 1.0 - (1.0l * d->shm->cacheAvail * d->shm->cachePageSize() / d->shm->cacheSize);
        bool cullCollisions = false;

        if (Q_UNLIKELY(loadFactor >= mustCullPoint)) {
            cullCollisions = true;
        } else if (loadFactor > startCullPoint) {
            const int tripWireValue = RAND_MAX * (loadFactor - startCullPoint) / (mustCullPoint - startCullPoint);
            if (QRandomGenerator::global()->bounded(RAND_MAX) >= tripWireValue) {
                cullCollisions = true;
            }
        }

        // In case of collisions in the index table (i.e. identical positions), use
        // quadratic chaining to attempt to find an empty slot. The equation we use
        // is:
        // position = (hash + (i + i*i) / 2) % size, where i is the probe number.
        uint probeNumber = 1;
        while (indices[position].useCount > 0 && probeNumber < SharedMemory::MAX_PROBE_COUNT) {
            // If we actually stumbled upon an old version of the key we are
            // overwriting, then use that position, do not skip over it.

            if (Q_UNLIKELY(indices[position].fileNameHash == keyHash)) {
                break;
            }

            // If we are "culling" old entries, see if this one is old and if so
            // reduce its use count. If it reduces to zero then eliminate it and
            // use its old spot.

            if (cullCollisions && (::time(nullptr) - indices[position].lastUsedTime) > 60) {
                indices[position].useCount >>= 1;
                if (indices[position].useCount == 0) {
                    qCDebug(KCOREADDONS_DEBUG) << "Overwriting existing old cached entry due to collision.";
                    d->shm->removeEntry(position); // Remove it first
                    break;
                }
            }

            position = (keyHash + (probeNumber + probeNumber * probeNumber) / 2) % d->shm->indexTableSize();
            probeNumber++;
        }

        if (indices[position].useCount > 0 && indices[position].firstPage >= 0) {
            qCDebug(KCOREADDONS_DEBUG) << "Overwriting existing cached entry due to collision.";
            d->shm->removeEntry(position); // Remove it first
        }

        // Data will be stored as fileNamefoo\0PNGimagedata.....
        // So total size required is the length of the encoded file name + 1
        // for the trailing null, and then the length of the image data.
        uint fileNameLength = 1 + encodedKey.length();
        uint requiredSize = fileNameLength + data.size();
        uint pagesNeeded = SharedMemory::intCeil(requiredSize, d->shm->cachePageSize());
        uint firstPage(-1);

        if (pagesNeeded >= d->shm->pageTableSize()) {
            qCWarning(KCOREADDONS_DEBUG) << key << "is too large to be cached.";
            return false;
        }

        // If the cache has no room, or the fragmentation is too great to find
        // the required number of consecutive free pages, take action.
        if (pagesNeeded > d->shm->cacheAvail || (firstPage = d->shm->findEmptyPages(pagesNeeded)) >= d->shm->pageTableSize()) {
            // If we have enough free space just defragment
            uint freePagesDesired = 3 * qMax(1u, pagesNeeded / 2);

            if (d->shm->cacheAvail > freePagesDesired) {
                // TODO: How the hell long does this actually take on real
                // caches?
                d->shm->defragment();
                firstPage = d->shm->findEmptyPages(pagesNeeded);
            } else {
                // If we already have free pages we don't want to remove a ton
                // extra. However we can't rely on the return value of
                // removeUsedPages giving us a good location since we're not
                // passing in the actual number of pages that we need.
                d->shm->removeUsedPages(qMin(2 * freePagesDesired, d->shm->pageTableSize()) - d->shm->cacheAvail);
                firstPage = d->shm->findEmptyPages(pagesNeeded);
            }

            if (firstPage >= d->shm->pageTableSize() || d->shm->cacheAvail < pagesNeeded) {
                qCCritical(KCOREADDONS_DEBUG) << "Unable to free up memory for" << key;
                return false;
            }
        }

        // Update page table
        PageTableEntry *table = d->shm->pageTable();
        for (uint i = 0; i < pagesNeeded; ++i) {
            table[firstPage + i].index = position;
        }

        // Update index
        indices[position].fileNameHash = keyHash;
        indices[position].totalItemSize = requiredSize;
        indices[position].useCount = 1;
        indices[position].addTime = ::time(nullptr);
        indices[position].lastUsedTime = indices[position].addTime;
        indices[position].firstPage = firstPage;

        // Update cache
        d->shm->cacheAvail -= pagesNeeded;

        // Actually move the data in place
        void *dataPage = d->shm->page(firstPage);
        if (Q_UNLIKELY(!dataPage)) {
            throw KSDCCorrupted();
        }

        // Verify it will all fit
        d->m_mapping->verifyProposedMemoryAccess(dataPage, requiredSize);

        // Cast for byte-sized pointer arithmetic
        uchar *startOfPageData = reinterpret_cast<uchar *>(dataPage);
        ::memcpy(startOfPageData, encodedKey.constData(), fileNameLength);
        ::memcpy(startOfPageData + fileNameLength, data.constData(), data.size());

        return true;
    } catch (KSDCCorrupted) {
        d->recoverCorruptedCache();
        return false;
    }
}

bool KSharedDataCache::find(const QString &key, QByteArray *destination) const
{
    try {
        Private::CacheLocker lock(d);
        if (lock.failed()) {
            return false;
        }

        // Search in the index for our data, hashed by key;
        QByteArray encodedKey = key.toUtf8();
        qint32 entry = d->shm->findNamedEntry(encodedKey);

        if (entry >= 0) {
            const IndexTableEntry *header = &d->shm->indexTable()[entry];
            const void *resultPage = d->shm->page(header->firstPage);
            if (Q_UNLIKELY(!resultPage)) {
                throw KSDCCorrupted();
            }

            d->m_mapping->verifyProposedMemoryAccess(resultPage, header->totalItemSize);

            header->useCount++;
            header->lastUsedTime = ::time(nullptr);

            // Our item is the key followed immediately by the data, so skip
            // past the key.
            const char *cacheData = reinterpret_cast<const char *>(resultPage);
            cacheData += encodedKey.size();
            cacheData++; // Skip trailing null -- now we're pointing to start of data

            if (destination) {
                *destination = QByteArray(cacheData, header->totalItemSize - encodedKey.size() - 1);
            }

            return true;
        }
    } catch (KSDCCorrupted) {
        d->recoverCorruptedCache();
    }

    return false;
}

void KSharedDataCache::clear()
{
    try {
        Private::CacheLocker lock(d);

        if (!lock.failed()) {
            d->shm->clear();
        }
    } catch (KSDCCorrupted) {
        d->recoverCorruptedCache();
    }
}

bool KSharedDataCache::contains(const QString &key) const
{
    try {
        Private::CacheLocker lock(d);
        if (lock.failed()) {
            return false;
        }

        return d->shm->findNamedEntry(key.toUtf8()) >= 0;
    } catch (KSDCCorrupted) {
        d->recoverCorruptedCache();
        return false;
    }
}

void KSharedDataCache::deleteCache(const QString &cacheName)
{
    QString cachePath = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1String("/") + cacheName + QLatin1String(".kcache");

    // Note that it is important to simply unlink the file, and not truncate it
    // smaller first to avoid SIGBUS errors and similar with shared memory
    // attached to the underlying inode.
    qCDebug(KCOREADDONS_DEBUG) << "Removing cache at" << cachePath;
    QFile::remove(cachePath);
}

unsigned KSharedDataCache::totalSize() const
{
    try {
        Private::CacheLocker lock(d);
        if (lock.failed()) {
            return 0u;
        }

        return d->shm->cacheSize;
    } catch (KSDCCorrupted) {
        d->recoverCorruptedCache();
        return 0u;
    }
}

unsigned KSharedDataCache::freeSize() const
{
    try {
        Private::CacheLocker lock(d);
        if (lock.failed()) {
            return 0u;
        }

        return d->shm->cacheAvail * d->shm->cachePageSize();
    } catch (KSDCCorrupted) {
        d->recoverCorruptedCache();
        return 0u;
    }
}

KSharedDataCache::EvictionPolicy KSharedDataCache::evictionPolicy() const
{
    if (d && d->shm) {
        return static_cast<EvictionPolicy>(d->shm->evictionPolicy.fetchAndAddAcquire(0));
    }

    return NoEvictionPreference;
}

void KSharedDataCache::setEvictionPolicy(EvictionPolicy newPolicy)
{
    if (d && d->shm) {
        d->shm->evictionPolicy.fetchAndStoreRelease(static_cast<int>(newPolicy));
    }
}

unsigned KSharedDataCache::timestamp() const
{
    if (d && d->shm) {
        return static_cast<unsigned>(d->shm->cacheTimestamp.fetchAndAddAcquire(0));
    }

    return 0;
}

void KSharedDataCache::setTimestamp(unsigned newTimestamp)
{
    if (d && d->shm) {
        d->shm->cacheTimestamp.fetchAndStoreRelease(static_cast<int>(newTimestamp));
    }
}
