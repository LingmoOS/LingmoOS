// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <type_traits>
#include <unistd.h>

#include "lingmo/files/operation_context.h"
#include "path_utils_p.h"

namespace Lingmo::Files::internal {

struct Fd
{
    int value;

    explicit Fd(int fd)
        : value(fd)
    {
    }

    ~Fd()
    {
        if (value >= 0)
            ::close(value);
    }

    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;
};

inline void check(int result, const std::string& path)
{
    if (result < 0)
        throw Failure(errno, path);
}

inline void check(ssize_t result, const std::string& path)
{
    if (result < 0)
        throw Failure(errno, path);
}

inline const std::string& native(const std::string& path)
{
    if (path.empty() || path.find('\0') != std::string::npos)
        throw Failure(EINVAL, path);

    return path;
}

struct Parent
{
    Fd fd;
    std::string name;

    explicit Parent(const std::string& path)
        : fd(::open(native(absolutePathOf(path)).c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC))
        , name(native(fileNameOf(path)))
    {
        check(fd.value, path);

        if (name == "." || name == ".." || cleanPath(path) == "/")
            throw Failure(EINVAL, path);
    }
};

template<class Visitor>
void children(int fd, const std::string& path, Visitor visitor)
{
    int duplicate = ::openat(fd, ".", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    check(duplicate, path);

    DIR* raw = ::fdopendir(duplicate);
    if (!raw) {
        ::close(duplicate);
        throw Failure(errno, path);
    }

    auto closeDir = [](DIR* value) {
        ::closedir(value);
    };
    std::unique_ptr<DIR, decltype(closeDir)> dir(raw, closeDir);

    while (true) {
        errno = 0;
        auto* entry = ::readdir(dir.get());
        if (!entry) {
            check(errno ? -1 : 0, path);
            break;
        }

        const std::string name(entry->d_name);
        if (name == "." || name == "..")
            continue;

        if constexpr (std::is_invocable_v<Visitor, const std::string&>)
            visitor(name);
        else
            visitor(name, path + '/' + name);
    }
}

void eraseAt(int parent, const std::string& name, const std::string& path, OperationContext& ctx,
             bool cleanup = false);

} // namespace Lingmo::Files::internal
