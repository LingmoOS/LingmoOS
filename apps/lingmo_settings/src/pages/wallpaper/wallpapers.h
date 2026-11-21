// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

namespace settings::wallpapers {

// The images under backgrounds/lingmoos in the system's data directories.
std::vector<std::string> system();
// Sorted by file name, not by path: a wallpaper library may hold
// subdirectories, and what is read in the grid is the name.
void sortByFileName(std::vector<std::string>& paths);
// The folder's own name, as a section title.
std::string folderTitle(const std::string& folder);

} // namespace settings::wallpapers
