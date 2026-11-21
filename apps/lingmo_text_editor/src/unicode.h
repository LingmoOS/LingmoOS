// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UNICODE_H
#define UNICODE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

// UTF-8 text handled a code point at a time. Every text in the editor is
// valid UTF-8 (loading and input replace what is not), so decoding here
// does not check. The character classes go by Unicode category, on code points.
namespace Unicode
{
// The code point starting at `index`.
char32_t at(std::string_view text, size_t index);
// The code point ending right before `index`.
char32_t before(std::string_view text, size_t index);
// Where the code point after the one at `index` starts.
size_t next(std::string_view text, size_t index);
// Where the code point ending at `index` starts.
size_t previous(std::string_view text, size_t index);
// Moves `index` back to the start of the code point it is inside of.
size_t boundary(std::string_view text, size_t index);
// The UTF-8 length of a code point from its lead byte.
inline size_t sequenceLength(unsigned char lead)
{
    return lead < 0x80 ? 1 : lead < 0xE0 ? 2 : lead < 0xF0 ? 3 : 4;
}

void append(std::string &text, char32_t codePoint);

// The length in UTF-16 code units.
size_t utf16Length(std::string_view text);
// Code points.
size_t length(std::string_view text);

// \t to \r, space, U+0085, U+00A0 and the separators.
bool isSpace(char32_t c);
// The L categories.
bool isLetter(char32_t c);
// The L and N categories.
bool isLetterOrNumber(char32_t c);
// Nd.
bool isDigit(char32_t c);
// Anything but the C categories (other than private use).
bool isPrint(char32_t c);
// Han, Hiragana or Katakana.
bool isIdeograph(char32_t c);
// The simple title case of one character.
char32_t toTitleCase(char32_t c);

// Full case mappings, locale independent.
std::string toUpper(std::string_view text);
std::string toLower(std::string_view text);
// Replaces malformed UTF-8 with U+FFFD.
std::string sanitized(std::string_view text);
bool isValid(std::string_view text);
}

#endif
