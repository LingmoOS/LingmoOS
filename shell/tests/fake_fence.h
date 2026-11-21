// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_TESTS_FAKE_FENCE_H_
#define LINGMO_TESTS_FAKE_FENCE_H_

#include <gtest/gtest.h>

#include <fcntl.h>
#include <unistd.h>

#include "render/sync_fence.h"

namespace Lingmo::Testsupport {

// A pipe read end stands in for a sync_file: poll() reports it unreadable
// until something is written, which is exactly how an unsignalled fence
// behaves.
class FakeFence
{
public:
    FakeFence()
    {
        if (::pipe2(m_fds, O_CLOEXEC) != 0)
            m_fds[0] = m_fds[1] = -1;
    }

    FakeFence(const FakeFence &) = delete;
    FakeFence &operator=(const FakeFence &) = delete;

    ~FakeFence()
    {
        if (m_fds[1] >= 0)
            ::close(m_fds[1]);
        if (m_fds[0] >= 0)
            ::close(m_fds[0]);
    }

    render::SyncFence take()
    {
        const int fd = m_fds[0];
        m_fds[0] = -1;
        return render::SyncFence(ScopedFd(fd));
    }

    void signal()
    {
        if (m_fds[1] >= 0) {
            const char byte = 1;
            EXPECT_EQ(::write(m_fds[1], &byte, 1), 1);
        }
    }

private:
    int m_fds[2] = {-1, -1};
};

} // namespace Lingmo::Testsupport

#endif // LINGMO_TESTS_FAKE_FENCE_H_
