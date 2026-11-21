// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textutil.h"

#include <algorithm>
#include <vector>

#include <unicode/uchar.h>
#include <unicode/utf8.h>

namespace textutil {

static std::vector<UChar32> codePoints(std::string_view text)
{
    std::vector<UChar32> result;
    result.reserve(text.size());
    int32_t i = 0;
    const auto length = int32_t(text.size());
    const auto* bytes = reinterpret_cast<const uint8_t*>(text.data());
    while (i < length) {
        UChar32 c;
        U8_NEXT(bytes, i, length, c);
        result.push_back(c < 0 ? 0xFFFD : c);
    }
    return result;
}

static bool isSpace(UChar32 c)
{
    return (c >= 0x09 && c <= 0x0D) || c == 0x20 || c == 0x85 || c == 0xA0 || u_isUWhiteSpace(c);
}

std::string trimmed(std::string_view text)
{
    size_t start = 0;
    size_t end = text.size();
    const auto* bytes = reinterpret_cast<const uint8_t*>(text.data());
    while (start < end) {
        int32_t i = int32_t(start);
        UChar32 c;
        U8_NEXT(bytes, i, int32_t(end), c);
        if (c < 0 || !isSpace(c))
            break;
        start = size_t(i);
    }
    while (end > start) {
        int32_t i = int32_t(end);
        UChar32 c;
        U8_PREV(bytes, int32_t(start), i, c);
        if (c < 0 || !isSpace(c))
            break;
        end = size_t(i);
    }
    return std::string(text.substr(start, end - start));
}

bool containsIgnoringCase(std::string_view text, std::string_view part)
{
    std::vector<UChar32> haystack = codePoints(text);
    std::vector<UChar32> needle = codePoints(part);
    for (UChar32& c : haystack)
        c = u_foldCase(c, U_FOLD_CASE_DEFAULT);
    for (UChar32& c : needle)
        c = u_foldCase(c, U_FOLD_CASE_DEFAULT);
    return std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end()) != haystack.end();
}

} // namespace textutil
