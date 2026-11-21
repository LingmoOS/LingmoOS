// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/common/text_match.h"

#include <algorithm>
#include <string>

#include <unicode/uchar.h>
#include <unicode/ustring.h>

#include "base/strings.h"
#include <string_view>

namespace Lingmo::Shell {

static std::u32string folded(std::string_view text)
{
    std::u32string result = base::strings::utf8ToUtf32(text);
    for (char32_t &c : result)
        c = char32_t(u_foldCase(UChar32(c), U_FOLD_CASE_DEFAULT));
    return result;
}

bool containsIgnoringCase(std::string_view text, std::string_view needle)
{
    if (needle.empty())
        return true;
    const std::u32string haystack = folded(text);
    const std::u32string wanted = folded(needle);
    return std::search(haystack.begin(), haystack.end(), wanted.begin(), wanted.end()) != haystack.end();
}

namespace {

// Where a word begins: the first character, and whatever follows a separator.
// The launcher's names are "Text Editor", "lingmo-settings", "org.gnome.Foo".
bool startsWord(char32_t previous)
{
    return previous == U' ' || previous == U'-' || previous == U'_' || previous == U'.'
            || previous == U'/' || previous == U':';
}

} // namespace

int fuzzyMatchScore(std::string_view text, std::string_view needle)
{
    // An empty search matches everything equally, and ranking is then the
    // caller's business.
    if (needle.empty())
        return 1;

    const std::u32string haystack = folded(text);
    const std::u32string wanted = folded(needle);
    if (haystack.empty() || wanted.size() > haystack.size())
        return 0;

    if (haystack == wanted)
        return 1000;

    if (std::equal(wanted.begin(), wanted.end(), haystack.begin()))
        return 900;

    // A word start, so "te" finds "Text Editor" before "Time Editor" would
    // reach it by subsequence.
    for (size_t i = 1; i < haystack.size(); ++i) {
        if (!startsWord(haystack[i - 1]))
            continue;
        if (haystack.size() - i < wanted.size())
            break;
        if (std::equal(wanted.begin(), wanted.end(), haystack.begin() + long(i)))
            return 800;
    }

    // The characters in order: a run of adjacent ones, and one that starts a
    // word, both count for more.
    int score = 100;
    size_t matched = 0;
    bool previousMatched = false;
    for (size_t i = 0; i < haystack.size() && matched < wanted.size(); ++i) {
        if (haystack[i] != wanted[matched]) {
            previousMatched = false;
            continue;
        }
        score += 10;
        if (previousMatched)
            score += 15;
        if (i == 0 || startsWord(haystack[i - 1]))
            score += 20;
        previousMatched = true;
        ++matched;
    }
    if (matched != wanted.size())
        return 0;

    // Among subsequences, the one that covers more of the name is the better.
    score += int(40 * wanted.size() / haystack.size());
    // Keep the bands apart however long the needle is.
    return std::min(score, 799);
}

std::string toLower(std::string_view text)
{
    const std::u16string source = base::strings::utf8ToUtf16(text);
    std::u16string result(source.size() + 8, u'\0');
    for (int attempt = 0; attempt < 2; ++attempt) {
        UErrorCode status = U_ZERO_ERROR;
        const int32_t length = u_strToLower(reinterpret_cast<UChar *>(result.data()), int32_t(result.size()),
                                            reinterpret_cast<const UChar *>(source.data()), int32_t(source.size()),
                                            "", &status);
        if (status == U_BUFFER_OVERFLOW_ERROR) {
            result.resize(size_t(length));
            continue;
        }
        if (U_FAILURE(status))
            return std::string(text);
        result.resize(size_t(length));
        return base::strings::utf16ToUtf8(result);
    }
    return std::string(text);
}

} // namespace Lingmo::Shell
