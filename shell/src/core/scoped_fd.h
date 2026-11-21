// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_CORE_SCOPED_FD_H_
#define LINGMO_CORE_SCOPED_FD_H_

#include <unistd.h>

#include <utility>

namespace Lingmo {

// Owning file descriptor. Stands in for base::ScopedFD.
class ScopedFd
{
public:
    ScopedFd() = default;

    explicit ScopedFd(int fd)
        : m_fd(fd)
    {
    }

    ScopedFd(const ScopedFd&) = delete;
    ScopedFd& operator=(const ScopedFd&) = delete;

    ScopedFd(ScopedFd&& other) noexcept
        : m_fd(other.release())
    {
    }

    ScopedFd& operator=(ScopedFd&& other) noexcept
    {
        if (this != &other)
            reset(other.release());
        return *this;
    }

    ~ScopedFd() { reset(); }

    int get() const { return m_fd; }

    bool isValid() const { return m_fd >= 0; }

    int release() { return std::exchange(m_fd, -1); }

    void reset(int fd = -1)
    {
        if (m_fd >= 0 && m_fd != fd)
            ::close(m_fd);
        m_fd = fd;
    }

private:
    int m_fd = -1;
};

} // namespace Lingmo

#endif // LINGMO_CORE_SCOPED_FD_H_
