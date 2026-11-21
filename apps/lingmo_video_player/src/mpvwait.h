// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>

#include <mpv/client.h>

// Waits up to timeout milliseconds for event on a worker's mpv; false on an
// error, the end of the file, or when cancelled says so.
template<typename Cancelled>
static bool waitForMpvEvent(mpv_handle* mpv, mpv_event_id wanted, int timeout, Cancelled cancelled)
{
    using Clock = std::chrono::steady_clock;
    const Clock::time_point deadline = Clock::now() + std::chrono::milliseconds(timeout);
    while (Clock::now() < deadline && !cancelled()) {
        const double left = std::chrono::duration<double>(deadline - Clock::now()).count();
        mpv_event* event = mpv_wait_event(mpv, left);
        if (event->event_id == wanted)
            return true;
        if (event->event_id == MPV_EVENT_END_FILE || event->event_id == MPV_EVENT_SHUTDOWN)
            return false;
    }
    return false;
}
