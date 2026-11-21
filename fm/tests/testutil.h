// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/event_loop.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <poll.h>
#include <string>

namespace fmtest {

// Runs the loop until the condition holds or the time is up; the condition's last answer.
inline bool runUntil(Lingmo::base::EventLoop& loop, const std::function<bool()>& condition,
                     int milliseconds = 5000)
{
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds);
    while (!condition()) {
        if (std::chrono::steady_clock::now() >= deadline)
            return condition();
        pollfd fd {loop.fd(), POLLIN, 0};
        ::poll(&fd, 1, 10);
        loop.dispatch();
    }
    return true;
}

// Runs the loop for a while, whatever happens.
inline void runFor(Lingmo::base::EventLoop& loop, int milliseconds)
{
    runUntil(loop, [] { return false; }, milliseconds);
}

// A directory of its own under the temporary directory, removed with the object.
class TemporaryDir
{
public:
    TemporaryDir()
    {
        std::string pattern = (std::filesystem::temp_directory_path() / "fmtest-XXXXXX").string();
        if (::mkdtemp(pattern.data()))
            m_path = pattern;
    }

    ~TemporaryDir()
    {
        std::error_code error;
        if (!m_path.empty())
            std::filesystem::remove_all(m_path, error);
    }

    TemporaryDir(const TemporaryDir&) = delete;
    TemporaryDir& operator=(const TemporaryDir&) = delete;

    bool isValid() const { return !m_path.empty(); }
    const std::string& path() const { return m_path; }
    std::string filePath(const std::string& name) const { return m_path + "/" + name; }

private:
    std::string m_path;
};

inline bool writeFile(const std::string& path, const std::string& contents = "text\n")
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << contents;
    return bool(file);
}

} // namespace fmtest
