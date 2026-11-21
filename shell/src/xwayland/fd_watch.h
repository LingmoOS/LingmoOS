// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_XWAYLAND_FD_WATCH_H_
#define LINGMO_XWAYLAND_FD_WATCH_H_

#include <cstdint>
#include <functional>
#include <sys/epoll.h>
#include <utility>

#include "base/event_loop.h"
#include "core/main_loop.h"

namespace Lingmo::XWayland {

// A descriptor watched on the main loop while enabled; created disabled.
// Disabled means not watched at all: epoll reports a hang-up or an error
// whatever events are asked for. It goes before its descriptor is closed: epoll
// keeps watching a closed descriptor whose file another process still holds.
class FdWatch
{
public:
    FdWatch(int fd, uint32_t events, std::function<void()> ready)
        : m_loop(*core::mainLoop())
        , m_fd(fd)
        , m_events(events)
        , m_ready(std::move(ready))
    {
    }
    ~FdWatch() { setEnabled(false); }

    FdWatch(const FdWatch &) = delete;
    FdWatch &operator=(const FdWatch &) = delete;

    bool isEnabled() const { return m_watch != 0; }
    void setEnabled(bool enabled)
    {
        if (enabled == isEnabled())
            return;
        if (enabled)
            m_watch = m_loop.watch(m_fd, m_events, [this](uint32_t) {
                // A copy: the callback may destroy this watch.
                const auto ready = m_ready;
                ready();
            });
        else
            m_loop.unwatch(std::exchange(m_watch, 0));
    }

private:
    base::EventLoop &m_loop;
    int m_fd;
    uint32_t m_events;
    std::function<void()> m_ready;
    int m_watch = 0;
};

} // namespace Lingmo::XWayland

#endif // LINGMO_XWAYLAND_FD_WATCH_H_
