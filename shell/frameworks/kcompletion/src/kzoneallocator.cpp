/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2002 Michael Matz <matz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/* Fast zone memory allocator with deallocation support, for use as obstack
   or as general purpose allocator.  It does no compaction.  If the usage
   pattern is non-optimal it might waste some memory while running.  E.g.
   allocating many small things at once, and then deallocating only every
   second one, there is a high chance, that actually no memory is freed.
 */

#include "kzoneallocator_p.h"

#include <kcompletion_debug.h>

#include <QList>

#include <stdio.h>

class KZoneAllocator::MemBlock
{
public:
    MemBlock(size_t s)
        : size(s)
        , ref(0)
        , older(nullptr)
        , newer(nullptr)
    {
        begin = new char[s];
    }
    ~MemBlock()
    {
        delete[] begin;
    }
    MemBlock(const MemBlock &) = delete;
    MemBlock &operator=(const MemBlock &) = delete;
    bool is_in(void *ptr) const
    {
        return !(begin > (char *)ptr || (begin + size) <= (char *)ptr);
    }
    size_t size;
    unsigned int ref;
    char *begin;
    MemBlock *older;
    MemBlock *newer;
};

class KZoneAllocator::Private
{
public:
    Private()
        : currentBlock(nullptr)
        , blockSize(1)
        , blockOffset(0)
        , log2(0)
        , num_blocks(0)
        , hashList(nullptr)
        , hashSize(0)
        , hashDirty(true)
    {
    }

    /** One block is 'current' to satisfy requests. @internal */
    MemBlock *currentBlock;
    /** Store block size from constructor. @internal */
    quintptr blockSize;
    /** Store offset into current block; size-offset is free. @internal */
    quintptr blockOffset;
    /** base-2 log of the block size. @internal */
    unsigned int log2;
    /** Count total number of allocated blocks. @internal */
    unsigned int num_blocks;
    /** Collection of lists of blocks, for lookups. @internal */
    MemList **hashList;
    /** Count of hashes. @internal */
    unsigned int hashSize;
    /** Flag the hashes as in need of reorganization. @internal */
    bool hashDirty;
};

KZoneAllocator::KZoneAllocator(unsigned long _blockSize)
    : d(new Private)
{
    while (d->blockSize < _blockSize) {
        d->blockSize <<= 1;
        d->log2++;
    }

    /* Make sure, that a block is allocated at the first time allocate()
       is called (even with a 0 size).  */
    d->blockOffset = d->blockSize + 1;
}

KZoneAllocator::~KZoneAllocator()
{
    unsigned int count = 0;
    if (d->hashList) {
        /* No need to maintain the different lists in d->hashList[] anymore.
           I.e. no need to use delBlock().  */
        for (unsigned int i = 0; i < d->hashSize; i++) {
            delete d->hashList[i];
        }
        delete[] d->hashList;
        d->hashList = nullptr;
    }
    MemBlock *next;
    for (; d->currentBlock; d->currentBlock = next) {
        next = d->currentBlock->older;
        delete d->currentBlock;
        count++;
    }
#ifndef NDEBUG // as this is called quite late in the app, we don't care
    // to use qDebug
    if (count > 1) {
        fprintf(stderr, "zone still contained %u blocks", count);
    }
#endif
    delete d;
}

void KZoneAllocator::insertHash(MemBlock *b)
{
    quintptr adr = ((quintptr)b->begin) & (~(d->blockSize - 1));
    quintptr end = ((quintptr)b->begin) + d->blockSize;
    while (adr < end) {
        quintptr key = adr >> d->log2;
        key = key & (d->hashSize - 1);
        if (!d->hashList[key]) {
            d->hashList[key] = new QList<MemBlock *>;
        }
        d->hashList[key]->append(b);
        adr += d->blockSize;
    }
}

/** Add a new memory block to the pool of blocks,
    and reorganize the hash lists if needed.
    @param b block to add
    @internal
*/
void KZoneAllocator::addBlock(MemBlock *b)
{
    b->newer = nullptr;
    b->older = d->currentBlock;
    if (d->currentBlock) {
        b->older->newer = b;
    }
    d->currentBlock = b;
    d->num_blocks++;
    /* If we either have no d->hashList at all, or since it's last construction
       there are now many more blocks we reconstruct the list.  But don't
       make it larger than a certain maximum.  */
    if (d->hashList && ((d->num_blocks / 4) > d->hashSize && d->hashSize < 64 * 1024)) {
        d->hashDirty = true;
    }
    /* Only insert this block into the hashlists, if we aren't going to
       reconstruct them anyway.  */
    if (d->hashList && !d->hashDirty) {
        insertHash(b);
    }
}

/** Reinitialize hash list. @internal */
void KZoneAllocator::initHash()
{
    if (d->hashList) {
        for (unsigned int i = 0; i < d->hashSize; i++) {
            delete d->hashList[i];
        }
        delete[] d->hashList;
        d->hashList = nullptr;
    }
    d->hashSize = 1;
    while (d->hashSize < d->num_blocks) {
        d->hashSize <<= 1;
    }
    if (d->hashSize < 1024) {
        d->hashSize = 1024;
    }
    if (d->hashSize > 64 * 1024) {
        d->hashSize = 64 * 1024;
    }
    d->hashList = new QList<MemBlock *> *[d->hashSize];
    memset(d->hashList, 0, sizeof(QList<MemBlock *> *) * d->hashSize);
    d->hashDirty = false;
    for (MemBlock *b = d->currentBlock; b; b = b->older) {
        insertHash(b);
    }
}

/** Delete a memory block. This @em really returns the memory to the heap.
    @param b block to delete
    @internal
*/
void KZoneAllocator::delBlock(MemBlock *b)
{
    /* Update also the hashlists if we aren't going to reconstruct them
       soon.  */
    if (d->hashList && !d->hashDirty) {
        quintptr adr = ((quintptr)b->begin) & (~(d->blockSize - 1));
        quintptr end = ((quintptr)b->begin) + d->blockSize;
        while (adr < end) {
            quintptr key = adr >> d->log2;
            key = key & (d->hashSize - 1);
            if (d->hashList[key]) {
                QList<MemBlock *> *list = d->hashList[key];
                QList<MemBlock *>::Iterator it = list->begin();
                QList<MemBlock *>::Iterator endit = list->end();
                for (; it != endit; ++it) {
                    if (*it == b) {
                        list->erase(it);
                        break;
                    }
                }
            }
            adr += d->blockSize;
        }
    }
    if (b->older) {
        b->older->newer = b->newer;
    }
    if (b->newer) {
        b->newer->older = b->older;
    }
    if (b == d->currentBlock) {
        d->currentBlock = nullptr;
        d->blockOffset = d->blockSize;
    }
    delete b;
    d->num_blocks--;
}

void *KZoneAllocator::allocate(size_t _size)
{
    // Use the size of (void *) as alignment
    const size_t alignment = sizeof(void *) - 1;
    _size = (_size + alignment) & ~alignment;

    if ((unsigned long)_size + d->blockOffset > d->blockSize) {
        if (_size > d->blockSize) {
            qCDebug(KCOMPLETION_LOG, "KZoneAllocator: allocating more than %zu bytes", (size_t)d->blockSize);
            return nullptr;
        }
        addBlock(new MemBlock(d->blockSize));
        d->blockOffset = 0;
        // qDebug ("Allocating block #%d (%x)\n", d->num_blocks, d->currentBlock->begin);
    }
    void *result = (void *)(d->currentBlock->begin + d->blockOffset);
    d->currentBlock->ref++;
    d->blockOffset += _size;
    return result;
}

void KZoneAllocator::deallocate(void *ptr)
{
    if (d->hashDirty) {
        initHash();
    }

    quintptr key = (((quintptr)ptr) >> d->log2) & (d->hashSize - 1);
    const QList<MemBlock *> *list = d->hashList[key];
    if (!list) {
        /* Can happen with certain usage pattern of intermixed free_since()
           and deallocate().  */
        // qDebug("Uhoh");
        return;
    }
    QList<MemBlock *>::ConstIterator it = list->begin();
    QList<MemBlock *>::ConstIterator endit = list->end();
    for (; it != endit; ++it) {
        MemBlock *cur = *it;
        if (cur->is_in(ptr)) {
            if (!--cur->ref) {
                if (cur != d->currentBlock) {
                    delBlock(cur);
                } else {
                    d->blockOffset = 0;
                }
            }
            return;
        }
    }
    /* Can happen with certain usage pattern of intermixed free_since()
       and deallocate().  */
    // qDebug("Uhoh2");
}

void KZoneAllocator::free_since(void *ptr)
{
    /* If we have a d->hashList and it's not yet dirty, see, if we will dirty
       it by removing too many blocks.  This will make the below delBlock()s
       faster.  */
    if (d->hashList && !d->hashDirty) {
        const MemBlock *b;
        unsigned int removed = 0;
        for (b = d->currentBlock; b; b = b->older, removed++) {
            if (b->is_in(ptr)) {
                break;
            }
        }
        if (d->hashSize >= 4 * (d->num_blocks - removed)) {
            d->hashDirty = true;
        }
    }
    while (d->currentBlock && !d->currentBlock->is_in(ptr)) {
        d->currentBlock = d->currentBlock->older;
        delBlock(d->currentBlock->newer);
    }
    d->blockOffset = ((char *)ptr) - d->currentBlock->begin;
}
