// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "durationprobe.h"
#include "mpvwait.h"
#include "urls.h"

#include <mpv/client.h>

#include <chrono>
#include <clocale>

using namespace Lingmo;

DurationProbe::DurationProbe()
{
    if (base::EventLoop* loop = base::EventLoop::current())
        m_loop = loop->handle();
    m_weak = m_weakFactory.getWeakPtr();
}

DurationProbe::~DurationProbe()
{
    {
        std::lock_guard lock(m_mutex);
        m_stopping = true;
        if (m_activeMpv)
            mpv_wakeup(m_activeMpv);
    }
    m_wake.notify_all();
    if (m_thread.joinable())
        m_thread.join();
}

void DurationProbe::probe(const std::vector<std::string>& urls)
{
    std::deque<std::string> queue;
    for (const std::string& url : urls) {
        if (urls::isLocalFile(url))
            queue.push_back(url);
    }
    bool failed = false;
    {
        std::lock_guard lock(m_mutex);
        m_queue = queue;
        failed = m_failed;
    }
    m_wake.notify_all();
    if (!m_thread.joinable() && !failed && !queue.empty()) {
        // mpv needs the C locale for numbers, and the queue may come before the player.
        std::setlocale(LC_NUMERIC, "C");
        m_thread = std::thread([this] { run(); });
    }
}

void DurationProbe::run()
{
    mpv_handle* mpv = mpv_create();
    const auto fail = [this] {
        std::lock_guard lock(m_mutex);
        m_failed = true;
    };
    if (!mpv)
        return fail();
    const auto setOption = [mpv](const char* name, const char* value) {
        mpv_set_option_string(mpv, name, value);
    };
    // With every track off mpv ends the file before it counts as loaded, so
    // tracks stay selected and the null outputs skip what decoding they can.
    setOption("vo", "null");
    setOption("ao", "null");
    setOption("sid", "no");
    setOption("hwdec", "no");
    setOption("vd-lavc-skipframe", "all");
    setOption("vd-lavc-skipidct", "all");
    setOption("pause", "yes");
    setOption("idle", "yes");
    setOption("cache", "no");
    setOption("osd-level", "0");
    setOption("load-scripts", "no");
    setOption("ytdl", "no");
    if (mpv_initialize(mpv) < 0) {
        mpv_terminate_destroy(mpv);
        return fail();
    }
    {
        std::lock_guard lock(m_mutex);
        m_activeMpv = mpv;
    }

    const auto stopping = [this] {
        std::lock_guard lock(m_mutex);
        return m_stopping;
    };

    for (;;) {
        std::string url;
        {
            std::unique_lock lock(m_mutex);
            m_wake.wait(lock, [this] { return m_stopping || !m_queue.empty(); });
            if (m_stopping)
                break;
            url = std::move(m_queue.front());
            m_queue.pop_front();
        }

        const std::string path = urls::toLocalFile(url);
        const char* load[] = {"loadfile", path.c_str(), nullptr};
        double seconds = 0;
        if (mpv_command(mpv, load) >= 0) {
            bool ended = false;
            using Clock = std::chrono::steady_clock;
            const Clock::time_point deadline = Clock::now() + std::chrono::seconds(5);
            while (Clock::now() < deadline && !stopping()) {
                const double left = std::chrono::duration<double>(deadline - Clock::now()).count();
                const mpv_event_id event = mpv_wait_event(mpv, left)->event_id;
                if (event == MPV_EVENT_FILE_LOADED) {
                    mpv_get_property(mpv, "duration", MPV_FORMAT_DOUBLE, &seconds);
                    break;
                }
                if (event == MPV_EVENT_END_FILE) {
                    ended = true;
                    break;
                }
            }
            // The next loadfile would otherwise see this file's end first.
            const char* stop[] = {"stop", nullptr};
            if (!ended && mpv_command(mpv, stop) >= 0)
                waitForMpvEvent(mpv, MPV_EVENT_END_FILE, 2000, stopping);
        }
        if (stopping())
            break;
        m_loop.post([weak = m_weak, url, seconds] {
            if (weak)
                weak->found.emit(url, seconds);
        });
    }

    {
        std::lock_guard lock(m_mutex);
        m_activeMpv = nullptr;
    }
    mpv_terminate_destroy(mpv);
}
