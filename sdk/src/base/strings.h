// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <charconv>
#include <concepts>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <vector>

// Helpers for UTF-8 std::string text. Case folding and whitespace are ASCII
// only; text that needs Unicode rules goes through ICU.
namespace Lingmo::base::strings {

enum class CaseSensitivity { Sensitive, Insensitive };
enum class SplitBehavior { KeepEmptyParts, SkipEmptyParts };

constexpr bool isAsciiWhitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

constexpr bool isAsciiDigit(char c)
{
    return c >= '0' && c <= '9';
}

constexpr bool isAsciiAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

constexpr char toAsciiLower(char c)
{
    return c >= 'A' && c <= 'Z' ? char(c + ('a' - 'A')) : c;
}

constexpr char toAsciiUpper(char c)
{
    return c >= 'a' && c <= 'z' ? char(c - ('a' - 'A')) : c;
}

std::string toAsciiLower(std::string_view text);
std::string toAsciiUpper(std::string_view text);
// Full Unicode upper case, locale independent, special casing included
// ("ß" becomes "SS", "ŉ" "ʼN"); invalid UTF-8 becomes U+FFFD first.
std::string toUpper(std::string_view text);

// Negative, zero or positive as `a` sorts before, with or after `b`.
int compare(std::string_view a, std::string_view b, CaseSensitivity cs = CaseSensitivity::Sensitive);
bool equalsIgnoreAsciiCase(std::string_view a, std::string_view b);
// For std::sort and ordered containers.
struct LessIgnoreAsciiCase
{
    bool operator()(std::string_view a, std::string_view b) const
    {
        return compare(a, b, CaseSensitivity::Insensitive) < 0;
    }
};

bool startsWith(std::string_view text, std::string_view prefix,
                CaseSensitivity cs = CaseSensitivity::Sensitive);
bool endsWith(std::string_view text, std::string_view suffix,
              CaseSensitivity cs = CaseSensitivity::Sensitive);
bool contains(std::string_view text, std::string_view needle,
              CaseSensitivity cs = CaseSensitivity::Sensitive);
// True when `list` has an element equal to `value`.
bool contains(std::span<const std::string> list, std::string_view value,
              CaseSensitivity cs = CaseSensitivity::Sensitive);
// The byte offset of the first match at or after `from`, or npos.
size_t indexOf(std::string_view text, std::string_view needle, size_t from = 0,
               CaseSensitivity cs = CaseSensitivity::Sensitive);

// Without leading and trailing ASCII whitespace.
std::string_view trimmed(std::string_view text);
// Trimmed, with each inner run of ASCII whitespace replaced by one space.
std::string simplified(std::string_view text);

std::vector<std::string> split(std::string_view text, char separator,
                               SplitBehavior behavior = SplitBehavior::KeepEmptyParts);
std::vector<std::string> split(std::string_view text, std::string_view separator,
                               SplitBehavior behavior = SplitBehavior::KeepEmptyParts);
// Views into `text`; they live as long as the text does.
std::vector<std::string_view> splitViews(std::string_view text, char separator,
                                         SplitBehavior behavior = SplitBehavior::KeepEmptyParts);
// Split at runs of ASCII whitespace, no empty parts.
std::vector<std::string> splitWhitespace(std::string_view text);

std::string join(std::span<const std::string> parts, std::string_view separator);
std::string join(std::span<const std::string_view> parts, std::string_view separator);

// Fields `start` to `end` (inclusive) of `text` split at `separator`,
// joined back with it. Negative indices count from the end: -1 is the last.
std::string_view section(std::string_view text, char separator, int start, int end = -1);

std::string replaceAll(std::string_view text, std::string_view from, std::string_view to);

// Parses the whole of `text` after trimming ASCII whitespace; a leading '+'
// is accepted. Base 0 picks 16 for "0x", 8 for "0" and 2 for "0b" prefixes.
// Nothing on any other character, overflow or an empty text.
template<std::integral T>
std::optional<T> toNumber(std::string_view text, int base = 10);
std::optional<double> toDouble(std::string_view text);

std::string number(long long value, int base = 10);
std::string number(unsigned long long value, int base = 10);
inline std::string number(int value, int base = 10) { return number((long long)value, base); }
inline std::string number(unsigned value, int base = 10) { return number((unsigned long long)value, base); }
inline std::string number(long value, int base = 10) { return number((long long)value, base); }
inline std::string number(unsigned long value, int base = 10) { return number((unsigned long long)value, base); }
// As printf's %.*g (format 'g'), %.*f ('f') or %.*e ('e'); precision -1
// gives the shortest text that reads back to the same value.
std::string number(double value, char format = 'g', int precision = 6);

// UTF-8 decoding replaces each maximal invalid subsequence with U+FFFD, as
// the WHATWG decoder does; encoding replaces lone surrogates and values past
// U+10FFFF with U+FFFD.
bool isValidUtf8(std::string_view text);
std::string sanitizedUtf8(std::string_view text);
std::u16string utf8ToUtf16(std::string_view text);
std::string utf16ToUtf8(std::u16string_view text);
std::u32string utf8ToUtf32(std::string_view text);
std::string utf32ToUtf8(std::u32string_view text);
std::string latin1ToUtf8(std::string_view text);
void appendUtf8(std::string& text, char32_t codePoint);
// The number of UTF-16 code units the UTF-8 text takes.
size_t utf16Length(std::string_view text);

// Lower-case hex, two digits per byte.
std::string toHex(std::string_view bytes);
// Nothing on an odd length or a non-hex character.
std::optional<std::string> fromHex(std::string_view hex);

template<std::integral T>
std::optional<T> toNumber(std::string_view text, int base)
{
    text = trimmed(text);
    bool negative = false;
    if (!text.empty() && (text.front() == '+' || text.front() == '-')) {
        negative = text.front() == '-';
        text.remove_prefix(1);
    }
    if (base == 0) {
        if (text.size() > 1 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
            base = 16;
            text.remove_prefix(2);
        } else if (text.size() > 1 && text[0] == '0' && (text[1] == 'b' || text[1] == 'B')) {
            base = 2;
            text.remove_prefix(2);
        } else if (text.size() > 1 && text[0] == '0') {
            base = 8;
            text.remove_prefix(1);
        } else {
            base = 10;
        }
    }
    if (text.empty() || text.front() == '+' || text.front() == '-')
        return std::nullopt;

    // Parsed unsigned in the widest type, so that the minimum of a signed type
    // reads back.
    unsigned long long magnitude = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), magnitude, base);
    if (error != std::errc() || end != text.data() + text.size())
        return std::nullopt;
    if constexpr (std::is_signed_v<T>) {
        using U = std::make_unsigned_t<T>;
        const unsigned long long limit = negative ? (unsigned long long)(U(std::numeric_limits<T>::max()) + 1)
                                                  : (unsigned long long)std::numeric_limits<T>::max();
        if (magnitude > limit)
            return std::nullopt;
        return negative ? T(-(long long)(magnitude - 1) - 1) : T(magnitude);
    } else {
        if (magnitude > std::numeric_limits<T>::max())
            return std::nullopt;
        if (negative && magnitude != 0)
            return std::nullopt;
        return T(magnitude);
    }
}

} // namespace Lingmo::base::strings
