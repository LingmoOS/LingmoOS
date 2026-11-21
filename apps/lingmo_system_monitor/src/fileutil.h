// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

std::string readFile(const std::string& path);
// Whitespace-separated fields, none empty.
std::vector<std::string_view> splitFields(std::string_view line);
// Every piece between separators, empty ones included.
std::vector<std::string_view> split(std::string_view text, char separator);
std::string_view trimmed(std::string_view text);
// Unsigned decimal; false for anything else, a sign or trailing text included.
bool parseNumber(std::string_view text, uint64_t* value);
bool parseNumber(std::string_view text, int64_t* value);
// 0 when the text is no number.
uint64_t toUnsigned(std::string_view text);
int64_t toSigned(std::string_view text);

bool fileExists(const std::string& path);
// Where a symbolic link points, made absolute; empty for anything else.
std::string symLinkTarget(const std::string& path);
std::string fileName(std::string_view path);
// The file name without its last suffix.
std::string completeBaseName(std::string_view path);
// Follows every link to the file itself; empty when it does not exist.
std::string canonicalPath(const std::string& path);
// Directory entries other than . and .., in no particular order.
std::vector<std::string> directoryEntries(const std::string& path);

// The text as valid UTF-8, each invalid byte replaced, as text shown from
// the system arrives in no particular encoding.
std::string validUtf8(std::string_view text);

uint64_t counterDelta(uint64_t current, uint64_t previous);
double byteRate(uint64_t current, uint64_t previous, double elapsed);
