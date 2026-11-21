// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/files/file_system_url.h"

#include <cerrno>

#include "base/strings.h"
#include "lingmo/files/error.h"
#include "trash_directory_p.h"

namespace Lingmo::Files {

using namespace Lingmo::Base;

FileSystemUrl::FileSystemUrl(Url url)
    : m_url(std::move(url))
{
    if (!m_url.isValid() || m_url.hasQuery() || m_url.hasFragment() || !m_url.userInfo().empty()
        || m_url.port() != -1 || (!m_url.host().empty() && m_url.host() != "localhost")
        || (!m_url.isLocalFile() && m_url.scheme() != "trash") || !m_url.path().starts_with('/')
        || m_url.path().find('\0') != std::string::npos)
        throw Failure(ENOTSUP, m_url.toString());

    if (m_url.scheme() == "trash") {
        for (const std::string_view part : strings::splitViews(m_url.path(), '/')) {
            if (part == ".." || part == ".")
                throw Failure(EINVAL, m_url.toString());
        }

        if (m_url.path() != "/")
            internal::trashDirectory(m_url);
    }
}

std::string FileSystemUrl::trashRoot()
{
    return internal::homeTrashRoot();
}

std::string FileSystemUrl::localPath() const
{
    if (m_url.isLocalFile())
        return m_url.path();

    if (m_url.path() == "/")
        return trashRoot() + "/files";

    std::string inside;
    const auto trash = internal::trashDirectory(m_url, &inside);
    return trash.path + "/files" + (inside.empty() ? std::string() : '/' + inside);
}

} // namespace Lingmo::Files
