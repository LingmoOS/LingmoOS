// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/event_loop.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <poll.h>
#include <QString>

namespace SessionTest {

// Runs the loop until the condition holds or the time is up.
inline bool runUntil(Lingmo::Base::EventLoop &loop,
                     const std::function<bool()> &condition,
                     int milliseconds = 5000)
{
    const auto deadline = std::chrono::steady_clock::now()
                        + std::chrono::milliseconds(milliseconds);
    while (!condition()) {
        if (std::chrono::steady_clock::now() >= deadline)
            return condition();
        pollfd fd{loop.fd(), POLLIN, 0};
        ::poll(&fd, 1, 10);
        loop.dispatch();
    }
    return true;
}

// Runs the loop for a fixed duration, regardless of what happens.
inline void runFor(Lingmo::Base::EventLoop &loop, int milliseconds)
{
    runUntil(loop, [] { return false; }, milliseconds);
}

inline QString temporaryParent()
{
    return QString::fromStdString(
        std::filesystem::temp_directory_path().string());
}

inline bool writeFile(const QString &path, const QString &contents)
{
    std::filesystem::create_directories(
        std::filesystem::path(path.toStdString()).parent_path());
    std::ofstream file(path.toStdString(), std::ios::binary | std::ios::trunc);
    file << contents.toStdString();
    return bool(file);
}

inline QString readFile(const QString &path)
{
    std::ifstream file(path.toStdString(), std::ios::binary);
    return QString::fromStdString(
        std::string(std::istreambuf_iterator<char>(file),
                    std::istreambuf_iterator<char>()));
}

} // namespace SessionTest
