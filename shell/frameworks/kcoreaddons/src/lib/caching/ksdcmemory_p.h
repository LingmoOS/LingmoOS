/*
 *    This file is part of the KDE project.
 *
 *    SPDX-FileCopyrightText: 2010 Michael Pyne <mpyne@kde.org>
 *    SPDX-License-Identifier: LGPL-2.0-only
 */

#ifndef KSDCMEMORY_P_H
#define KSDCMEMORY_P_H

#include "kcoreaddons_debug.h"
#include "ksdclock_p.h"
#include "kshareddatacache.h"

/**
 * A very simple class whose only purpose is to be thrown as an exception from
 * underlying code to indicate that the shared cache is apparently corrupt.
 * This must be caught by top-level library code and used to unlink the cache
 * in this circumstance.
 *
 * @internal
 */
class KSDCCorrupted
{
public:
    KSDCCorrupted()
    {
        qCWarning(KCOREADDONS_DEBUG) << "Error detected in cache!";
    }

    KSDCCorrupted(const QString message)
    {
        qCWarning(KCOREADDONS_DEBUG).noquote() << message;
    }

    KSDCCorrupted(const char *message)
    {
        KSDCCorrupted(QLatin1String(message));
    }
};

typedef qint32 pageID;

// =========================================================================
// Description of the cache:
//
// The shared memory cache is designed to be handled as two separate objects,
// all contained in the same global memory segment. First off, there is the
// basic header data, consisting of the global header followed by the
// accounting data necessary to hold items (described in more detail
// momentarily). Following the accounting data is the start of the "page table"
// (essentially just as you'd see it in an Operating Systems text).
//
// The page table contains shared memory split into fixed-size pages, with a
// configurable page size. In the event that the data is too large to fit into
// a single logical page, it will need to occupy consecutive pages of memory.
//
// The accounting data that was referenced earlier is split into two:
//
// 1. index table, containing a fixed-size list of possible cache entries.
// Each index entry is of type IndexTableEntry (below), and holds the various
// accounting data and a pointer to the first page.
//
// 2. page table, which is used to speed up the process of searching for
// free pages of memory. There is one entry for every page in the page table,
// and it contains the index of the one entry in the index table actually
// holding the page (or <0 if the page is free).
//
// The entire segment looks like so:
// ?════════?═════════════?════════════?═══════?═══════?═══════?═══════?═══?
// ? Header │ Index Table │ Page Table ? Pages │       │       │       │...?
// ?════════?═════════════?════════════?═══════?═══════?═══════?═══════?═══?
// =========================================================================

// All elements of this struct must be "plain old data" (POD) types since it
// will be in shared memory.  In addition, no pointers!  To point to something
// you must use relative offsets since the pointer start addresses will be
// different in each process.
struct IndexTableEntry {
    uint fileNameHash;
    uint totalItemSize; // in bytes
    mutable uint useCount;
    time_t addTime;
    mutable time_t lastUsedTime;
    pageID firstPage;
};

// Page table entry
struct PageTableEntry {
    // int so we can use values <0 for unassigned pages.
    qint32 index;
};

// Each individual page contains the cached data. The first page starts off with
// the utf8-encoded key, a null '\0', and then the data follows immediately
// from the next byte, possibly crossing consecutive page boundaries to hold
// all of the data.
// There is, however, no specific struct for a page, it is simply a location in
// memory.

// This is effectively the layout of the shared memory segment. The variables
// contained within form the header, data contained afterwards is pointed to
// by using special accessor functions.
struct SharedMemory {
    /**
     * Note to downstream packagers: This version flag is intended to be
     * machine-specific. The KDE-provided source code will not set the lower
     * two bits to allow for distribution-specific needs, with the exception
     * of version 1 which was already defined in KDE Platform 4.5.
     * e.g. the next version bump will be from 4 to 8, then 12, etc.
     */
    enum {
        PIXMAP_CACHE_VERSION = 12,
        MINIMUM_CACHE_SIZE = 4096,
    };

    /// The maximum number of probes to make while searching for a bucket in
    /// the presence of collisions in the cache index table.
    static const uint MAX_PROBE_COUNT = 6;

    // Note to those who follow me. You should not, under any circumstances, ever
    // re-arrange the following two fields, even if you change the version number
    // for later revisions of this code.
    QAtomicInt ready; ///< DO NOT INITIALIZE
    quint8 version;

    // See kshareddatacache_p.h
    SharedLock shmLock;

    uint cacheSize;
    uint cacheAvail;
    QAtomicInt evictionPolicy;

    // pageSize and cacheSize determine the number of pages. The number of
    // pages determine the page table size and (indirectly) the index table
    // size.
    QAtomicInt pageSize;

    // This variable is added to reserve space for later cache timestamping
    // support. The idea is this variable will be updated when the cache is
    // written to, to allow clients to detect a changed cache quickly.
    QAtomicInt cacheTimestamp;

    /**
     * Converts the given average item size into an appropriate page size.
     */
    static unsigned equivalentPageSize(unsigned itemSize);

    // Returns pageSize in unsigned format.
    unsigned cachePageSize() const;

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
    bool performInitialSetup(uint _cacheSize, uint _pageSize);

    void clearInternalTables();
    const IndexTableEntry *indexTable() const;
    const PageTableEntry *pageTable() const;
    const void *cachePages() const;
    const void *page(pageID at) const;

    // The following are non-const versions of some of the methods defined
    // above.  They use const_cast<> because I feel that is better than
    // duplicating the code.  I suppose template member functions (?)
    // may work, may investigate later.
    IndexTableEntry *indexTable();
    PageTableEntry *pageTable();
    void *cachePages();
    void *page(pageID at);
    uint pageTableSize() const;
    uint indexTableSize() const;

    /**
     * @return the index of the first page, for the set of contiguous
     * pages that can hold @p pagesNeeded PAGES.
     */
    pageID findEmptyPages(uint pagesNeeded) const;

    // left < right?
    static bool lruCompare(const IndexTableEntry &l, const IndexTableEntry &r);

    // left < right?
    static bool seldomUsedCompare(const IndexTableEntry &l, const IndexTableEntry &r);

    // left < right?
    static bool ageCompare(const IndexTableEntry &l, const IndexTableEntry &r);

    void defragment();

    /**
     * Finds the index entry for a given key.
     * @param key UTF-8 encoded key to search for.
     * @return The index of the entry in the cache named by @p key. Returns
     *         <0 if no such entry is present.
     */
    qint32 findNamedEntry(const QByteArray &key) const;

    // Function to use with std::unique_ptr in removeUsedPages below...
    static void deleteTable(IndexTableEntry *table);

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
    uint removeUsedPages(uint numberNeeded);

    // Returns the total size required for a given cache size.
    static uint totalSize(uint cacheSize, uint effectivePageSize);

    uint fileNameHash(const QByteArray &utf8FileName) const;
    void clear();
    void removeEntry(uint index);

    static quint32 generateHash(const QByteArray &buffer);

    /**
     * @return the smallest integer greater than or equal to (@p a / @p b).
     * @param a Numerator, should be ≥ 0.
     * @param b Denominator, should be > 0.
     */
    static unsigned intCeil(unsigned a, unsigned b);
};

#endif /* KSDCMEMORY_P_H */
