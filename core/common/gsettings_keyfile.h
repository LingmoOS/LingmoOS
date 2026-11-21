// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <map>
#include <string>
#include <variant>
#include <vector>

namespace Lingmo::Core {

// Line-preserving access to INI-style files that several programs share, such
// as GLib's GSettings keyfile. Lines this code does not touch stay byte-identical.
namespace keyfile {

using Group = std::map<std::string, std::string>;

// Raw text after '=' for every key, by group.
std::map<std::string, Group> read(const std::string& path);
// Replaces or appends the given keys of one group. True only when the file changed.
bool writeGroup(const std::string& path, const std::string& group, const Group& values);

} // namespace keyfile

// The session runs GLib with GSETTINGS_BACKEND=keyfile: a schema at /a/b/
// lives in group [a/b] and each value is GVariant text. Missing keys mean the
// schema default.
namespace gsettings {

using Value = std::variant<bool, int, std::string, std::vector<std::string>>;

std::string path();
std::map<std::string, keyfile::Group> read();
// The values written as GVariant text; true only when the file changed.
bool writeGroup(const std::string& group, const std::map<std::string, Value>& values);

std::string toString(const std::string& text, const std::string& fallback = {});
int toInt(const std::string& text, int fallback);
bool toBool(const std::string& text, bool fallback);
std::vector<std::string> toStringList(const std::string& text, const std::vector<std::string>& fallback);
// GVariant text for each kind of value.
std::string stringValue(const std::string& value);
std::string intValue(int value);
std::string boolValue(bool value);
std::string stringListValue(const std::vector<std::string>& values);

} // namespace gsettings

} // namespace Lingmo::Core
