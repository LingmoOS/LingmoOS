/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "services/threadpool/threadpool.h"

namespace cpr {

ThreadPool::ThreadPool(size_t min_threads, size_t max_threads, std::chrono::milliseconds max_idle_ms) : min_thread_num(min_threads), max_thread_num(max_threads), max_idle_time(max_idle_ms), status(STOP), cur_thread_num(0), idle_thread_num(0) {}

ThreadPool::~ThreadPool() {
    Stop();
}

int ThreadPool::Start(size_t start_threads) {
    if (status != STOP) {
        return -1;
    }
    status = RUNNING;
    if (start_threads < min_thread_num) {
        start_threads = min_thread_num;
    }
    if (start_threads > max_thread_num) {
        start_threads = max_thread_num;
    }
    for (size_t i = 0; i < start_threads; ++i) {
        CreateThread();
    }
    return 0;
}

int ThreadPool::Stop() {
    if (status == STOP) {
        return -1;
    }
    status = STOP;
    task_cond.notify_all();
    for (auto& i : threads) {
        if (i.thread->joinable()) {
            i.thread->join();
        }
    }
    threads.clear();
    cur_thread_num = 0;
    idle_thread_num = 0;
    return 0;
}

int ThreadPool::Pause() {
    if (status == RUNNING) {
        status = PAUSE;
    }
    return 0;
}

int ThreadPool::Resume() {
    if (status == PAUSE) {
        status = RUNNING;
    }
    return 0;
}

int ThreadPool::Wait() {
    while (true) {
        if (status == STOP || (tasks.empty() && idle_thread_num == cur_thread_num)) {
            break;
        }
        std::this_thread::yield();
    }
    return 0;
}

bool ThreadPool::CreateThread() {
    if (cur_thread_num >= max_thread_num) {
        return false;
    }
    std::thread* thread = new std::thread([this] {
        bool initialRun = true;
        while (status != STOP) {
            while (status == PAUSE) {
                std::this_thread::yield();
            }

            Task task;
            {
                std::unique_lock<std::mutex> locker(task_mutex);
                task_cond.wait_for(locker, std::chrono::milliseconds(max_idle_time), [this]() { return status == STOP || !tasks.empty(); });
                if (status == STOP) {
                    return;
                }
                if (tasks.empty()) {
                    if (cur_thread_num > min_thread_num) {
                        DelThread(std::this_thread::get_id());
                        return;
                    }
                    continue;
                }
                if (!initialRun) {
                    --idle_thread_num;
                }
                task = std::move(tasks.front());
                tasks.pop();
            }
            if (task) {
                task();
                ++idle_thread_num;
                if (initialRun) {
                    initialRun = false;
                }
            }
        }
    });
    AddThread(thread);
    return true;
}

void ThreadPool::AddThread(std::thread* thread) {
    thread_mutex.lock();
    ++cur_thread_num;
    ThreadData data;
    data.thread = std::shared_ptr<std::thread>(thread);
    data.id = thread->get_id();
    data.status = RUNNING;
    data.start_time = time(nullptr);
    data.stop_time = 0;
    threads.emplace_back(data);
    thread_mutex.unlock();
}

void ThreadPool::DelThread(std::thread::id id) {
    const time_t now = time(nullptr);
    thread_mutex.lock();
    --cur_thread_num;
    --idle_thread_num;
    auto iter = threads.begin();
    while (iter != threads.end()) {
        if (iter->status == STOP && now > iter->stop_time) {
            if (iter->thread->joinable()) {
                iter->thread->join();
                iter = threads.erase(iter);
                continue;
            }
        } else if (iter->id == id) {
            iter->status = STOP;
            iter->stop_time = time(nullptr);
        }
        ++iter;
    }
    thread_mutex.unlock();
}

} // namespace cpr
