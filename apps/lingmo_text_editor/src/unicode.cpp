// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "unicode.h"

#include "base/strings.h"

#include <unicode/uchar.h>
#include <unicode/ucasemap.h>
#include <unicode/uscript.h>

#include <memory>

char32_t Unicode::at(std::string_view text, size_t index)
{
    if (index >= text.size())
        return 0;
    const auto *p = reinterpret_cast<const unsigned char *>(text.data() + index);
    const size_t available = text.size() - index;
    const unsigned char lead = p[0];
    if (lead < 0x80)
        return lead;
    if (lead < 0xE0)
        return available >= 2 ? char32_t(((lead & 0x1F) << 6) | (p[1] & 0x3F)) : 0xFFFD;
    if (lead < 0xF0)
        return available >= 3 ? char32_t(((lead & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F)) : 0xFFFD;
    return available >= 4
            ? char32_t(((lead & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F))
            : 0xFFFD;
}

char32_t Unicode::before(std::string_view text, size_t index)
{
    if (index == 0)
        return 0;
    return at(text, previous(text, index));
}

size_t Unicode::next(std::string_view text, size_t index)
{
    if (index >= text.size())
        return text.size();
    return std::min(text.size(), index + sequenceLength(static_cast<unsigned char>(text[index])));
}

size_t Unicode::previous(std::string_view text, size_t index)
{
    if (index == 0)
        return 0;
    index = std::min(index, text.size());
    --index;
    while (index > 0 && (static_cast<unsigned char>(text[index]) & 0xC0) == 0x80)
        --index;
    return index;
}

size_t Unicode::boundary(std::string_view text, size_t index)
{
    if (index >= text.size())
        return text.size();
    while (index > 0 && (static_cast<unsigned char>(text[index]) & 0xC0) == 0x80)
        --index;
    return index;
}

void Unicode::append(std::string &text, char32_t codePoint)
{
    Lingmo::base::strings::appendUtf8(text, codePoint);
}

size_t Unicode::utf16Length(std::string_view text)
{
    size_t count = 0;
    for (const char c : text) {
        const auto byte = static_cast<unsigned char>(c);
        // A lead byte starts a character; four-byte ones are two UTF-16 units.
        if ((byte & 0xC0) != 0x80)
            ++count;
        if (byte >= 0xF0)
            ++count;
    }
    return count;
}

size_t Unicode::length(std::string_view text)
{
    size_t count = 0;
    for (const char c : text) {
        if ((static_cast<unsigned char>(c) & 0xC0) != 0x80)
            ++count;
    }
    return count;
}

bool Unicode::isSpace(char32_t c)
{
    if (c == ' ' || (c >= '\t' && c <= '\r'))
        return true;
    if (c < 0x80)
        return false;
    if (c == 0x85 || c == 0xA0)
        return true;
    const int8_t type = u_charType(UChar32(c));
    return type == U_SPACE_SEPARATOR || type == U_LINE_SEPARATOR || type == U_PARAGRAPH_SEPARATOR;
}

bool Unicode::isLetter(char32_t c)
{
    if (c < 0x80)
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    return (U_GET_GC_MASK(UChar32(c)) & U_GC_L_MASK) != 0;
}

bool Unicode::isLetterOrNumber(char32_t c)
{
    if (c < 0x80)
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
    return (U_GET_GC_MASK(UChar32(c)) & (U_GC_L_MASK | U_GC_N_MASK)) != 0;
}

bool Unicode::isDigit(char32_t c)
{
    if (c < 0x80)
        return c >= '0' && c <= '9';
    return u_charType(UChar32(c)) == U_DECIMAL_DIGIT_NUMBER;
}

bool Unicode::isPrint(char32_t c)
{
    if (c >= 0x20 && c < 0x7F)
        return true;
    const uint32_t mask = U_GET_GC_MASK(UChar32(c));
    return (mask & (U_GC_CC_MASK | U_GC_CF_MASK | U_GC_CS_MASK | U_GC_CN_MASK)) == 0;
}

bool Unicode::isIdeograph(char32_t c)
{
    if (c < 0x2E80)
        return false;
    UErrorCode status = U_ZERO_ERROR;
    const UScriptCode script = uscript_getScript(UChar32(c), &status);
    return script == USCRIPT_HAN || script == USCRIPT_HIRAGANA || script == USCRIPT_KATAKANA;
}

char32_t Unicode::toTitleCase(char32_t c)
{
    return char32_t(u_totitle(UChar32(c)));
}

static std::string mapCase(std::string_view text, bool upper)
{
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<UCaseMap, decltype(&ucasemap_close)> map(ucasemap_open("", 0, &status), &ucasemap_close);
    if (U_FAILURE(status))
        return std::string(text);
    std::string result(text.size() + 16, '\0');
    for (int attempt = 0; attempt < 2; ++attempt) {
        status = U_ZERO_ERROR;
        const int32_t length = upper
                ? ucasemap_utf8ToUpper(map.get(), result.data(), int32_t(result.size()), text.data(), int32_t(text.size()), &status)
                : ucasemap_utf8ToLower(map.get(), result.data(), int32_t(result.size()), text.data(), int32_t(text.size()), &status);
        if (status == U_BUFFER_OVERFLOW_ERROR) {
            result.resize(size_t(length));
            continue;
        }
        if (U_FAILURE(status))
            return std::string(text);
        result.resize(size_t(length));
        return result;
    }
    return std::string(text);
}

std::string Unicode::toUpper(std::string_view text)
{
    return mapCase(text, true);
}

std::string Unicode::toLower(std::string_view text)
{
    return mapCase(text, false);
}

std::string Unicode::sanitized(std::string_view text)
{
    return Lingmo::base::strings::sanitizedUtf8(text);
}

bool Unicode::isValid(std::string_view text)
{
    return Lingmo::base::strings::isValidUtf8(text);
}
