// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/common/file_thread.h"

#include <future>
#include <memory>

namespace Lingmo::Shell {

base::ThreadPool &fileThread()
{
    static base::ThreadPool pool(1);
    return pool;
}

void waitForFileThread()
{
    auto done = std::make_shared<std::promise<void>>();
    std::future<void> finished = done->get_future();
    fileThread().post([done] { done->set_value(); });
    finished.wait();
}

void writeSettings(const std::string &path, std::function<void(base::Settings &settings)> change)
{
    fileThread().post([path, change = std::move(change)] {
        base::Settings settings(path);
        change(settings);
        settings.sync();
    });
}

} // namespace Lingmo::Shell
