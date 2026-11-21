// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_TESTS_SHM_TEST_UTIL_H_
#define LINGMO_TESTS_SHM_TEST_UTIL_H_

#include <sys/mman.h>
#include <unistd.h>

#include <cstddef>
#include <memory>

#include "compositor/shm_pool.h"

namespace Lingmo::Testsupport {

// A memfd of `size` bytes, owned by the caller.
inline int makeMemfd(size_t size)
{
    const int fd = memfd_create("lingmo-test", MFD_CLOEXEC);
    if (fd < 0)
        return -1;
    if (ftruncate(fd, static_cast<off_t>(size)) != 0) {
        close(fd);
        return -1;
    }
    return fd;
}

inline std::shared_ptr<compositor::ShmPool> makePool(size_t size)
{
    const int fd = makeMemfd(size);
    return fd < 0 ? nullptr : compositor::ShmPool::create(fd, static_cast<int32_t>(size));
}

} // namespace Lingmo::Testsupport

#endif // LINGMO_TESTS_SHM_TEST_UTIL_H_
