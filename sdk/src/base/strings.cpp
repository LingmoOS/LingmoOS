// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/strings.h"

#include <algorithm>
#include <cmath>

#include <unicode/ustring.h>

namespace Lingmo::base::strings {

static constexpr char32_t kReplacement = 0xFFFD;
// Out of the Unicode range, so that a decoded U+FFFD stays valid.
static constexpr char32_t kInvalid = 0x110000;

std::string toAsciiLower(std::string_view text)
{
    std::string result(text);
    for (char& c : result)
        c = toAsciiLower(c);
    return result;
}

std::string toAsciiUpper(std::string_view text)
{
    std::string result(text);
    for (char& c : result)
        c = toAsciiUpper(c);
    return result;
}

std::string toUpper(std::string_view text)
{
    const std::u16string source = utf8ToUtf16(text);
    if (source.empty())
        return {};
    // Special casing can lengthen the text up to three times.
    std::u16string result(source.size() * 3, u'\0');
    UErrorCode error = U_ZERO_ERROR;
    // The root locale: no Turkic or Lithuanian rules, as the common string class.
    const int32_t length = u_strToUpper(reinterpret_cast<UChar*>(result.data()), int32_t(result.size()),
                                        reinterpret_cast<const UChar*>(source.data()), int32_t(source.size()), "",
                                        &error);
    if (U_FAILURE(error))
        return std::string(text);
    result.resize(size_t(length));
    return utf16ToUtf8(result);
}

int compare(std::string_view a, std::string_view b, CaseSensitivity cs)
{
    if (cs == CaseSensitivity::Sensitive) {
        const int result = a.compare(b);
        return result < 0 ? -1 : result > 0 ? 1 : 0;
    }
    const size_t length = std::min(a.size(), b.size());
    for (size_t i = 0; i < length; ++i) {
        const unsigned char ca = toAsciiLower(a[i]);
        const unsigned char cb = toAsciiLower(b[i]);
        if (ca != cb)
            return ca < cb ? -1 : 1;
    }
    return a.size() < b.size() ? -1 : a.size() > b.size() ? 1 : 0;
}

bool equalsIgnoreAsciiCase(std::string_view a, std::string_view b)
{
    return a.size() == b.size() && compare(a, b, CaseSensitivity::Insensitive) == 0;
}

bool startsWith(std::string_view text, std::string_view prefix, CaseSensitivity cs)
{
    return text.size() >= prefix.size() && compare(text.substr(0, prefix.size()), prefix, cs) == 0;
}

bool endsWith(std::string_view text, std::string_view suffix, CaseSensitivity cs)
{
    return text.size() >= suffix.size()
            && compare(text.substr(text.size() - suffix.size()), suffix, cs) == 0;
}

size_t indexOf(std::string_view text, std::string_view needle, size_t from, CaseSensitivity cs)
{
    if (cs == CaseSensitivity::Sensitive)
        return text.find(needle, from);
    if (from > text.size() || needle.size() > text.size() - from)
        return std::string_view::npos;
    for (size_t i = from; i + needle.size() <= text.size(); ++i) {
        if (equalsIgnoreAsciiCase(text.substr(i, needle.size()), needle))
            return i;
    }
    return std::string_view::npos;
}

bool contains(std::string_view text, std::string_view needle, CaseSensitivity cs)
{
    return indexOf(text, needle, 0, cs) != std::string_view::npos;
}

bool contains(std::span<const std::string> list, std::string_view value, CaseSensitivity cs)
{
    return std::any_of(list.begin(), list.end(),
                       [&](const std::string& item) { return compare(item, value, cs) == 0; });
}

std::string_view trimmed(std::string_view text)
{
    while (!text.empty() && isAsciiWhitespace(text.front()))
        text.remove_prefix(1);
    while (!text.empty() && isAsciiWhitespace(text.back()))
        text.remove_suffix(1);
    return text;
}

std::string simplified(std::string_view text)
{
    std::string result;
    result.reserve(text.size());
    bool space = false;
    for (char c : trimmed(text)) {
        if (isAsciiWhitespace(c)) {
            space = true;
            continue;
        }
        if (space)
            result += ' ';
        space = false;
        result += c;
    }
    return result;
}

std::vector<std::string_view> splitViews(std::string_view text, char separator, SplitBehavior behavior)
{
    std::vector<std::string_view> parts;
    size_t start = 0;
    while (true) {
        const size_t end = text.find(separator, start);
        const std::string_view part = text.substr(start, end == std::string_view::npos ? end : end - start);
        if (!part.empty() || behavior == SplitBehavior::KeepEmptyParts)
            parts.push_back(part);
        if (end == std::string_view::npos)
            break;
        start = end + 1;
    }
    return parts;
}

std::vector<std::string> split(std::string_view text, char separator, SplitBehavior behavior)
{
    const std::vector<std::string_view> views = splitViews(text, separator, behavior);
    return {views.begin(), views.end()};
}

std::vector<std::string> split(std::string_view text, std::string_view separator, SplitBehavior behavior)
{
    std::vector<std::string> parts;
    if (separator.empty()) {
        // Every byte apart, as splitting at an empty separator does elsewhere.
        for (char c : text)
            parts.emplace_back(1, c);
        return parts;
    }
    size_t start = 0;
    while (true) {
        const size_t end = text.find(separator, start);
        const std::string_view part = text.substr(start, end == std::string_view::npos ? end : end - start);
        if (!part.empty() || behavior == SplitBehavior::KeepEmptyParts)
            parts.emplace_back(part);
        if (end == std::string_view::npos)
            break;
        start = end + separator.size();
    }
    return parts;
}

std::vector<std::string> splitWhitespace(std::string_view text)
{
    std::vector<std::string> parts;
    size_t i = 0;
    while (i < text.size()) {
        while (i < text.size() && isAsciiWhitespace(text[i]))
            ++i;
        const size_t start = i;
        while (i < text.size() && !isAsciiWhitespace(text[i]))
            ++i;
        if (i > start)
            parts.emplace_back(text.substr(start, i - start));
    }
    return parts;
}

template<typename T>
static std::string joinParts(std::span<const T> parts, std::string_view separator)
{
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i)
            result += separator;
        result += parts[i];
    }
    return result;
}

std::string join(std::span<const std::string> parts, std::string_view separator)
{
    return joinParts(parts, separator);
}

std::string join(std::span<const std::string_view> parts, std::string_view separator)
{
    return joinParts(parts, separator);
}

std::string_view section(std::string_view text, char separator, int start, int end)
{
    const std::vector<std::string_view> fields = splitViews(text, separator);
    const int count = int(fields.size());
    if (start < 0)
        start += count;
    if (end < 0)
        end += count;
    start = std::max(start, 0);
    end = std::min(end, count - 1);
    if (start > end || start >= count)
        return {};
    const char* first = fields[size_t(start)].data();
    const char* last = fields[size_t(end)].data() + fields[size_t(end)].size();
    return std::string_view(first, size_t(last - first));
}

std::string replaceAll(std::string_view text, std::string_view from, std::string_view to)
{
    if (from.empty())
        return std::string(text);
    std::string result;
    size_t start = 0;
    while (true) {
        const size_t found = text.find(from, start);
        if (found == std::string_view::npos)
            break;
        result += text.substr(start, found - start);
        result += to;
        start = found + from.size();
    }
    result += text.substr(start);
    return result;
}

std::optional<double> toDouble(std::string_view text)
{
    text = trimmed(text);
    if (!text.empty() && text.front() == '+') {
        text.remove_prefix(1);
        if (!text.empty() && text.front() == '-')
            return std::nullopt;
    }
    double value = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc() || end != text.data() + text.size())
        return std::nullopt;
    return value;
}

std::string number(long long value, int base)
{
    char buffer[72];
    const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value, base);
    return std::string(buffer, result.ptr);
}

std::string number(unsigned long long value, int base)
{
    char buffer[72];
    const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value, base);
    return std::string(buffer, result.ptr);
}

std::string number(double value, char format, int precision)
{
    std::chars_format chars = std::chars_format::general;
    if (format == 'f')
        chars = std::chars_format::fixed;
    else if (format == 'e')
        chars = std::chars_format::scientific;
    // Fixed notation of 1e308 with 6 decimals fits in 330 characters.
    char buffer[400];
    const auto result = precision < 0 ? std::to_chars(buffer, buffer + sizeof(buffer), value, chars)
                                      : std::to_chars(buffer, buffer + sizeof(buffer), value, chars,
                                                      std::min(precision, 40));
    return std::string(buffer, result.ptr);
}

// Decodes one code point at `i` and moves past it; kInvalid for an invalid
// sequence, consuming its maximal subpart.
static char32_t decodeUtf8(std::string_view text, size_t& i)
{
    const unsigned char lead = text[i];
    if (lead < 0x80) {
        ++i;
        return lead;
    }
    int needed = 0;
    unsigned char lower = 0x80;
    unsigned char upper = 0xBF;
    char32_t codePoint = 0;
    if (lead >= 0xC2 && lead <= 0xDF) {
        needed = 1;
        codePoint = lead & 0x1F;
    } else if (lead >= 0xE0 && lead <= 0xEF) {
        needed = 2;
        codePoint = lead & 0x0F;
        if (lead == 0xE0)
            lower = 0xA0;
        else if (lead == 0xED)
            upper = 0x9F;
    } else if (lead >= 0xF0 && lead <= 0xF4) {
        needed = 3;
        codePoint = lead & 0x07;
        if (lead == 0xF0)
            lower = 0x90;
        else if (lead == 0xF4)
            upper = 0x8F;
    } else {
        ++i;
        return kInvalid;
    }
    ++i;
    for (int k = 0; k < needed; ++k) {
        if (i >= text.size())
            return kInvalid;
        const unsigned char byte = text[i];
        if (byte < lower || byte > upper)
            return kInvalid;
        lower = 0x80;
        upper = 0xBF;
        codePoint = codePoint << 6 | (byte & 0x3F);
        ++i;
    }
    return codePoint;
}

static char32_t decoded(std::string_view text, size_t& i)
{
    const char32_t codePoint = decodeUtf8(text, i);
    return codePoint == kInvalid ? kReplacement : codePoint;
}

bool isValidUtf8(std::string_view text)
{
    size_t i = 0;
    while (i < text.size()) {
        if (decodeUtf8(text, i) == kInvalid)
            return false;
    }
    return true;
}

void appendUtf8(std::string& text, char32_t codePoint)
{
    if ((codePoint >= 0xD800 && codePoint <= 0xDFFF) || codePoint > 0x10FFFF)
        codePoint = kReplacement;
    if (codePoint < 0x80) {
        text += char(codePoint);
    } else if (codePoint < 0x800) {
        text += char(0xC0 | codePoint >> 6);
        text += char(0x80 | (codePoint & 0x3F));
    } else if (codePoint < 0x10000) {
        text += char(0xE0 | codePoint >> 12);
        text += char(0x80 | (codePoint >> 6 & 0x3F));
        text += char(0x80 | (codePoint & 0x3F));
    } else {
        text += char(0xF0 | codePoint >> 18);
        text += char(0x80 | (codePoint >> 12 & 0x3F));
        text += char(0x80 | (codePoint >> 6 & 0x3F));
        text += char(0x80 | (codePoint & 0x3F));
    }
}

std::string sanitizedUtf8(std::string_view text)
{
    std::string result;
    result.reserve(text.size());
    size_t i = 0;
    while (i < text.size())
        appendUtf8(result, decoded(text, i));
    return result;
}

std::u16string utf8ToUtf16(std::string_view text)
{
    std::u16string result;
    result.reserve(text.size());
    size_t i = 0;
    while (i < text.size()) {
        const char32_t codePoint = decoded(text, i);
        if (codePoint >= 0x10000) {
            result += char16_t(0xD800 + ((codePoint - 0x10000) >> 10));
            result += char16_t(0xDC00 + ((codePoint - 0x10000) & 0x3FF));
        } else {
            result += char16_t(codePoint);
        }
    }
    return result;
}

std::string utf16ToUtf8(std::u16string_view text)
{
    std::string result;
    result.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        const char16_t unit = text[i];
        if (unit >= 0xD800 && unit <= 0xDBFF && i + 1 < text.size() && text[i + 1] >= 0xDC00
            && text[i + 1] <= 0xDFFF) {
            appendUtf8(result, 0x10000 + ((char32_t(unit) - 0xD800) << 10) + (text[i + 1] - 0xDC00));
            ++i;
        } else {
            appendUtf8(result, unit);
        }
    }
    return result;
}

std::u32string utf8ToUtf32(std::string_view text)
{
    std::u32string result;
    result.reserve(text.size());
    size_t i = 0;
    while (i < text.size())
        result += decoded(text, i);
    return result;
}

std::string utf32ToUtf8(std::u32string_view text)
{
    std::string result;
    result.reserve(text.size());
    for (char32_t codePoint : text)
        appendUtf8(result, codePoint);
    return result;
}

std::string latin1ToUtf8(std::string_view text)
{
    std::string result;
    result.reserve(text.size());
    for (unsigned char c : text)
        appendUtf8(result, c);
    return result;
}

size_t utf16Length(std::string_view text)
{
    size_t length = 0;
    size_t i = 0;
    while (i < text.size())
        length += decoded(text, i) >= 0x10000 ? 2 : 1;
    return length;
}

std::string toHex(std::string_view bytes)
{
    static constexpr char kDigits[] = "0123456789abcdef";
    std::string result;
    result.reserve(bytes.size() * 2);
    for (unsigned char byte : bytes) {
        result += kDigits[byte >> 4];
        result += kDigits[byte & 0xF];
    }
    return result;
}

static int hexValue(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

std::optional<std::string> fromHex(std::string_view hex)
{
    if (hex.size() % 2)
        return std::nullopt;
    std::string result;
    result.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        const int high = hexValue(hex[i]);
        const int low = hexValue(hex[i + 1]);
        if (high < 0 || low < 0)
            return std::nullopt;
        result += char(high << 4 | low);
    }
    return result;
}

} // namespace Lingmo::base::strings
