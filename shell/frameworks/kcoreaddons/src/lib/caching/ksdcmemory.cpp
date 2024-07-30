/*
 *    This file is part of the KDE project.
 *
 *    SPDX-FileCopyrightText: 2010 Michael Pyne <mpyne@kde.org>
 *    SPDX-License-Identifier: LGPL-2.0-only
 *
 *    This library includes "MurmurHash" code from Austin Appleby, which is
 *    placed in the public domain. See http://sites.google.com/site/murmurhash/
 */

#include "kcoreaddons_debug.h"

#include "ksdcmemory_p.h"

#include <QByteArray>

//-----------------------------------------------------------------------------
// MurmurHashAligned, by Austin Appleby
// (Released to the public domain, or licensed under the MIT license where
// software may not be released to the public domain. See
// http://sites.google.com/site/murmurhash/)

// Same algorithm as MurmurHash, but only does aligned reads - should be safer
// on certain platforms.
static unsigned int MurmurHashAligned(const void *key, int len, unsigned int seed)
{
    const unsigned int m = 0xc6a4a793;
    const int r = 16;

    const unsigned char *data = reinterpret_cast<const unsigned char *>(key);

    unsigned int h = seed ^ (len * m);

    int align = reinterpret_cast<quintptr>(data) & 3;

    if (align && len >= 4) {
        // Pre-load the temp registers

        unsigned int t = 0;
        unsigned int d = 0;

        switch (align) {
        case 1:
            t |= data[2] << 16;
            Q_FALLTHROUGH();
        case 2:
            t |= data[1] << 8;
            Q_FALLTHROUGH();
        case 3:
            t |= data[0];
        }

        t <<= (8 * align);

        data += 4 - align;
        len -= 4 - align;

        int sl = 8 * (4 - align);
        int sr = 8 * align;

        // Mix

        while (len >= 4) {
            d = *reinterpret_cast<const unsigned int *>(data);
            t = (t >> sr) | (d << sl);
            h += t;
            h *= m;
            h ^= h >> r;
            t = d;

            data += 4;
            len -= 4;
        }

        // Handle leftover data in temp registers

        int pack = len < align ? len : align;

        d = 0;

        switch (pack) {
        case 3:
            d |= data[2] << 16;
            Q_FALLTHROUGH();
        case 2:
            d |= data[1] << 8;
            Q_FALLTHROUGH();
        case 1:
            d |= data[0];
            Q_FALLTHROUGH();
        case 0:
            h += (t >> sr) | (d << sl);
            h *= m;
            h ^= h >> r;
        }

        data += pack;
        len -= pack;
    } else {
        while (len >= 4) {
            h += *reinterpret_cast<const unsigned int *>(data);
            h *= m;
            h ^= h >> r;

            data += 4;
            len -= 4;
        }
    }

    //----------
    // Handle tail bytes

    switch (len) {
    case 3:
        h += data[2] << 16;
        Q_FALLTHROUGH();
    case 2:
        h += data[1] << 8;
        Q_FALLTHROUGH();
    case 1:
        h += data[0];
        h *= m;
        h ^= h >> r;
    };

    h *= m;
    h ^= h >> 10;
    h *= m;
    h ^= h >> 17;

    return h;
}

/**
 * This is the hash function used for our data to hopefully make the
 * hashing used to place the QByteArrays as efficient as possible.
 */
quint32 SharedMemory::generateHash(const QByteArray &buffer)
{
    // The final constant is the "seed" for MurmurHash. Do *not* change it
    // without incrementing the cache version.
    return MurmurHashAligned(buffer.data(), buffer.size(), 0xF0F00F0F);
}

// Alignment concerns become a big deal when we're dealing with shared memory,
// since trying to access a structure sized at, say 8 bytes at an address that
// is not evenly divisible by 8 is a crash-inducing error on some
// architectures. The compiler would normally take care of this, but with
// shared memory the compiler will not necessarily know the alignment expected,
// so make sure we account for this ourselves. To do so we need a way to find
// out the expected alignment. Enter ALIGNOF...
#ifndef ALIGNOF
#if defined(Q_CC_GNU) || defined(Q_CC_SUN)
#define ALIGNOF(x) (__alignof__(x)) // GCC provides what we want directly
#else

#include <stddef.h> // offsetof

template<class T>
struct __alignmentHack {
    char firstEntry;
    T obj;
    static const size_t size = offsetof(__alignmentHack, obj);
};
#define ALIGNOF(x) (__alignmentHack<x>::size)
#endif // Non gcc
#endif // ALIGNOF undefined

// Returns a pointer properly aligned to handle size alignment.
// size should be a power of 2. start is assumed to be the lowest
// permissible address, therefore the return value will be >= start.
template<class T>
T *alignTo(const void *start, uint size = ALIGNOF(T))
{
    quintptr mask = size - 1;

    // Cast to int-type to handle bit-twiddling
    quintptr basePointer = reinterpret_cast<quintptr>(start);

    // If (and only if) we are already aligned, adding mask into basePointer
    // will not increment any of the bits in ~mask and we get the right answer.
    basePointer = (basePointer + mask) & ~mask;

    return reinterpret_cast<T *>(basePointer);
}

/**
 * Returns a pointer to a const object of type T, assumed to be @p offset
 * *BYTES* greater than the base address. Note that in order to meet alignment
 * requirements for T, it is possible that the returned pointer points greater
 * than @p offset into @p base.
 */
template<class T>
const T *offsetAs(const void *const base, qint32 offset)
{
    const char *ptr = reinterpret_cast<const char *>(base);
    return alignTo<const T>(ptr + offset);
}

// Same as above, but for non-const objects
template<class T>
T *offsetAs(void *const base, qint32 offset)
{
    char *ptr = reinterpret_cast<char *>(base);
    return alignTo<T>(ptr + offset);
}

/**
 * @return the smallest integer greater than or equal to (@p a / @p b).
 * @param a Numerator, should be ≥ 0.
 * @param b Denominator, should be > 0.
 */
unsigned SharedMemory::intCeil(unsigned a, unsigned b)
{
    // The overflow check is unsigned and so is actually defined behavior.
    if (Q_UNLIKELY(b == 0 || ((a + b) < a))) {
        throw KSDCCorrupted();
    }

    return (a + b - 1) / b;
}

/**
 * @return number of set bits in @p value (see also "Hamming weight")
 */
static unsigned countSetBits(unsigned value)
{
    // K&R / Wegner's algorithm used. GCC supports __builtin_popcount but we
    // expect there to always be only 1 bit set so this should be perhaps a bit
    // faster 99.9% of the time.
    unsigned count = 0;
    for (count = 0; value != 0; count++) {
        value &= (value - 1); // Clears least-significant set bit.
    }
    return count;
}

/**
 * Converts the given average item size into an appropriate page size.
 */
unsigned SharedMemory::equivalentPageSize(unsigned itemSize)
{
    if (itemSize == 0) {
        return 4096; // Default average item size.
    }

    int log2OfSize = 0;
    while ((itemSize >>= 1) != 0) {
        log2OfSize++;
    }

    // Bound page size between 512 bytes and 256 KiB.
    // If this is adjusted, also alter validSizeMask in cachePageSize
    log2OfSize = qBound(9, log2OfSize, 18);

    return (1 << log2OfSize);
}

// Returns pageSize in unsigned format.
unsigned SharedMemory::cachePageSize() const
{
    unsigned _pageSize = static_cast<unsigned>(pageSize.loadRelaxed());
    // bits 9-18 may be set.
    static const unsigned validSizeMask = 0x7FE00u;

    // Check for page sizes that are not a power-of-2, or are too low/high.
    if (Q_UNLIKELY(countSetBits(_pageSize) != 1 || (_pageSize & ~validSizeMask))) {
        throw KSDCCorrupted();
    }

    return _pageSize;
}

/**
 * This is effectively the class ctor.  But since we're in shared memory,
 * there's a few rules:
 *
 * 1. To allow for some form of locking in the initial-setup case, we
 * use an atomic int, which will be initialized to 0 by mmap().  Then to
 * take the lock we atomically increment the 0 to 1.  If we end up calling
 * the QAtomicInt constructor we can mess that up, so we can't use a
 * constructor for this class either.
 * 2. Any member variable you add takes up space in shared memory as well,
 * so make sure you need it.
 */
bool SharedMemory::performInitialSetup(uint _cacheSize, uint _pageSize)
{
    if (_cacheSize < MINIMUM_CACHE_SIZE) {
        qCCritical(KCOREADDONS_DEBUG) << "Internal error: Attempted to create a cache sized < " << MINIMUM_CACHE_SIZE;
        return false;
    }

    if (_pageSize == 0) {
        qCCritical(KCOREADDONS_DEBUG) << "Internal error: Attempted to create a cache with 0-sized pages.";
        return false;
    }

    shmLock.type = findBestSharedLock();
    if (shmLock.type == LOCKTYPE_INVALID) {
        qCCritical(KCOREADDONS_DEBUG) << "Unable to find an appropriate lock to guard the shared cache. "
                                      << "This *should* be essentially impossible. :(";
        return false;
    }

    bool isProcessShared = false;
    std::unique_ptr<KSDCLock> tempLock(createLockFromId(shmLock.type, shmLock));

    if (!tempLock->initialize(isProcessShared)) {
        qCCritical(KCOREADDONS_DEBUG) << "Unable to initialize the lock for the cache!";
        return false;
    }

    if (!isProcessShared) {
        qCWarning(KCOREADDONS_DEBUG) << "Cache initialized, but does not support being"
                                     << "shared across processes.";
    }

    // These must be updated to make some of our auxiliary functions
    // work right since their values will be based on the cache size.
    cacheSize = _cacheSize;
    pageSize = _pageSize;
    version = PIXMAP_CACHE_VERSION;
    cacheTimestamp = static_cast<unsigned>(::time(nullptr));

    clearInternalTables();

    // Unlock the mini-lock, and introduce a total memory barrier to make
    // sure all changes have propagated even without a mutex.
    return ready.ref();
}

void SharedMemory::clearInternalTables()
{
    // Assumes we're already locked somehow.
    cacheAvail = pageTableSize();

    // Setup page tables to point nowhere
    PageTableEntry *table = pageTable();
    for (uint i = 0; i < pageTableSize(); ++i) {
        table[i].index = -1;
    }

    // Setup index tables to be accurate.
    IndexTableEntry *indices = indexTable();
    for (uint i = 0; i < indexTableSize(); ++i) {
        indices[i].firstPage = -1;
        indices[i].useCount = 0;
        indices[i].fileNameHash = 0;
        indices[i].totalItemSize = 0;
        indices[i].addTime = 0;
        indices[i].lastUsedTime = 0;
    }
}

const IndexTableEntry *SharedMemory::indexTable() const
{
    // Index Table goes immediately after this struct, at the first byte
    // where alignment constraints are met (accounted for by offsetAs).
    return offsetAs<IndexTableEntry>(this, sizeof(*this));
}

const PageTableEntry *SharedMemory::pageTable() const
{
    const IndexTableEntry *base = indexTable();
    base += indexTableSize();

    // Let's call wherever we end up the start of the page table...
    return alignTo<PageTableEntry>(base);
}

const void *SharedMemory::cachePages() const
{
    const PageTableEntry *tableStart = pageTable();
    tableStart += pageTableSize();

    // Let's call wherever we end up the start of the data...
    return alignTo<void>(tableStart, cachePageSize());
}

const void *SharedMemory::page(pageID at) const
{
    if (static_cast<uint>(at) >= pageTableSize()) {
        return nullptr;
    }

    // We must manually calculate this one since pageSize varies.
    const char *pageStart = reinterpret_cast<const char *>(cachePages());
    pageStart += (at * cachePageSize());

    return reinterpret_cast<const void *>(pageStart);
}

// The following are non-const versions of some of the methods defined
// above.  They use const_cast<> because I feel that is better than
// duplicating the code.  I suppose template member functions (?)
// may work, may investigate later.
IndexTableEntry *SharedMemory::indexTable()
{
    const SharedMemory *that = const_cast<const SharedMemory *>(this);
    return const_cast<IndexTableEntry *>(that->indexTable());
}

PageTableEntry *SharedMemory::pageTable()
{
    const SharedMemory *that = const_cast<const SharedMemory *>(this);
    return const_cast<PageTableEntry *>(that->pageTable());
}

void *SharedMemory::cachePages()
{
    const SharedMemory *that = const_cast<const SharedMemory *>(this);
    return const_cast<void *>(that->cachePages());
}

void *SharedMemory::page(pageID at)
{
    const SharedMemory *that = const_cast<const SharedMemory *>(this);
    return const_cast<void *>(that->page(at));
}

uint SharedMemory::pageTableSize() const
{
    return cacheSize / cachePageSize();
}

uint SharedMemory::indexTableSize() const
{
    // Assume 2 pages on average are needed -> the number of entries
    // would be half of the number of pages.
    return pageTableSize() / 2;
}

/**
 * @return the index of the first page, for the set of contiguous
 * pages that can hold @p pagesNeeded PAGES.
 */
pageID SharedMemory::findEmptyPages(uint pagesNeeded) const
{
    if (Q_UNLIKELY(pagesNeeded > pageTableSize())) {
        return pageTableSize();
    }

    // Loop through the page table, find the first empty page, and just
    // makes sure that there are enough free pages.
    const PageTableEntry *table = pageTable();
    uint contiguousPagesFound = 0;
    pageID base = 0;
    for (pageID i = 0; i < static_cast<int>(pageTableSize()); ++i) {
        if (table[i].index < 0) {
            if (contiguousPagesFound == 0) {
                base = i;
            }
            contiguousPagesFound++;
        } else {
            contiguousPagesFound = 0;
        }

        if (contiguousPagesFound == pagesNeeded) {
            return base;
        }
    }

    return pageTableSize();
}

// left < right?
bool SharedMemory::lruCompare(const IndexTableEntry &l, const IndexTableEntry &r)
{
    // Ensure invalid entries migrate to the end
    if (l.firstPage < 0 && r.firstPage >= 0) {
        return false;
    }
    if (l.firstPage >= 0 && r.firstPage < 0) {
        return true;
    }

    // Most recently used will have the highest absolute time =>
    // least recently used (lowest) should go first => use left < right
    return l.lastUsedTime < r.lastUsedTime;
}

// left < right?
bool SharedMemory::seldomUsedCompare(const IndexTableEntry &l, const IndexTableEntry &r)
{
    // Ensure invalid entries migrate to the end
    if (l.firstPage < 0 && r.firstPage >= 0) {
        return false;
    }
    if (l.firstPage >= 0 && r.firstPage < 0) {
        return true;
    }

    // Put lowest use count at start by using left < right
    return l.useCount < r.useCount;
}

// left < right?
bool SharedMemory::ageCompare(const IndexTableEntry &l, const IndexTableEntry &r)
{
    // Ensure invalid entries migrate to the end
    if (l.firstPage < 0 && r.firstPage >= 0) {
        return false;
    }
    if (l.firstPage >= 0 && r.firstPage < 0) {
        return true;
    }

    // Oldest entries die first -- they have the lowest absolute add time,
    // so just like the others use left < right
    return l.addTime < r.addTime;
}

void SharedMemory::defragment()
{
    if (cacheAvail * cachePageSize() == cacheSize) {
        return; // That was easy
    }

    qCDebug(KCOREADDONS_DEBUG) << "Defragmenting the shared cache";

    // Just do a linear scan, and anytime there is free space, swap it
    // with the pages to its right. In order to meet the precondition
    // we need to skip any used pages first.

    pageID currentPage = 0;
    pageID idLimit = static_cast<pageID>(pageTableSize());
    PageTableEntry *pages = pageTable();

    if (Q_UNLIKELY(!pages || idLimit <= 0)) {
        throw KSDCCorrupted();
    }

    // Skip used pages
    while (currentPage < idLimit && pages[currentPage].index >= 0) {
        ++currentPage;
    }

    pageID freeSpot = currentPage;

    // Main loop, starting from a free page, skip to the used pages and
    // move them back.
    while (currentPage < idLimit) {
        // Find the next used page
        while (currentPage < idLimit && pages[currentPage].index < 0) {
            ++currentPage;
        }

        if (currentPage >= idLimit) {
            break;
        }

        // Found an entry, move it.
        qint32 affectedIndex = pages[currentPage].index;
        if (Q_UNLIKELY(affectedIndex < 0 || affectedIndex >= idLimit || indexTable()[affectedIndex].firstPage != currentPage)) {
            throw KSDCCorrupted();
        }

        indexTable()[affectedIndex].firstPage = freeSpot;

        // Moving one page at a time guarantees we can use memcpy safely
        // (in other words, the source and destination will not overlap).
        while (currentPage < idLimit && pages[currentPage].index >= 0) {
            const void *const sourcePage = page(currentPage);
            void *const destinationPage = page(freeSpot);

            // We always move used pages into previously-found empty spots,
            // so check ordering as well for logic errors.
            if (Q_UNLIKELY(!sourcePage || !destinationPage || sourcePage < destinationPage)) {
                throw KSDCCorrupted();
            }

            ::memcpy(destinationPage, sourcePage, cachePageSize());
            pages[freeSpot].index = affectedIndex;
            pages[currentPage].index = -1;
            ++currentPage;
            ++freeSpot;

            // If we've just moved the very last page and it happened to
            // be at the very end of the cache then we're done.
            if (currentPage >= idLimit) {
                break;
            }

            // We're moving consecutive used pages whether they belong to
            // our affected entry or not, so detect if we've started moving
            // the data for a different entry and adjust if necessary.
            if (affectedIndex != pages[currentPage].index) {
                indexTable()[pages[currentPage].index].firstPage = freeSpot;
            }
            affectedIndex = pages[currentPage].index;
        }

        // At this point currentPage is on a page that is unused, and the
        // cycle repeats. However, currentPage is not the first unused
        // page, freeSpot is, so leave it alone.
    }
}

/**
 * Finds the index entry for a given key.
 * @param key UTF-8 encoded key to search for.
 * @return The index of the entry in the cache named by @p key. Returns
 *         <0 if no such entry is present.
 */
qint32 SharedMemory::findNamedEntry(const QByteArray &key) const
{
    uint keyHash = SharedMemory::generateHash(key);
    uint position = keyHash % indexTableSize();
    uint probeNumber = 1; // See insert() for description

    // Imagine 3 entries A, B, C in this logical probing chain. If B is
    // later removed then we can't find C either. So, we must keep
    // searching for probeNumber number of tries (or we find the item,
    // obviously).
    while (indexTable()[position].fileNameHash != keyHash && probeNumber < MAX_PROBE_COUNT) {
        position = (keyHash + (probeNumber + probeNumber * probeNumber) / 2) % indexTableSize();
        probeNumber++;
    }

    if (indexTable()[position].fileNameHash == keyHash) {
        pageID firstPage = indexTable()[position].firstPage;
        if (firstPage < 0 || static_cast<uint>(firstPage) >= pageTableSize()) {
            return -1;
        }

        const void *resultPage = page(firstPage);
        if (Q_UNLIKELY(!resultPage)) {
            throw KSDCCorrupted();
        }

        const char *utf8FileName = reinterpret_cast<const char *>(resultPage);
        if (qstrncmp(utf8FileName, key.constData(), cachePageSize()) == 0) {
            return position;
        }
    }

    return -1; // Not found, or a different one found.
}

// Function to use with std::unique_ptr in removeUsedPages below...
void SharedMemory::deleteTable(IndexTableEntry *table)
{
    delete[] table;
}

/**
 * Removes the requested number of pages.
 *
 * @param numberNeeded the number of pages required to fulfill a current request.
 *        This number should be <0 and <= the number of pages in the cache.
 * @return The identifier of the beginning of a consecutive block of pages able
 *         to fill the request. Returns a value >= pageTableSize() if no such
 *         request can be filled.
 * @internal
 */
uint SharedMemory::removeUsedPages(uint numberNeeded)
{
    if (numberNeeded == 0) {
        qCCritical(KCOREADDONS_DEBUG) << "Internal error: Asked to remove exactly 0 pages for some reason.";
        throw KSDCCorrupted();
    }

    if (numberNeeded > pageTableSize()) {
        qCCritical(KCOREADDONS_DEBUG) << "Internal error: Requested more space than exists in the cache.";
        qCCritical(KCOREADDONS_DEBUG) << numberNeeded << "requested, " << pageTableSize() << "is the total possible.";
        throw KSDCCorrupted();
    }

    // If the cache free space is large enough we will defragment first
    // instead since it's likely we're highly fragmented.
    // Otherwise, we will (eventually) simply remove entries per the
    // eviction order set for the cache until there is enough room
    // available to hold the number of pages we need.

    qCDebug(KCOREADDONS_DEBUG) << "Removing old entries to free up" << numberNeeded << "pages," << cacheAvail << "are already theoretically available.";

    if (cacheAvail > 3 * numberNeeded) {
        defragment();
        uint result = findEmptyPages(numberNeeded);

        if (result < pageTableSize()) {
            return result;
        } else {
            qCCritical(KCOREADDONS_DEBUG) << "Just defragmented a locked cache, but still there"
                                          << "isn't enough room for the current request.";
        }
    }

    // At this point we know we'll have to free some space up, so sort our
    // list of entries by whatever the current criteria are and start
    // killing expired entries.
    std::unique_ptr<IndexTableEntry, decltype(deleteTable) *> tablePtr(new IndexTableEntry[indexTableSize()], deleteTable);

    if (!tablePtr) {
        qCCritical(KCOREADDONS_DEBUG) << "Unable to allocate temporary memory for sorting the cache!";
        clearInternalTables();
        throw KSDCCorrupted();
    }

    // We use tablePtr to ensure the data is destroyed, but do the access
    // via a helper pointer to allow for array ops.
    IndexTableEntry *table = tablePtr.get();

    ::memcpy(table, indexTable(), sizeof(IndexTableEntry) * indexTableSize());

    // Our entry ID is simply its index into the
    // index table, which qSort will rearrange all willy-nilly, so first
    // we'll save the *real* entry ID into firstPage (which is useless in
    // our copy of the index table). On the other hand if the entry is not
    // used then we note that with -1.
    for (uint i = 0; i < indexTableSize(); ++i) {
        table[i].firstPage = table[i].useCount > 0 ? static_cast<pageID>(i) : -1;
    }

    // Declare the comparison function that we'll use to pass to qSort,
    // based on our cache eviction policy.
    bool (*compareFunction)(const IndexTableEntry &, const IndexTableEntry &);
    switch (evictionPolicy.loadRelaxed()) {
    case KSharedDataCache::EvictLeastOftenUsed:
    case KSharedDataCache::NoEvictionPreference:
    default:
        compareFunction = seldomUsedCompare;
        break;

    case KSharedDataCache::EvictLeastRecentlyUsed:
        compareFunction = lruCompare;
        break;

    case KSharedDataCache::EvictOldest:
        compareFunction = ageCompare;
        break;
    }

    std::sort(table, table + indexTableSize(), compareFunction);

    // Least recently used entries will be in the front.
    // Start killing until we have room.

    // Note on removeEntry: It expects an index into the index table,
    // but our sorted list is all jumbled. But we stored the real index
    // in the firstPage member.
    // Remove entries until we've removed at least the required number
    // of pages.
    uint i = 0;
    while (i < indexTableSize() && numberNeeded > cacheAvail) {
        int curIndex = table[i++].firstPage; // Really an index, not a page

        // Removed everything, still no luck (or curIndex is set but too high).
        if (curIndex < 0 || static_cast<uint>(curIndex) >= indexTableSize()) {
            qCCritical(KCOREADDONS_DEBUG) << "Trying to remove index" << curIndex << "out-of-bounds for index table of size" << indexTableSize();
            throw KSDCCorrupted();
        }

        qCDebug(KCOREADDONS_DEBUG) << "Removing entry of" << indexTable()[curIndex].totalItemSize << "size";
        removeEntry(curIndex);
    }

    // At this point let's see if we have freed up enough data by
    // defragmenting first and seeing if we can find that free space.
    defragment();

    pageID result = pageTableSize();
    while (i < indexTableSize() && (static_cast<uint>(result = findEmptyPages(numberNeeded))) >= pageTableSize()) {
        int curIndex = table[i++].firstPage;

        if (curIndex < 0) {
            // One last shot.
            defragment();
            return findEmptyPages(numberNeeded);
        }

        if (Q_UNLIKELY(static_cast<uint>(curIndex) >= indexTableSize())) {
            throw KSDCCorrupted();
        }

        removeEntry(curIndex);
    }

    // Whew.
    return result;
}

// Returns the total size required for a given cache size.
uint SharedMemory::totalSize(uint cacheSize, uint effectivePageSize)
{
    uint numberPages = intCeil(cacheSize, effectivePageSize);
    uint indexTableSize = numberPages / 2;

    // Knowing the number of pages, we can determine what addresses we'd be
    // using (properly aligned), and from there determine how much memory
    // we'd use.
    IndexTableEntry *indexTableStart = offsetAs<IndexTableEntry>(static_cast<void *>(nullptr), sizeof(SharedMemory));

    indexTableStart += indexTableSize;

    PageTableEntry *pageTableStart = reinterpret_cast<PageTableEntry *>(indexTableStart);
    pageTableStart = alignTo<PageTableEntry>(pageTableStart);
    pageTableStart += numberPages;

    // The weird part, we must manually adjust the pointer based on the page size.
    char *cacheStart = reinterpret_cast<char *>(pageTableStart);
    cacheStart += (numberPages * effectivePageSize);

    // ALIGNOF gives pointer alignment
    cacheStart = alignTo<char>(cacheStart, ALIGNOF(void *));

    // We've traversed the header, index, page table, and cache.
    // Wherever we're at now is the size of the enchilada.
    return static_cast<uint>(reinterpret_cast<quintptr>(cacheStart));
}

uint SharedMemory::fileNameHash(const QByteArray &utf8FileName) const
{
    return generateHash(utf8FileName) % indexTableSize();
}

void SharedMemory::clear()
{
    clearInternalTables();
}

// Must be called while the lock is already held!
void SharedMemory::removeEntry(uint index)
{
    if (index >= indexTableSize() || cacheAvail > pageTableSize()) {
        throw KSDCCorrupted();
    }

    PageTableEntry *pageTableEntries = pageTable();
    IndexTableEntry *entriesIndex = indexTable();

    // Update page table first
    pageID firstPage = entriesIndex[index].firstPage;
    if (firstPage < 0 || static_cast<quint32>(firstPage) >= pageTableSize()) {
        qCDebug(KCOREADDONS_DEBUG) << "Trying to remove an entry which is already invalid. This "
                                   << "cache is likely corrupt.";
        throw KSDCCorrupted();
    }

    if (index != static_cast<uint>(pageTableEntries[firstPage].index)) {
        qCCritical(KCOREADDONS_DEBUG) << "Removing entry" << index << "but the matching data"
                                      << "doesn't link back -- cache is corrupt, clearing.";
        throw KSDCCorrupted();
    }

    uint entriesToRemove = intCeil(entriesIndex[index].totalItemSize, cachePageSize());
    uint savedCacheSize = cacheAvail;
    for (uint i = firstPage; i < pageTableSize() && static_cast<uint>(pageTableEntries[i].index) == index; ++i) {
        pageTableEntries[i].index = -1;
        cacheAvail++;
    }

    if ((cacheAvail - savedCacheSize) != entriesToRemove) {
        qCCritical(KCOREADDONS_DEBUG) << "We somehow did not remove" << entriesToRemove << "when removing entry" << index << ", instead we removed"
                                      << (cacheAvail - savedCacheSize);
        throw KSDCCorrupted();
    }

// For debugging
#ifdef NDEBUG
    void *const startOfData = page(firstPage);
    if (startOfData) {
        QByteArray str((const char *)startOfData);
        str.prepend(" REMOVED: ");
        str.prepend(QByteArray::number(index));
        str.prepend("ENTRY ");

        ::memcpy(startOfData, str.constData(), str.size() + 1);
    }
#endif

    // Update the index
    entriesIndex[index].fileNameHash = 0;
    entriesIndex[index].totalItemSize = 0;
    entriesIndex[index].useCount = 0;
    entriesIndex[index].lastUsedTime = 0;
    entriesIndex[index].addTime = 0;
    entriesIndex[index].firstPage = -1;
}
