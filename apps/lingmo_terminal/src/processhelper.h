// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace ProcessHelper
{

// Opens a link or a file with the application the desktop has for it.
bool openUrl(const std::string& url);
// The file manager on a folder, or showing a file in its folder; neither waits.
bool openFileManager(const std::string& path);
bool showInFileManager(const std::string& path);

// The file or folder that selected text names, resolved against baseDir,
// or an empty string when it does not name one.
std::string existingPath(const std::string& text, const std::string& baseDir);

}
