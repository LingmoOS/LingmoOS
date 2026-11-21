// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <string_view>
#include <vector>

// The shared-mime-info database, from the mime directories of ~/.local/share
// and the data directories: type names, their aliases and their parents. Read
// once, on first use; any thread.
namespace Lingmo::base::mime {

// The type a name or alias stands for, such as "application/pdf" for
// "application/x-pdf"; empty for an unknown name.
std::string canonicalName(std::string_view name);
bool isKnown(std::string_view name);
// The other names of a type.
std::vector<std::string> aliases(std::string_view name);
// The types it is a direct subclass of:
// text types without one are text/plain's, other file types
// application/octet-stream's, unknown names included.
std::vector<std::string> parents(std::string_view name);
// Whether the type is the other or one of its ancestors, aliases resolved.
bool inherits(std::string_view type, std::string_view ancestor);

// The type of a file: a directory is inode/directory; one glob match (globs2,
// weights, longest pattern, case flags) decides by the name alone; otherwise
// the first 16 KiB are sniffed with the magic rules, which pick among several
// glob matches; an empty file is application/x-zerosize, other content
// text/plain when it looks like text, else application/octet-stream. Blocks on
// the file: call it off the UI thread for files that may be slow to read.
std::string typeForFile(const std::string& path);
// The type of a name alone, by its glob matches only; the first of several
// matches in name order, application/octet-stream for none.
std::string typeForFileName(std::string_view fileName);
// The type of content, by the magic rules alone.
std::string typeForData(std::string_view data);
// The icons file's name for the type, else the type with '/' as '-'.
std::string iconName(std::string_view type);
// The generic-icons file's name, else the group's "-x-generic", as
// "text-x-generic".
std::string genericIconName(std::string_view type);

} // namespace Lingmo::base::mime
