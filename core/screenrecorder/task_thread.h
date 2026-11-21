// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINGMO_RECORDER_TASK_THREAD_H
#define LINGMO_RECORDER_TASK_THREAD_H

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>

// A thread that runs the tasks posted to it one by one, in order.
class TaskThread
{
public:
    TaskThread() = default;
    ~TaskThread() { stop(); }

    TaskThread(const TaskThread &) = delete;
    TaskThread &operator=(const TaskThread &) = delete;

    void start()
    {
        m_thread = std::thread([this] { run(); });
    }

    // Callable from any thread; dropped once the thread is stopping.
    void post(std::function<void()> task)
    {
        {
            std::lock_guard lock(m_mutex);
            if (m_quit)
                return;
            m_tasks.push_back(std::move(task));
        }
        m_wake.notify_one();
    }

    // Lets the task that runs finish, drops the ones waiting, and joins.
    void stop()
    {
        std::deque<std::function<void()>> dropped;
        {
            std::lock_guard lock(m_mutex);
            m_quit = true;
            dropped.swap(m_tasks);
        }
        m_wake.notify_one();
        if (m_thread.joinable())
            m_thread.join();
    }

private:
    void run()
    {
        for (;;) {
            std::function<void()> task;
            {
                std::unique_lock lock(m_mutex);
                m_wake.wait(lock, [this] { return m_quit || !m_tasks.empty(); });
                if (m_quit)
                    return;
                task = std::move(m_tasks.front());
                m_tasks.pop_front();
            }
            task();
        }
    }

    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_wake;
    std::deque<std::function<void()>> m_tasks;
    bool m_quit = false;
};

#endif // LINGMO_RECORDER_TASK_THREAD_H
