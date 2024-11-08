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

#ifndef CPR_ASYNC_H
#define CPR_ASYNC_H

#include "singleton.h"
#include "threadpool.h"

namespace cpr {

class GlobalThreadPool : public ThreadPool {
    CPR_SINGLETON_DECL(GlobalThreadPool)
  protected:
    GlobalThreadPool() = default;

  public:
    ~GlobalThreadPool() override = default;
};

/**
 * Return a future, calling future.get() will wait task done and return RetType.
 * async(fn, args...)
 * async(std::bind(&Class::mem_fn, &obj))
 * async(std::mem_fn(&Class::mem_fn, &obj))
 **/
template <class Fn, class... Args>
auto async(Fn&& fn, Args&&... args) -> std::future<decltype(fn(args...))> {
    return GlobalThreadPool::GetInstance()->Submit(std::forward<Fn>(fn), std::forward<Args>(args)...);
}

class async {
  public:
    static void startup(size_t min_threads = CPR_DEFAULT_THREAD_POOL_MIN_THREAD_NUM, size_t max_threads = CPR_DEFAULT_THREAD_POOL_MAX_THREAD_NUM, std::chrono::milliseconds max_idle_ms = CPR_DEFAULT_THREAD_POOL_MAX_IDLE_TIME) {
        GlobalThreadPool* gtp = GlobalThreadPool::GetInstance();
        if (gtp->IsStarted()) {
            return;
        }
        gtp->SetMinThreadNum(min_threads);
        gtp->SetMaxThreadNum(max_threads);
        gtp->SetMaxIdleTime(max_idle_ms);
        gtp->Start();
    }

    static void cleanup() {
        GlobalThreadPool::ExitInstance();
    }
};

} // namespace cpr

#endif
