// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The player passes media around as URLs in their fully encoded form
// ("file:///a%20b.mkv", "https://example.com/v.mp4").
namespace urls
{

struct Parts
{
    // Lower case; empty for a relative reference.
    std::string scheme;
    // Present when the URL has "//"; may be empty ("file:///a").
    std::optional<std::string> authority;
    // Lower case, without user information or port.
    std::string host;
    // -1 when absent; nothing at all when it is not a number.
    std::optional<int> port = -1;
    // Still encoded.
    std::string path;
    std::string query;
    std::string fragment;
};

std::optional<Parts> parse(std::string_view url);

// Typed input: an existing file (relative to workingDirectory when
// one is given) becomes a file URL, as does any input without a scheme when
// assumeLocalFile; otherwise "example.com/v.mp4" becomes http://...; empty
// when the input makes no URL.
std::string fromUserInput(std::string_view input, const std::string& workingDirectory = {},
                          bool assumeLocalFile = false);

bool isLocalFile(std::string_view url);
// The path of a file URL, else empty.
std::string toLocalFile(std::string_view url);
std::string fromLocalFile(std::string_view path);

std::string scheme(std::string_view url);
std::string host(std::string_view url);
// Decoded.
std::string path(std::string_view url);
// The last segment of the path, decoded.
std::string fileName(std::string_view url);
// For people: escapes decoded and any password left out.
std::string displayString(std::string_view url);

// The URLs of a text/uri-list, comments and blank lines left out.
std::vector<std::string> fromUriList(std::string_view list);

} // namespace urls
