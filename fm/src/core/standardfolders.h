// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <map>
#include <memory>
#include <string>

namespace fm {

// The home folder and the XDG user folders, as the session set them up when
// the process started: their shown names and icons, keyed by path. The
// folders keep their names on disk.
std::shared_ptr<const std::map<std::string, std::string>> standardFolderNames();
// The shown names are made again on the next call, in the language of then.
void forgetStandardFolderNames();
// Empty for any other path.
std::string standardFolderName(const std::string& path);
std::string standardFolderIconName(const std::string& path);
bool isStandardFolder(const std::string& path);
// A standard folder moves only from home to home; others anywhere.
bool canMoveStandardFolderTo(const std::string& sourcePath, const std::string& destinationPath);

// Without repeated or trailing slashes and with "." and ".." resolved.
std::string cleanPath(const std::string& path);
std::string fileName(const std::string& path);
std::string parentPath(const std::string& path);
std::string homePath();

} // namespace fm
