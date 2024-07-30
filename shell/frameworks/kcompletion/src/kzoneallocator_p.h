/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2002 Michael Matz <matz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
//----------------------------------------------------------------------------
//
// KDE Memory Allocator

#ifndef KZONEALLOCATOR_P_H
#define KZONEALLOCATOR_P_H

#include <cstddef> // size_t

template<typename T>
class QList;

/**
 * \class KZoneAllocator kallocator.h <KZoneAllocator>
 *
 * Memory allocator for large groups of small objects.
 * This should be used for large groups of objects that are created and
 * destroyed together. When used carefully for this purpose it is faster
 * and more memory efficient than malloc. Additionally to a usual obstack
 * like allocator you can also free the objects individually. Because it
 * does no compaction it still is faster than malloc()/free(). Depending
 * on the exact usage pattern that might come at the expense of some
 * memory though.
 * @author Waldo Bastian <bastian@kde.org>, Michael Matz <matz@kde.org>
 */
class KZoneAllocator
{
public:
    /**
     * Creates a KZoneAllocator object.
     * @param _blockSize Size in bytes of the blocks requested from malloc.
     */
    explicit KZoneAllocator(unsigned long _blockSize = 8 * 1024);

    /**
     * Destructs the ZoneAllocator and free all memory allocated by it.
     */
    ~KZoneAllocator();

    KZoneAllocator(const KZoneAllocator &) = delete;
    KZoneAllocator &operator=(const KZoneAllocator &) = delete;

    /**
     * Allocates a memory block.
     * @param _size Size in bytes of the memory block. Memory is aligned to
     * the size of a pointer.
     */
    void *allocate(size_t _size);

    /**
     * Gives back a block returned by allocate() to the zone
     * allocator, and possibly deallocates the block holding it (when it's
     * empty). The first deallocate() after many allocate() calls
     * (or the first at all) builds an internal data structure for speeding
     * up deallocation. The consistency of that structure is maintained
     * from then on (by allocate() and deallocate()) unless many
     * more objects are allocated without any intervening deallocation, in
     * which case it's thrown away and rebuilt at the next deallocate().
     *
     * The effect of this is, that such initial deallocate() calls take
     * more time then the normal calls, and that after this list is built, i.e.
     * generally if deallocate() is used at all, also allocate() is a
     * little bit slower. This means, that if you want to squeeze out the last
     * bit performance you would want to use KZoneAllocator as an obstack, i.e.
     * just use the functions allocate() and free_since(). All the
     * remaining memory is returned to the system if the zone allocator
     * is destroyed.
     * @param ptr Pointer as returned by allocate().
     */
    void deallocate(void *ptr);

    /**
     * Deallocate many objects at once.
     * free_since() deallocates all objects allocated after @p ptr,
     * @em including @p ptr itself.
     *
     * The intended use is something along the lines of:
     * \code
     * KZoneAllocator alloc(8192);
     * void *remember_me = alloc.allocate(0);
     * for (int i = 0; i < 1000; i++)
     *   do_something_with (alloc.allocate(12));
     * alloc.free_since (remember_me);
     * \endcode
     * Note, that we don't need to remember all the pointers to the 12-byte
     * objects for freeing them. The free_since() does deallocate them
     * all at once.
     * @param ptr Pointer as returned by allocate(). It acts like
     * a kind of mark of a certain position in the stack of all objects,
     * off which you can throw away everything above that mark.
     */
    void free_since(void *ptr);

protected:
    /** A single chunk of memory from the heap. @internal */
    class MemBlock;
    /**< A list of chunks. @internal */
    typedef QList<MemBlock *> MemList;
    void addBlock(MemBlock *b);
    void delBlock(MemBlock *b);
    void insertHash(MemBlock *b);
    void initHash();

private:
    class Private;
    Private *const d;
};

#endif
