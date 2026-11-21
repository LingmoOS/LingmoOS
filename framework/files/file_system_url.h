// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "lingmo/files/url.h"

namespace Lingmo::Files {

// A URL the local file system serves: file:// paths, and trash:/, which lists
// every trash. trash:/<id>/<name> is an item at the top of one: id is "home"
// for the home trash, else it names a mounted volume's trash. Throws ENOTSUP
// for anything else and EINVAL for a malformed trash URL.
class FileSystemUrl
{
public:
    explicit FileSystemUrl(Url url);

    const Url& url() const { return m_url; }

    std::string localPath() const;
    // ~/.local/share/Trash.
    static std::string trashRoot();

private:
    Url m_url;
};

} // namespace Lingmo::Files
