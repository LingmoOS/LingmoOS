// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "languages.h"

#include "base/locale.h"
#include "base/strings.h"

#include <unicode/uldnames.h>
#include <unicode/uloc.h>
#include <unicode/ustring.h>

#include <algorithm>
#include <array>
#include <utility>

using namespace Lingmo;

// ISO 639-2 has a bibliographic code apart from the terminology one for these.
static constexpr std::array<std::pair<std::string_view, std::string_view>, 20> kBibliographic = {{
    {"alb", "sqi"}, {"arm", "hye"}, {"baq", "eus"}, {"bur", "mya"}, {"chi", "zho"},
    {"cze", "ces"}, {"dut", "nld"}, {"fre", "fra"}, {"geo", "kat"}, {"ger", "deu"},
    {"gre", "ell"}, {"ice", "isl"}, {"mac", "mkd"}, {"mao", "mri"}, {"may", "msa"},
    {"per", "fas"}, {"rum", "ron"}, {"slo", "slk"}, {"tib", "bod"}, {"wel", "cym"},
}};

static std::string displayLanguage(const std::string& code, const char* inLocale)
{
    UChar name[128];
    UErrorCode status = U_ZERO_ERROR;
    const int32_t length = uloc_getDisplayLanguage(code.c_str(), inLocale, name, 128, &status);
    if (U_FAILURE(status) || length <= 0)
        return {};
    return base::strings::utf16ToUtf8(std::u16string_view(reinterpret_cast<const char16_t*>(name), length));
}

// The language's own name in its likely script
// when CLDR names that pair as one ("简体中文" for zh), else its own name.
static std::string nativeName(const std::string& code)
{
    char likely[64] = {};
    UErrorCode status = U_ZERO_ERROR;
    uloc_addLikelySubtags(code.c_str(), likely, sizeof(likely), &status);
    const std::string plain = displayLanguage(code, code.c_str());
    if (U_FAILURE(status))
        return plain;
    char script[16] = {};
    uloc_getScript(likely, script, sizeof(script), &status);
    const std::string withScript = code + "_" + script;
    status = U_ZERO_ERROR;
    ULocaleDisplayNames* names = uldn_open(likely, ULDN_DIALECT_NAMES, &status);
    if (U_FAILURE(status) || !script[0])
        return plain;
    UChar name[128];
    const int32_t length = uldn_localeDisplayName(names, withScript.c_str(), name, 128, &status);
    uldn_close(names);
    if (U_FAILURE(status) || length <= 0)
        return plain;
    const std::string dialect =
            base::strings::utf16ToUtf8(std::u16string_view(reinterpret_cast<const char16_t*>(name), length));
    // A name made of the language and the script in brackets is no dialect name.
    if (dialect.find('(') != std::string::npos || dialect.find("\xef\xbc\x88") != std::string::npos)
        return plain;
    return dialect;
}

// The language's terminology or two-letter code ICU knows it by, or empty.
static std::string canonicalCode(std::string_view code)
{
    if (code.size() < 2 || code.size() > 3
            || !std::all_of(code.begin(), code.end(), [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }))
        return {};
    std::string lower = base::strings::toAsciiLower(code);
    for (const auto& [bibliographic, terminology] : kBibliographic) {
        if (lower == bibliographic)
            lower = terminology;
    }
    // ICU echoes a code it has no name for.
    const std::string english = displayLanguage(lower, "en");
    if (english.empty() || english == lower)
        return {};
    return lower;
}

static std::string twoLetterCode(const std::string& code)
{
    if (code.size() == 2)
        return code;
    for (const char* const* language = uloc_getISOLanguages(); *language; ++language) {
        if (code == uloc_getISO3Language(*language))
            return *language;
    }
    return {};
}

namespace languages
{

std::vector<std::string> codes(std::string_view code)
{
    const std::string canonical = canonicalCode(code);
    if (canonical.empty())
        return {};
    const std::string part1 = twoLetterCode(canonical);
    const std::string part2T = canonical.size() == 3 ? canonical : uloc_getISO3Language(canonical.c_str());
    std::string part2B = part2T;
    for (const auto& [bibliographic, terminology] : kBibliographic) {
        if (part2T == terminology)
            part2B = bibliographic;
    }

    std::vector<std::string> result;
    for (const std::string& form : {part1, part2B, part2T}) {
        if (!form.empty() && std::find(result.begin(), result.end(), form) == result.end())
            result.push_back(form);
    }
    return result;
}

std::string displayName(std::string_view code)
{
    if (code.empty())
        return {};
    const std::string canonical = canonicalCode(code);
    if (canonical.empty())
        return std::string(code);
    const std::string english = displayLanguage(canonical, "en");
    // English's own name would name a variant; the language alone is meant.
    if (canonical == "en" || canonical == "eng")
        return english;
    const std::string native = nativeName(canonical);
    return native.empty() || native == canonical ? english : native;
}

std::string systemLanguage()
{
    const std::string language = base::Locale::system().language();
    return language == "C" ? std::string() : language;
}

} // namespace languages
