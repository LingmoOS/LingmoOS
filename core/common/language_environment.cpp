// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common/language_environment.h"

#include "base/locale.h"
#include "base/paths.h"
#include "base/settings.h"
#include "base/strings.h"

#include <unicode/locid.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <langinfo.h>
#include <locale.h>
#include <unistd.h>

namespace Lingmo::Core::language {

namespace base = Lingmo::Base;
namespace strings = Lingmo::Base::strings;

static void removeDuplicates(std::vector<std::string>& list)
{
    std::vector<std::string> unique;
    for (std::string& item : list) {
        if (std::find(unique.begin(), unique.end(), item) == unique.end())
            unique.push_back(std::move(item));
    }
    list = std::move(unique);
}

// ll_CC for the code, with a script modifier where the code's script is not
// the one the territory uses by default.
static std::string posixLanguage(const std::string& code)
{
    const icu::Locale locale(code.c_str());
    if (locale.isBogus() || !*locale.getLanguage())
        return code;

    UErrorCode status = U_ZERO_ERROR;
    icu::Locale full(locale);
    full.addLikelySubtags(status);
    if (U_FAILURE(status) || !*full.getCountry())
        return code;

    std::string name = std::string(full.getLanguage()) + '_' + full.getCountry();
    icu::Locale regional(name.c_str());
    regional.addLikelySubtags(status);
    if (U_SUCCESS(status) && std::strcmp(full.getScript(), regional.getScript()) != 0) {
        const std::string script = full.getScript();
        if (script == "Latn")
            name += "@latin";
        else if (script == "Cyrl")
            name += "@cyrillic";
        else if (script == "Deva")
            name += "@devanagari";
    }
    return name;
}

static std::string utf8Locale(std::string name)
{
    const size_t modifier = name.find('@');
    name.insert(modifier == std::string::npos ? name.size() : modifier, ".UTF-8");
    return name;
}

// A locale generated while this process runs is not in the archive it mapped,
// and the C library remembers a failed lookup; lingmo-locales generates
// directories, which name it either way.
bool isAvailable(const std::string& name)
{
    locale_t locale = newlocale(LC_ALL_MASK, name.c_str(), nullptr);
    if (locale) {
        freelocale(locale);
        return true;
    }
    const std::string directory = strings::replaceAll(name, ".UTF-8", ".utf8");
    return ::access(("/usr/lib/locale/" + directory + "/LC_CTYPE").c_str(), F_OK) == 0;
}

static bool isUtf8Locale(const std::string& name)
{
    locale_t locale = newlocale(LC_ALL_MASK, name.c_str(), nullptr);
    if (!locale)
        return false;
    const std::string encoding = strings::toAsciiUpper(nl_langinfo_l(CODESET, locale));
    freelocale(locale);
    return encoding == "UTF-8" || encoding == "UTF8";
}

static std::string installedUtf8Locale()
{
    FILE* pipe = ::popen("locale -a 2>/dev/null", "r");
    if (!pipe)
        return "C.UTF-8";
    std::string output;
    char buffer[4096];
    size_t length;
    while ((length = std::fread(buffer, 1, sizeof(buffer), pipe)) > 0)
        output.append(buffer, length);
    ::pclose(pipe);

    for (const std::string& line : strings::split(output, '\n')) {
        const std::string name(strings::trimmed(line));
        const std::string normalized = strings::toAsciiLower(strings::replaceAll(name, "-", ""));
        if (!normalized.starts_with("c.") && normalized.find("utf8") != std::string::npos && isAvailable(name))
            return name;
    }
    return "C.UTF-8";
}

static std::string fallbackLocale()
{
    if (isAvailable("en_US.UTF-8"))
        return "en_US.UTF-8";

    static const std::string installed = installedUtf8Locale();
    return installed;
}

static std::string valueOf(const Environment& environment, const std::string& name)
{
    const auto found = environment.find(name);
    return found == environment.end() ? std::string() : found->second;
}

std::string systemLanguage()
{
    return base::Locale::systemLanguage();
}

std::vector<std::string> preferredLanguages()
{
    const base::Settings settings(base::paths::lingmoConfig("services"));
    const std::string primary = settings.stringValue("Language/language");
    if (primary.empty())
        return {systemLanguage()};

    std::vector<std::string> codes{primary};
    for (const std::string& code : settings.stringListValue("Language/preferredLanguages")) {
        if (!code.empty() && code != primary)
            codes.push_back(code);
    }
    removeDuplicates(codes);
    return codes;
}

std::string localeFor(const std::string& code)
{
    return utf8Locale(posixLanguage(code));
}

std::vector<std::string> translationLanguages(const std::vector<std::string>& codes)
{
    std::vector<std::string> languages;
    for (const std::string& code : codes) {
        if (code.empty())
            continue;
        const std::string language = posixLanguage(code);
        languages.push_back(language);
        // Hong Kong reads Traditional Chinese and rarely has catalogs of its
        // own; the Taiwanese ones are the ones that exist, so a zh_HK session
        // falls through to them rather than to English.
        if (language == "zh_HK")
            languages.push_back("zh_TW");
    }
    removeDuplicates(languages);
    return languages;
}

void apply(Environment& environment, const std::vector<std::string>& codes)
{
    const std::vector<std::string> languages = translationLanguages(codes);
    if (languages.empty())
        return;

    std::string localeName = utf8Locale(languages.front());
    if (!isAvailable(localeName)) {
        // Gettext needs a valid non-C message locale even when its language packs are installed.
        localeName = fallbackLocale();
    }
    environment.erase("LC_ALL");
    for (auto& [name, value] : environment) {
        if (name.starts_with("LC_") && !value.empty() && !isAvailable(value))
            value = localeName;
    }
    // LC_CTYPE overrides LANG for toolkits and Fontconfig, including valid but non-UTF-8 values like C.
    const std::string ctype = valueOf(environment, "LC_CTYPE");
    if (!ctype.empty() && !isUtf8Locale(ctype))
        environment["LC_CTYPE"] = localeName;
    environment["LANG"] = localeName;
    environment["LC_MESSAGES"] = localeName;

    // Gettext has no English catalogs to stop at, so languages after English would replace the source strings.
    std::vector<std::string> messageLanguages;
    for (const std::string& language : languages) {
        messageLanguages.push_back(language);
        if (language == "en" || language.starts_with("en_") || language.starts_with("en@"))
            break;
    }
    environment["LANGUAGE"] = strings::join(messageLanguages, ":");
}

void apply(Environment& environment)
{
    language::apply(environment, preferredLanguages());
}

} // namespace Lingmo::Core::language
