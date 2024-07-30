/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: 2010 Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KVOIDPOINTERFACTORY_P_H
#define KVOIDPOINTERFACTORY_P_H

#include <cstddef>
#include <cstdlib>
#include <vector>

#define DEFAULT_BLOCK_SIZE 256

/**
 * @brief A Class for managing void pointers for use in QModelIndexes.
 *
 * This class creates void pointers pointing to individual blocks of memory.
 * The pointed-to memory contains zeros.
 *
 * Memory is allocated in blocks of size @p blockSize times sizeof(void*) at a time. The used
 * memory is automatically freed and can be cleared manually.
 *
 * The void pointers should not be dereferenced, but only used as a unique
 * identifier suitable for use with createIndex() and for comparison with other void pointers.
 */
template<std::size_t blockSize = DEFAULT_BLOCK_SIZE>
class KVoidPointerFactory
{
    // a class with size 1.
    class Bit
    {
        bool bit;
    };

public:
    KVoidPointerFactory()
        : m_previousPointer(nullptr)
        , m_finalPointer(nullptr)
    {
    }

    KVoidPointerFactory(const KVoidPointerFactory<blockSize> &other)
    {
        *this = other;
    }

    KVoidPointerFactory<blockSize> &operator=(const KVoidPointerFactory<blockSize> &other)
    {
        m_previousPointer = other.m_previousPointer;
        m_finalPointer = other.m_finalPointer;
        m_blocks = other.m_blocks;
        return *this;
    }

    ~KVoidPointerFactory()
    {
        clear();
    }

    void clear()
    {
        typename std::vector<Bit *>::const_iterator it = m_blocks.begin();
        const typename std::vector<Bit *>::const_iterator end = m_blocks.end();
        for (; it != end; ++it) {
            free(*it);
        }
        m_blocks.clear();
        m_finalPointer = nullptr;
        m_previousPointer = nullptr;
    }

    void *createPointer() const
    {
        if (m_previousPointer == m_finalPointer) {
            static const std::size_t pointer_size = sizeof(Bit *);
            Bit *const bit = static_cast<Bit *>(calloc(blockSize, pointer_size));
            m_blocks.push_back(bit);
            m_finalPointer = bit + (blockSize * pointer_size) - 1;
            m_previousPointer = bit;
            return bit;
        }
        return ++m_previousPointer;
    }

private:
    mutable std::vector<Bit *> m_blocks;
    mutable Bit *m_previousPointer;
    mutable Bit *m_finalPointer;
};

// Disable factory with 0 blockSize
template<>
class KVoidPointerFactory<0>
{
public:
    KVoidPointerFactory();

    void clear();
    void *createPointer();
};

#endif
