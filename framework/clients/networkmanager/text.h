// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <string_view>
#include <vector>

// String helpers the client's parsers and forms share.
namespace Lingmo::Networkmanager::text {

std::string trimmed(std::string_view text);
std::string lower(std::string_view text);
bool equalsIgnoringCase(std::string_view a, std::string_view b);
bool startsWith(std::string_view text, std::string_view prefix);
bool endsWith(std::string_view text, std::string_view suffix);
// Without empty parts.
std::vector<std::string> split(std::string_view text, char separator);
std::string join(const std::vector<std::string>& parts, std::string_view separator);
// %1, %2 ... replaced in order.
std::string arg(std::string_view pattern, const std::vector<std::string>& values);
// A random version 4 UUID, as NetworkManager's profiles carry.
std::string newUuid();

} // namespace Lingmo::Networkmanager::text
