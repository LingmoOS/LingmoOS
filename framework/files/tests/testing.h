// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <optional>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/file.h"
#include "base/timer.h"
#include "lingmo/files/error.h"

namespace Lingmo::Files::testing {

using namespace std::chrono_literals;

// A folder of its own under parent, removed with everything in it, whatever
// its permissions, when the object goes.
class TemporaryDirectory
{
public:
    explicit TemporaryDirectory(const std::string& parent = "/tmp")
    {
        std::string pattern = parent + "/lingmo-files-XXXXXX";
        if (::mkdtemp(pattern.data()))
            m_path = pattern;
    }

    ~TemporaryDirectory()
    {
        if (!m_path.empty())
            removeAll(m_path);
    }

    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

    bool isValid() const { return !m_path.empty(); }
    const std::string& path() const { return m_path; }
    std::string filePath(const std::string& name) const { return m_path + '/' + name; }

    static void removeAll(const std::string& path)
    {
        struct stat st;
        if (::lstat(path.c_str(), &st) != 0)
            return;

        if (S_ISDIR(st.st_mode)) {
            ::chmod(path.c_str(), 0700);
            if (DIR* dir = ::opendir(path.c_str())) {
                while (const dirent* entry = ::readdir(dir)) {
                    const std::string name = entry->d_name;
                    if (name != "." && name != "..")
                        removeAll(path + '/' + name);
                }
                ::closedir(dir);
            }
            ::rmdir(path.c_str());
        } else {
            ::unlink(path.c_str());
        }
    }

private:
    std::string m_path;
};

inline void writeFile(const std::string& path, std::string_view data = "contents")
{
    const int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (fd < 0)
        throw Failure(errno, path);

    const ssize_t written = ::write(fd, data.data(), data.size());
    ::close(fd);
    if (written != ssize_t(data.size()))
        throw Failure(EIO, path);
}

inline std::string readFile(const std::string& path)
{
    const std::optional<std::string> contents = base::file::read(path);
    if (!contents)
        throw Failure(EIO, path);
    return *contents;
}

inline bool exists(const std::string& path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0;
}

inline bool isDirectory(const std::string& path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

inline bool isSymbolicLink(const std::string& path)
{
    struct stat st;
    return ::lstat(path.c_str(), &st) == 0 && S_ISLNK(st.st_mode);
}

inline int64_t fileSize(const std::string& path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 ? int64_t(st.st_size) : -1;
}

inline bool makePath(const std::string& path)
{
    for (size_t slash = path.find('/', 1);; slash = path.find('/', slash + 1)) {
        if (::mkdir(path.substr(0, slash).c_str(), 0777) != 0 && errno != EEXIST)
            return false;
        if (slash == std::string::npos)
            return true;
    }
}

// The entries of a folder, dot files included.
inline std::vector<std::string> entries(const std::string& path)
{
    std::vector<std::string> result;
    if (DIR* dir = ::opendir(path.c_str())) {
        while (const dirent* entry = ::readdir(dir)) {
            const std::string name = entry->d_name;
            if (name != "." && name != "..")
                result.push_back(name);
        }
        ::closedir(dir);
    }
    return result;
}

template<class F>
int errorCode(F action)
{
    try {
        action();
    } catch (const Failure& failure) {
        return failure.error.code;
    }

    return 0;
}

// Runs the loop for a while.
inline void wait(base::EventLoop& loop, std::chrono::milliseconds duration)
{
    base::Timer timer(&loop);
    timer.setSingleShot(true);
    timer.onTimeout = [&] {
        loop.quit();
    };
    timer.start(duration);
    loop.run();
}

// Runs the loop until the condition holds, at most timeout.
template<class Condition>
bool waitFor(base::EventLoop& loop, Condition condition, std::chrono::milliseconds timeout = 5000ms)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (!condition()) {
        if (std::chrono::steady_clock::now() > deadline)
            return condition();
        wait(loop, 10ms);
    }
    return true;
}

} // namespace Lingmo::Files::testing
