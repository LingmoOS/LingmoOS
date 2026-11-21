// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/shared_memory.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "compositor/shm_pool.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <limits>
#include <mutex>

#include "compositor/buffer.h"
#include "core/logging.h"

namespace Lingmo::Compositor {

namespace {

// The pool being read on the current thread. SIGBUS is delivered to the thread
// that touched the page, so the handler needs no locking to find it.
struct AccessState {
    ShmPool *pool = nullptr;
    int depth = 0;
};

thread_local AccessState t_access;

std::once_flag s_sigbusInstalled;
struct sigaction s_previousSigbusAction;

// A regular file shorter than the pool would mmap fine and fault on the first
// touch past its end. Other file types have no meaningful st_size.
bool fileBacks(int fd, size_t size)
{
    struct stat status {};
    if (fstat(fd, &status) != 0)
        return false;
    return !S_ISREG(status.st_mode)
            || (status.st_size >= 0 && static_cast<uint64_t>(status.st_size) >= size);
}

} // namespace

ShmPool::ShmPool(int fd, void *data, size_t size)
    : m_fd(fd)
    , m_data(data)
    , m_size(size)
{
}

ShmPool::~ShmPool()
{
    if (m_data)
        munmap(m_data, m_size);
    if (m_fd >= 0)
        close(m_fd);
}

std::shared_ptr<ShmPool> ShmPool::create(int fd, int32_t size)
{
    if (size <= 0) {
        LM_WARNING(lingmoAurora) << "Rejecting a shm pool of size" << size;
        close(fd);
        return nullptr;
    }

    if (!fileBacks(fd, static_cast<size_t>(size))) {
        LM_WARNING(lingmoAurora) << "Rejecting a shm pool of" << size
                                 << "bytes whose file is smaller than that";
        close(fd);
        return nullptr;
    }

    // Only ever read from; the client keeps writing to the same pages.
    void *data = mmap(nullptr, static_cast<size_t>(size), PROT_READ, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        LM_WARNING(lingmoAurora).printf(
                "Failed to map a shm pool of %d bytes: %s", size, strerror(errno));
        close(fd);
        return nullptr;
    }

    LM_DEBUG(lingmoAurora) << "Created shm pool of" << size << "bytes";
    return std::shared_ptr<ShmPool>(new ShmPool(fd, data, static_cast<size_t>(size)));
}

bool ShmPool::resize(int32_t newSize)
{
    if (newSize <= 0 || static_cast<size_t>(newSize) < m_size)
        return false;
    if (static_cast<size_t>(newSize) == m_size)
        return true;
    if (m_faulted)
        return false;

    if (!fileBacks(m_fd, static_cast<size_t>(newSize))) {
        LM_WARNING(lingmoAurora) << "Refusing to grow a shm pool to" << newSize
                                 << "bytes: its file is smaller than that";
        return false;
    }

    void *data = mremap(m_data, m_size, static_cast<size_t>(newSize), MREMAP_MAYMOVE);
    if (data == MAP_FAILED) {
        LM_WARNING(lingmoAurora).printf(
                "Failed to grow a shm pool to %d bytes: %s", newSize, strerror(errno));
        return false;
    }

    m_data = data;
    m_size = static_cast<size_t>(newSize);
    return true;
}

std::shared_ptr<ShmBuffer> ShmPool::createBuffer(const base::Size &size, int32_t offset,
                                                 int32_t stride, render::PixelFormat format,
                                                 ShmBufferError *error)
{
    const auto fail = [error](ShmBufferError code) {
        if (error)
            *error = code;
        return nullptr;
    };

    if (error)
        *error = ShmBufferError::None;

    if (size.width() <= 0 || size.height() <= 0 || stride <= 0 || offset < 0)
        return fail(ShmBufferError::InvalidStride);

    // Everything below is in int64 so that a hostile width/stride/offset cannot
    // wrap into a range that looks like it fits.
    const int64_t bytesPerRow = static_cast<int64_t>(size.width()) * 4;
    if (static_cast<int64_t>(stride) < bytesPerRow)
        return fail(ShmBufferError::InvalidStride);

    const int64_t last = static_cast<int64_t>(offset)
            + static_cast<int64_t>(stride) * (static_cast<int64_t>(size.height()) - 1)
            + bytesPerRow;
    if (last > static_cast<int64_t>(m_size))
        return fail(ShmBufferError::InvalidStride);

    LM_DEBUG(lingmoAurora).printf(
            "Created shm buffer %dx%d stride %d offset %d format %d", size.width(), size.height(),
            stride, offset, static_cast<int>(format));

    return std::make_shared<ShmBuffer>(shared_from_this(), size, offset, stride, format);
}

void ShmPool::beginAccess()
{
    std::call_once(s_sigbusInstalled, [] {
        struct sigaction action {};
        action.sa_sigaction = &ShmPool::handleSigbus;
        // SA_NODEFER so that the handler can re-raise faults that are not ours.
        action.sa_flags = SA_SIGINFO | SA_NODEFER;
        sigemptyset(&action.sa_mask);
        sigaction(SIGBUS, &action, &s_previousSigbusAction);
    });

    assert(t_access.depth == 0 || t_access.pool == this);
    t_access.pool = this;
    ++t_access.depth;
}

bool ShmPool::endAccess()
{
    assert(t_access.depth > 0 && t_access.pool == this);
    if (--t_access.depth == 0)
        t_access.pool = nullptr;
    return !m_faulted;
}

// Async-signal context: nothing here may allocate, lock or log.
void ShmPool::handleSigbus(int, siginfo_t *info, void *)
{
    ShmPool *pool = t_access.pool;
    const auto *address = static_cast<const uint8_t *>(info->si_addr);
    const bool ours = pool && address >= pool->data() && address < pool->data() + pool->m_size;

    // Cover the whole pool with private zero pages so the read that faulted,
    // and every later one, completes. The client's file is never touched.
    if (!ours
        || mmap(pool->m_data, pool->m_size, PROT_READ, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS,
                -1, 0)
                == MAP_FAILED) {
        // Not a pool fault: hand it back to whoever handled SIGBUS before us,
        // which by default terminates the process.
        sigaction(SIGBUS, &s_previousSigbusAction, nullptr);
        raise(SIGBUS);
        return;
    }

    pool->m_faulted = 1;
}

} // namespace Lingmo::Compositor

namespace Lingmo::Compositor {

// Both the pool range and the file behind it, which the client may have
// truncated since create_pool.
bool ShmPool::fileCovers(size_t offset, size_t length) const
{
    if (offset > m_size || length > m_size - offset)
        return false;
    struct stat status {};
    return fstat(m_fd, &status) == 0 && status.st_size >= 0
            && offset + length <= uint64_t(status.st_size);
}

bool ShmPool::writeVectors(size_t offset, std::vector<iovec> vectors) const
{
    size_t index = 0;
    while (index < vectors.size()) {
        const int count = int(std::min<size_t>(vectors.size() - index, IOV_MAX));
        ssize_t written = pwritev(m_fd, &vectors[index], count, off_t(offset));
        if (written < 0 && errno == EINTR)
            continue;
        if (written <= 0)
            return false;
        offset += size_t(written);
        // Drop what went out, trimming a vector that only partly did.
        while (written > 0) {
            iovec &vector = vectors[index];
            if (size_t(written) >= vector.iov_len) {
                written -= ssize_t(vector.iov_len);
                ++index;
            } else {
                vector.iov_base = static_cast<uint8_t *>(vector.iov_base) + written;
                vector.iov_len -= size_t(written);
                written = 0;
            }
        }
    }
    return true;
}

bool ShmPool::write(size_t offset, const void *data, size_t length) const
{
    if (!fileCovers(offset, length))
        return false;
    return writeVectors(offset, {iovec {const_cast<void *>(data), length}});
}

bool ShmPool::writeRows(size_t offset, size_t stride, size_t rows, size_t rowBytes,
                        const uint8_t *data, size_t dataStride) const
{
    if (!rows)
        return true;
    if (stride < rowBytes || dataStride < rowBytes)
        return false;
    size_t span = 0;
    if (__builtin_mul_overflow(rows - 1, stride, &span)
        || __builtin_add_overflow(span, rowBytes, &span) || !fileCovers(offset, span))
        return false;

    if (stride == rowBytes && dataStride == rowBytes)
        return writeVectors(offset, {iovec {const_cast<uint8_t *>(data), span}});

    // Padding between rows belongs to the client and stays as it is, which
    // rules out one contiguous write; rows then go out one at a time.
    for (size_t row = 0; row < rows; ++row) {
        if (!writeVectors(offset + row * stride,
                          {iovec {const_cast<uint8_t *>(data + row * dataStride), rowBytes}}))
            return false;
    }
    return true;
}

} // namespace Lingmo::Compositor
