// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

// MIME types from the shared-mime-info files of the XDG data directories;
// every function may run on any thread without a lock held.
namespace Lingmo::Files::mime {

// The type of the file at path: a directory is inode/directory, a
// device, fifo or socket its inode/ type, anything else is matched by name
// and its first 16 KiB. Reads the file.
std::string typeForFile(const std::string& path);
// The same for a file known to be regular, without the stat.
std::string typeForRegularFile(const std::string& path);

// The type a name or alias stands for; empty for an unknown one.
std::string canonicalName(const std::string& name);
bool inherits(const std::string& type, const std::string& ancestor);

// The description in the session's language (its
// LC_MESSAGES), else the untranslated one, else the type itself. Looked up
// once per type and language.
std::string description(const std::string& type);
// The icon the type names, else the type with '/' replaced by '-'.
std::string iconName(const std::string& type);

} // namespace Lingmo::Files::mime
