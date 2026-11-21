// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "base/elapsed_timer.h"

namespace Lingmo::Files::internal {

/**
 * The MIME type of a file on disk from its name and contents, matched
 * without a lock so listings can detect types on several threads.
 *
 * The globs and magic rules come from the mime.cache files, in the
 * directories' order, including the globs a local directory deletes. Where a directory holds only XML packages, the
 * toolkit's detection answers.
 */
class MimeSniffer
{
public:
    static MimeSniffer* self();

    // The canonical type name.
    std::string mimeTypeForFile(const std::string& path);
    // Inheritance and the canonical name, aliases resolved, from the same
    // directories; the name is empty for an unknown type.
    bool inherits(const std::string& type, const std::string& ancestor);
    std::string canonicalName(const std::string& name);

private:
    struct Database;

    std::shared_ptr<const Database> database();

    std::mutex m_mutex;
    base::ElapsedTimer m_lastCheck;
    std::shared_ptr<const Database> m_database;
};

// The XDG data directories' mime folders that exist, the user's first.
std::vector<std::string> mimeDirectories();
// ~/.local/share, where the user's MIME types and thumbnailers are.
std::string userDataDirectory();
// Tests use a data directory of their own before the system ones; empty
// restores the default.
void setUserDataDirectory(const std::string& path);

} // namespace Lingmo::Files::internal
