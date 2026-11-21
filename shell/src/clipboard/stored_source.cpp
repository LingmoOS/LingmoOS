// Copyright 2017 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.sommelier file.
//
// Upstream: chromiumos/platform2/vm_tools/sommelier/sommelier.cc (selection handlers)
// Revision: aa8a57908bdcff297e229679dc3240b1b108f57e

#include "clipboard/stored_source.h"

#include <cerrno>
#include <chrono>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <utility>

#include "base/event_loop.h"
#include "base/timer.h"
#include "core/main_loop.h"

namespace Lingmo::Clipboard {

namespace {

// Writes one payload to a receiver without blocking, and deletes itself once
// done, failed, or stalled for ten seconds.
class Writer
{
public:
    Writer(base::EventLoop &loop, int fd, std::string data)
        : m_loop(loop)
        , m_fd(fd)
        , m_data(std::move(data))
        , m_timeout(&loop)
    {
        m_timeout.setSingleShot(true);
        m_timeout.onTimeout = [this] {
            finish();
        };
        m_timeout.start(kTimeout);
        m_watch = m_loop.watch(m_fd, EPOLLOUT, [this](uint32_t) {
            writeAvailable();
        });
    }

    ~Writer()
    {
        if (m_watch)
            m_loop.unwatch(m_watch);
        close(m_fd);
    }

private:
    static constexpr std::chrono::seconds kTimeout {10};

    void writeAvailable()
    {
        const auto count = write(m_fd, m_data.data() + m_offset, m_data.size() - m_offset);
        if (count > 0) {
            m_offset += size_t(count);
            m_timeout.start(kTimeout);
        }
        if (m_offset == m_data.size() || (count < 0 && errno != EAGAIN && errno != EINTR))
            finish();
    }

    // Deleted from the loop, never from inside its own callbacks.
    void finish()
    {
        if (std::exchange(m_finished, true))
            return;
        m_loop.unwatch(std::exchange(m_watch, 0));
        m_timeout.stop();
        m_loop.post([this] {
            delete this;
        });
    }

    base::EventLoop &m_loop;
    const int m_fd;
    const std::string m_data;
    size_t m_offset = 0;
    base::Timer m_timeout;
    int m_watch = 0;
    bool m_finished = false;
};

} // namespace

void StoredSource::send(const std::string &mime, int fd)
{
    auto it = payloads.find(mime);
    if (it == payloads.end() || fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) < 0) {
        close(fd);
        return;
    }

    new Writer(*core::mainLoop(), fd, it->second);
}

} // namespace Lingmo::Clipboard
