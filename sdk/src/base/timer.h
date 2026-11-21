// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <functional>

namespace Lingmo::base {

class EventLoop;

// A timer on a loop, stopped when it goes.
class Timer
{
public:
    // On the calling thread's loop when none is given.
    explicit Timer(EventLoop* loop = nullptr);
    ~Timer();

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    // May destroy or restart the timer.
    std::function<void()> onTimeout;

    std::chrono::nanoseconds interval() const { return m_interval; }
    // Takes effect at the next start.
    void setInterval(std::chrono::nanoseconds interval) { m_interval = interval; }

    bool isSingleShot() const { return m_singleShot; }
    // Read at every expiry, so it also changes a running timer.
    void setSingleShot(bool singleShot) { m_singleShot = singleShot; }

    // Starts or restarts the timer.
    void start();
    void start(std::chrono::nanoseconds interval);
    void stop();
    bool isActive() const { return m_id != 0; }

private:
    EventLoop& m_loop;
    std::chrono::nanoseconds m_interval{0};
    bool m_singleShot = false;
    int m_id = 0;
};

} // namespace Lingmo::base
