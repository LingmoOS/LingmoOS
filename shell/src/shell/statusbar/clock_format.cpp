// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/clock_format.h"

#include <algorithm>
#include <memory>

#include <unicode/dtptngen.h>
#include <unicode/locid.h>

#include "base/locale.h"
#include "base/strings.h"

namespace Lingmo::Shell::ClockFormat {

static std::string bestPattern(icu::DateTimePatternGenerator *generator, const char *skeleton)
{
    UErrorCode status = U_ZERO_ERROR;
    const icu::UnicodeString pattern = generator->getBestPattern(icu::UnicodeString(skeleton), status);
    if (U_FAILURE(status))
        return {};
    std::string result;
    pattern.toUTF8String(result);
    return result;
}

// The target format has no field for the day periods b and B or for the hours
// K and k, so they become AM/PM and the nearest hour field.
static std::string localeFormatField(char32_t letter, int count)
{
    switch (letter) {
    case 'E':
    case 'c':
        return count <= 3 ? "ddd" : "dddd";
    case 'M':
    case 'L':
        return std::string(size_t(std::min(count, 4)), 'M');
    case 'd':
        return std::string(size_t(std::min(count, 2)), 'd');
    case 'h':
    case 'K':
        return std::string(size_t(std::min(count, 2)), 'h');
    case 'H':
    case 'k':
        return std::string(size_t(std::min(count, 2)), 'H');
    case 'm':
        return std::string(size_t(std::min(count, 2)), 'm');
    case 'a':
    case 'b':
    case 'B':
        // The locale's own case: "PM" in en_US, "pm" in en_GB.
        return "Ap";
    default:
        return {};
    }
}

// Both quote literal text with apostrophes, but their field letters differ,
// so every literal is quoted again: an unquoted "de" would be a day field.
std::string toLocaleFormat(const std::string &icuPattern)
{
    const std::u32string pattern = base::strings::utf8ToUtf32(icuPattern);
    std::string result;
    std::u32string literal;

    const auto flushLiteral = [&] {
        std::erase(literal, U',');
        if (literal.empty())
            return;
        std::string text = base::strings::utf32ToUtf8(literal);
        text = base::strings::replaceAll(text, "'", "''");
        result += '\'' + text + '\'';
        literal.clear();
    };

    bool quoted = false;
    for (size_t i = 0; i < pattern.size();) {
        const char32_t c = pattern[i];
        if (c == U'\'') {
            if (i + 1 < pattern.size() && pattern[i + 1] == U'\'') {
                literal += c;
                i += 2;
            } else {
                quoted = !quoted;
                ++i;
            }
            continue;
        }

        const bool field = !quoted && ((c >= U'a' && c <= U'z') || (c >= U'A' && c <= U'Z'));
        if (!field) {
            literal += c;
            ++i;
            continue;
        }

        int count = 1;
        while (i + size_t(count) < pattern.size() && pattern[i + size_t(count)] == c)
            ++count;
        const std::string converted = localeFormatField(c, count);
        if (converted.empty())
            return {};
        flushLiteral();
        result += converted;
        i += size_t(count);
    }

    flushLiteral();
    return result;
}

bool showsSeconds(const std::string &format)
{
    bool quoted = false;
    for (const char c : format) {
        if (c == '\'')
            quoted = !quoted;
        else if (!quoted && c == 's')
            return true;
    }
    return false;
}

std::string pattern(const std::string &localeName, bool twentyFourHour)
{
    const std::string name = base::Locale(localeName).name();
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::DateTimePatternGenerator> generator(
            icu::DateTimePatternGenerator::createInstance(icu::Locale(name.c_str()), status));
    if (U_SUCCESS(status)) {
        const std::string date = toLocaleFormat(bestPattern(generator.get(), "EEEMMMd"));
        const std::string time = toLocaleFormat(bestPattern(generator.get(), twentyFourHour ? "Hmm" : "hmm"));
        if (!date.empty() && !time.empty())
            return date + ' ' + time;
    }
    return twentyFourHour ? "ddd MMM d HH:mm" : "ddd MMM d h:mm AP";
}

} // namespace Lingmo::Shell::ClockFormat
