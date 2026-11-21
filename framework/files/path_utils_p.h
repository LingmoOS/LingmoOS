// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

// Path text handling on bytes.
namespace Lingmo::Files::internal {

// The path with no repeated slashes, "." or "..", nor a trailing slash.
inline std::string cleanPath(std::string_view path)
{
    if (path.empty())
        return {};

    const bool absolute = path.front() == '/';
    std::vector<std::string_view> parts;
    size_t start = 0;

    while (start <= path.size()) {
        size_t end = path.find('/', start);
        if (end == std::string_view::npos)
            end = path.size();

        const std::string_view part = path.substr(start, end - start);
        if (part.empty() || part == ".") {
        } else if (part == "..") {
            if (!parts.empty() && parts.back() != "..")
                parts.pop_back();
            else if (!absolute)
                parts.push_back(part);
        } else {
            parts.push_back(part);
        }
        start = end + 1;
    }

    std::string result = absolute ? "/" : "";
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i)
            result += '/';
        result += parts[i];
    }

    return result.empty() ? std::string(".") : result;
}

inline std::string currentDirectory()
{
    char buffer[PATH_MAX];
    return ::getcwd(buffer, sizeof(buffer)) ? std::string(buffer) : std::string();
}

// The path made absolute against the working directory, then cleaned.
inline std::string absoluteFilePath(std::string_view path)
{
    if (!path.empty() && path.front() == '/')
        return cleanPath(path);

    return cleanPath(currentDirectory() + '/' + std::string(path));
}

// The file name: what follows the last slash.
inline std::string fileNameOf(std::string_view path)
{
    const size_t slash = path.rfind('/');
    return std::string(slash == std::string_view::npos ? path : path.substr(slash + 1));
}

// The absolute folder the path names a file in.
inline std::string absolutePathOf(std::string_view path)
{
    std::string full = !path.empty() && path.front() == '/' ? std::string(path) : currentDirectory() + '/' + std::string(path);
    const size_t slash = full.rfind('/');
    if (slash == 0)
        return "/";
    return cleanPath(std::string_view(full).substr(0, slash));
}

// The path with links and dots resolved; empty when the
// file does not exist.
inline std::string canonicalPath(const std::string& path)
{
    char* resolved = ::realpath(path.c_str(), nullptr);
    if (!resolved)
        return {};
    std::string result(resolved);
    std::free(resolved);
    return result;
}

inline std::string joined(const std::string& directory, std::string_view name)
{
    return !directory.empty() && directory.back() == '/' ? directory + std::string(name)
                                                         : directory + '/' + std::string(name);
}

// Creates every missing folder on the way; errno says why it failed.
inline bool makePath(const std::string& path)
{
    const std::string clean = cleanPath(path);
    for (size_t slash = clean.find('/', 1);; slash = clean.find('/', slash + 1)) {
        const std::string part = clean.substr(0, slash);
        if (::mkdir(part.c_str(), 0777) != 0 && errno != EEXIST)
            return false;
        if (slash == std::string::npos)
            break;
    }

    struct stat st;
    return ::stat(clean.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

} // namespace Lingmo::Files::internal
