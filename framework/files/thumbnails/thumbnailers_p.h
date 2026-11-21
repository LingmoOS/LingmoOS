// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "base/elapsed_timer.h"
#include "base/geometry.h"
#include "ui/bitmap.h"

namespace Lingmo::Files::internal {

/**
 * The external thumbnailers installed as `thumbnailers/*.thumbnailer` in the
 * XDG data directories, as other desktops use them. Entries in earlier
 * directories win, and installing or removing one is picked up within seconds.
 */
class Thumbnailers
{
public:
    static Thumbnailers* self();

    bool contains(const std::string& mimeType);

    // Runs the thumbnailer for mimeType on path; null on failure or timeout.
    ui::Bitmap generate(const std::string& path, const std::string& mimeType, base::Size size);

private:
    std::vector<std::string> command(const std::string& mimeType);
    void refreshLocked();

    std::mutex m_mutex;
    base::ElapsedTimer m_checked;
    std::vector<std::pair<std::string, int64_t>> m_directories;
    std::map<std::string, std::vector<std::string>> m_commands;
};

} // namespace Lingmo::Files::internal
