// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "session.h"
#include "test_support.h"

#include "base/elapsed_timer.h"
#include "base/event_loop.h"
#include "base/file.h"

#ifdef TEXT_EDITOR_UI_TESTS
#include "ui/application.h"
#endif

#include <gtest/gtest.h>

#include <filesystem>
#include <poll.h>

static Lingmo::base::EventLoop *s_loop = nullptr;
static std::string s_directory;

const std::string &TestSupport::directory()
{
    return s_directory;
}

void TestSupport::wait(int ms)
{
    waitUntil([] { return false; }, ms);
}

bool TestSupport::waitUntil(const std::function<bool()> &done, int timeoutMs)
{
    Lingmo::base::ElapsedTimer timer;
    timer.start();
    while (!done()) {
        const int64_t left = timeoutMs - timer.elapsedMs();
        if (left <= 0)
            return false;
        pollfd descriptor{s_loop->fd(), POLLIN, 0};
        ::poll(&descriptor, 1, int(std::min<int64_t>(left, 20)));
        s_loop->dispatch();
    }
    return true;
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
#ifdef TEXT_EDITOR_UI_TESTS
    Lingmo::ui::Application application(argc, argv, Lingmo::ui::Application::Platform::Headless);
    s_loop = &application.loop();
#else
    Lingmo::base::EventLoop loop;
    s_loop = &loop;
#endif
    Lingmo::base::file::TemporaryDirectory scratch(std::filesystem::temp_directory_path().string(), "text-editor-test");
    if (!scratch.isValid())
        return 1;
    s_directory = scratch.path();
    Session::setDataDirectory(s_directory + "/data");
    return RUN_ALL_TESTS();
}
