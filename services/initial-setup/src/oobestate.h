// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <fstream>

namespace oobe {

#ifndef OOBE_SESSION_DESKTOP_FILE
#define OOBE_SESSION_DESKTOP_FILE "/usr/share/wayland-sessions/lingmo-initial-setup.desktop"
#endif

#ifndef OOBE_APPLICATION_DESKTOP_FILE
#define OOBE_APPLICATION_DESKTOP_FILE "/usr/share/applications/lingmo-initial-setup-app.desktop"
#endif

inline std::filesystem::path completionPath(const std::filesystem::path& home)
{
    return home / ".config" / "lingmo" / "oobe.conf";
}

inline bool isComplete(const std::filesystem::path& home)
{
    return std::filesystem::exists(completionPath(home));
}

inline bool markComplete(const std::filesystem::path& home)
{
    const auto path = completionPath(home);
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error)
        return false;

    std::ofstream marker(path, std::ios::trunc);
    if (!marker)
        return false;
    marker << "[Setup]\nComplete=true\n";
    return marker.good();
}

inline bool clearAccountState(const std::filesystem::path& root = "/etc")
{
    std::error_code error;
    return std::filesystem::remove(root / "lingmo-initial-setup-account.conf", error) || !error;
}

// The setup session is only useful before the first login. Remove both
// desktop entries after completion so display managers cannot offer it again.
inline bool removeDesktopEntries()
{
    bool removed = true;
    for (const std::filesystem::path path : {OOBE_SESSION_DESKTOP_FILE,
                                             OOBE_APPLICATION_DESKTOP_FILE}) {
        std::error_code error;
        if (std::filesystem::exists(path, error) && !std::filesystem::remove(path, error))
            removed = false;
        if (error)
            removed = false;
    }
    return removed;
}

} // namespace oobe
