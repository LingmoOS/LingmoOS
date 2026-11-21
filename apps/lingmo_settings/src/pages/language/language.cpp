// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/language/language.h"

#include <algorithm>
#include <map>
#include <memory>
#include <mutex>

#include <unicode/locid.h>
#include <unicode/unistr.h>

#include "base/i18n.h"
#include "base/locale.h"
#include "base/paths.h"
#include "base/settings.h"

using namespace Lingmo::base;
using Lingmo::i18n::tr;

namespace settings {

static constexpr const char* kService = "org.lingmo.Services";
static constexpr const char* kPath = "/org/lingmo/Services/Language";
static constexpr const char* kInterface = "org.lingmo.Services.Language";

static std::string displayName(const icu::Locale& locale, const icu::Locale& displayLocale)
{
    icu::UnicodeString name;
    locale.getDisplayName(displayLocale, name);
    std::string utf8;
    name.toUTF8String(utf8);
    return utf8;
}

static icu::UnicodeString folded(const std::string& text)
{
    icu::UnicodeString string = icu::UnicodeString::fromUTF8(text);
    string.foldCase();
    return string;
}

static std::string trimmed(const std::string& text)
{
    const size_t start = text.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos)
        return {};
    return text.substr(start, text.find_last_not_of(" \t\n\r\f\v") - start + 1);
}

std::string Language::systemLanguage()
{
    return Locale::systemLanguage();
}

std::string Language::defaultConfigPath()
{
    return paths::lingmoConfig("services");
}

std::vector<std::string> Language::preferredLanguageCodes(const std::string& configPath)
{
    Settings settings(configPath);
    const std::string primary = settings.stringValue("Language/language", {});
    if (primary.empty())
        return {systemLanguage()};
    std::vector<std::string> codes{primary};
    for (const std::string& code : settings.stringListValue("Language/preferredLanguages", {})) {
        if (!code.empty() && std::find(codes.begin(), codes.end(), code) == codes.end())
            codes.push_back(code);
    }
    return codes;
}

std::string Language::displayLanguage()
{
    return Lingmo::base::Locale::system().name();
}

std::vector<LanguageEntry> Language::catalog(const std::string& displayCode)
{
    static const char* const supportedSystemLanguages[] = {
        "en_US",       // English (US)
        "en_GB",       // English (UK)
        "ja_JP",       // Japanese
        "ko_KR",       // Korean
        "fr_FR",       // French
        "de_DE",       // German
        "es_ES",       // Spanish (Spain)
        "pt_BR",       // Portuguese (Brazil)
        "ru_RU",       // Russian
        "it_IT",       // Italian
        "id_ID",       // Indonesian
        "vi_VN",       // Vietnamese
        "th_TH",       // Thai
        "ms_MY",       // Malay
        "hi_IN",       // Hindi
        "bn_BD",       // Bangla
        "tr_TR",       // Turkish
        "uk_UA",       // Ukrainian
        "pl_PL",       // Polish
        "nl_NL",       // Dutch
        "fr_CA",       // French (Canada)
        "pt_PT",       // Portuguese (Portugal)
        "es_419",      // Spanish (Latin America)
        "es_MX",       // Spanish (Mexico)
        "cs_CZ",       // Czech
        "da_DK",       // Danish
        "fi_FI",       // Finnish
        "sv_SE",       // Swedish
        "nb_NO",       // Norwegian Bokmål
        "hu_HU",       // Hungarian
        "ro_RO",       // Romanian
        "sk_SK",       // Slovak
        "sl_SI",       // Slovenian
        "hr_HR",       // Croatian
        "bg_BG",       // Bulgarian
        "ca_ES",       // Catalan
        "el_GR",       // Greek
        "ar",          // Arabic
        "he_IL",       // Hebrew
        "zh_Hans_CN",  // Chinese, Simplified
        "zh_Hant_HK",  // Chinese, Traditional (Hong Kong)
        "zh_Hant_TW",  // Chinese, Traditional (Taiwan)
    };
    std::vector<LanguageEntry> entries;
    const icu::Locale displayLocale(displayCode.c_str());
    for (const char* code : supportedSystemLanguages) {
        const icu::Locale locale(code);
        if (locale.isBogus() || !*locale.getLanguage())
            continue;
        LanguageEntry entry;
        entry.code = code;
        entry.name = displayName(locale, locale);
        entry.description = displayName(locale, displayLocale);
        entry.searchText = entry.name + ' ' + entry.description + ' ' + entry.code;
        entries.push_back(std::move(entry));
    }
    return entries;
}

// Built once per display language and process, so reopening the page
// repeats no ICU work, even when the page that asked first closed meanwhile.
static std::mutex s_catalogMutex;
static std::map<std::string, std::shared_ptr<const std::vector<LanguageEntry>>> s_catalogs;

Language::Language(std::string configPath)
    : m_configPath(std::move(configPath))
    , m_service(dbus::Connection::session(), kService, kPath, kInterface)
{
    reload();
    m_changedMatch = m_service.onSignal("languageChanged", [this](const dbus::Message&) { reload(); });
}

std::vector<LanguageEntry> Language::availableLanguages() const
{
    const icu::UnicodeString needle = folded(m_search);
    std::vector<LanguageEntry> entries;
    for (const LanguageEntry& entry : m_catalog) {
        if (std::find(m_preferredCodes.begin(), m_preferredCodes.end(), entry.code) != m_preferredCodes.end())
            continue;
        if (!needle.isEmpty() && folded(entry.searchText).indexOf(needle) < 0)
            continue;
        entries.push_back(entry);
    }
    return entries;
}

void Language::loadLanguages()
{
    if (m_loading || m_loaded)
        return;
    const std::string displayCode = displayLanguage();
    {
        std::lock_guard lock(s_catalogMutex);
        if (const auto cached = s_catalogs.find(displayCode); cached != s_catalogs.end()) {
            m_catalog = *cached->second;
            m_loaded = true;
            availableLanguagesChanged.emit();
            return;
        }
    }
    m_loading = true;
    loadingChanged.emit();
    m_catalogJob = runInThread(
            [displayCode] {
                auto entries = std::make_shared<const std::vector<LanguageEntry>>(catalog(displayCode));
                std::lock_guard lock(s_catalogMutex);
                return s_catalogs.emplace(displayCode, std::move(entries)).first->second;
            },
            [this](std::shared_ptr<const std::vector<LanguageEntry>> entries) {
                m_catalog = *entries;
                m_loaded = true;
                m_loading = false;
                loadingChanged.emit();
                availableLanguagesChanged.emit();
            });
}

void Language::search(const std::string& text)
{
    const std::string search = trimmed(text);
    if (search == m_search)
        return;
    m_search = search;
    availableLanguagesChanged.emit();
}

void Language::reload()
{
    setPreferredCodes(preferredLanguageCodes(m_configPath));
}

void Language::setPreferredCodes(std::vector<std::string> codes)
{
    if (codes == m_preferredCodes)
        return;
    m_preferredCodes = std::move(codes);
    m_preferred.clear();
    const icu::Locale displayLocale(displayLanguage().c_str());
    for (const std::string& code : m_preferredCodes) {
        const icu::Locale locale(code.c_str());
        m_preferred.push_back({code, displayName(locale, locale), displayName(locale, displayLocale), {}});
    }
    preferredLanguagesChanged.emit();
    availableLanguagesChanged.emit();
}

void Language::addLanguage(const std::string& code)
{
    const bool known = std::any_of(m_catalog.begin(), m_catalog.end(),
                                   [&code](const LanguageEntry& entry) { return entry.code == code; });
    if (!known || std::find(m_preferredCodes.begin(), m_preferredCodes.end(), code) != m_preferredCodes.end())
        return;
    std::vector<std::string> codes = m_preferredCodes;
    codes.push_back(code);
    saveLanguages(codes);
}

void Language::removeLanguage(int index)
{
    if (m_preferredCodes.size() <= 1 || index < 0 || index >= int(m_preferredCodes.size()))
        return;
    std::vector<std::string> codes = m_preferredCodes;
    codes.erase(codes.begin() + index);
    saveLanguages(codes);
}

void Language::moveLanguage(int from, int to)
{
    const int count = int(m_preferredCodes.size());
    if (from < 0 || to < 0 || from >= count || to >= count || from == to)
        return;
    std::vector<std::string> codes = m_preferredCodes;
    std::string code = codes[from];
    codes.erase(codes.begin() + from);
    codes.insert(codes.begin() + to, std::move(code));
    saveLanguages(codes);
}

void Language::saveLanguages(const std::vector<std::string>& codes)
{
    if (m_saving)
        return;
    m_errorMessage.clear();
    errorMessageChanged.emit();
    m_saving = true;
    savingChanged.emit();
    if (!m_service.isValid()) {
        m_errorMessage = tr("Could not save preferred languages. Please try again.");
        errorMessageChanged.emit();
        m_saving = false;
        savingChanged.emit();
        return;
    }
    m_saveCall = m_service.callAsync("setPreferredLanguages", [this, codes](const dbus::Reply& reply) {
        if (reply.isError()) {
            m_errorMessage = tr("Could not save preferred languages. Please try again.");
            errorMessageChanged.emit();
        } else {
            setPreferredCodes(codes);
        }
        m_saving = false;
        savingChanged.emit();
    }, codes);
}

} // namespace settings
