// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_CORE_MALLOC_POLICY_H_
#define LINGMO_CORE_MALLOC_POLICY_H_

#include <malloc.h>

namespace Lingmo::Core {

// Before any thread starts. glibc otherwise raises its mmap threshold to the
// largest block freed (a decoded wallpaper or icon sheet), so later large
// blocks come from the heap and stay resident once freed, and it gives every
// worker thread an arena of its own that keeps what the thread freed.
inline void applyMallocPolicy()
{
    mallopt(M_MMAP_THRESHOLD, 128 * 1024);
    mallopt(M_ARENA_MAX, 4);
}

} // namespace Lingmo::Core

#endif // LINGMO_CORE_MALLOC_POLICY_H_
