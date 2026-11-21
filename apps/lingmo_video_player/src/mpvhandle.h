// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/event_loop.h"

#include <atomic>
#include <functional>
#include <mutex>

#include <mpv/client.h>

// The mpv handle outlives the player while a video output still holds its
// render context: that has to be freed (on the output's own thread) before
// the handle goes. mpv's callbacks run on its threads and reach the player's
// loop only through post(), which stops working once the player detaches.
struct MpvHandle
{
    mpv_handle* mpv = nullptr;

    ~MpvHandle()
    {
        if (mpv)
            mpv_terminate_destroy(mpv);
    }

    // From mpv's threads: mpv has events, or a video output has a frame
    // due. Each runs its task on the player's loop once until it clears its
    // flag, and not at all once the player has detached.
    void notifyEvents() { notify(eventsQueued, eventsTask); }
    void notifyFrame() { notify(frameQueued, frameTask); }

    void attach(Lingmo::base::LoopHandle handle, std::function<void()> events)
    {
        std::lock_guard lock(mutex);
        loop = std::move(handle);
        eventsTask = std::move(events);
        attached = true;
    }

    // The video output's frames, on the same loop.
    void setFrameTask(std::function<void()> task)
    {
        std::lock_guard lock(mutex);
        frameTask = std::move(task);
        frameQueued = false;
    }

    // Any thread: runs task on the player's loop while one is attached.
    void post(std::function<void()> task)
    {
        std::lock_guard lock(mutex);
        if (attached)
            loop.post(std::move(task));
    }

    void detach()
    {
        std::lock_guard lock(mutex);
        attached = false;
        eventsTask = nullptr;
        frameTask = nullptr;
    }

    std::atomic<bool> eventsQueued = false;
    std::atomic<bool> frameQueued = false;

private:
    void notify(std::atomic<bool>& queued, const std::function<void()>& task)
    {
        std::lock_guard lock(mutex);
        if (!attached || !task || queued.exchange(true))
            return;
        loop.post(task);
    }

    std::mutex mutex;
    Lingmo::base::LoopHandle loop;
    bool attached = false;
    std::function<void()> eventsTask;
    std::function<void()> frameTask;
};
