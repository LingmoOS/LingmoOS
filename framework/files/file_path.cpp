// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/files/file_path.h"

#include <cerrno>

#include "lingmo/files/error.h"

namespace Lingmo::Files {

FilePath::FilePath(std::string bytes)
    : m_bytes(std::move(bytes))
{
    if (m_bytes.empty() || m_bytes.find('\0') != std::string::npos)
        throw Failure(EINVAL, {});
}

FilePath FilePath::parent() const
{
    std::string path = m_bytes;
    while (path.size() > 1 && path.back() == '/')
        path.pop_back();

    const size_t slash = path.rfind('/');
    return FilePath(slash == std::string::npos ? std::string(".") : slash == 0 ? std::string("/") : path.substr(0, slash));
}

FilePath FilePath::child(const std::string& name) const
{
    if (name.empty() || name == "." || name == ".." || name.find('/') != std::string::npos
        || name.find('\0') != std::string::npos)
        throw Failure(EINVAL, {});

    return FilePath(m_bytes + (m_bytes.back() == '/' ? "" : "/") + name);
}

} // namespace Lingmo::Files
