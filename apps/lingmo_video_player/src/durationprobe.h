// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/event_loop.h"
#include "base/signal.h"
#include "base/weak_ptr.h"

#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct mpv_handle;

// Reads the length of queued local files with a silent mpv on a thread of its
// own, stopping each file as soon as it has loaded.
class DurationProbe
{
public:
    DurationProbe();
    ~DurationProbe();

    DurationProbe(const DurationProbe&) = delete;
    DurationProbe& operator=(const DurationProbe&) = delete;

    // Replaces what is still to be read (URLs); streams are left out.
    void probe(const std::vector<std::string>& urls);

    // seconds is 0 when the file has no length or could not be opened.
    Lingmo::base::Signal<const std::string&, double> found;

private:
    void run();

    Lingmo::base::LoopHandle m_loop;
    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_wake;
    // Guarded by m_mutex.
    bool m_stopping = false;
    // mpv could not start; the thread is not started again.
    bool m_failed = false;
    std::deque<std::string> m_queue;
    mpv_handle* m_activeMpv = nullptr;

    Lingmo::base::WeakPtr<DurationProbe> m_weak;
    Lingmo::base::WeakPtrFactory<DurationProbe> m_weakFactory{this};
};
