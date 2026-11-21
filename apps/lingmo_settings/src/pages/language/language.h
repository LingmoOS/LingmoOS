// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "base/thread_pool.h"

namespace settings {

struct LanguageEntry
{
    std::string code;
    // In the language itself.
    std::string name;
    // In the language of the user interface.
    std::string description;
    std::string searchText;
};

// The languages of the session, most preferred first, as lingmo-services
// keeps them, and the catalog of languages that can be added.
class Language
{
public:
    // Reads the languages from lingmo-services' settings at the path.
    explicit Language(std::string configPath = defaultConfigPath());

    // The first one is the system language.
    const std::vector<LanguageEntry>& preferredLanguages() const { return m_preferred; }
    // The catalog without the preferred languages, matching the search.
    std::vector<LanguageEntry> availableLanguages() const;

    bool isLoading() const { return m_loading; }
    bool isSaving() const { return m_saving; }
    const std::string& errorMessage() const { return m_errorMessage; }

    // Reads the catalog on a thread, once per display language and process.
    void loadLanguages();
    // Keeps catalog entries whose code or names contain the text, ignoring case.
    void search(const std::string& text);
    void addLanguage(const std::string& code);
    void removeLanguage(int index);
    void moveLanguage(int from, int to);

    Lingmo::base::Signal<> preferredLanguagesChanged;
    // The catalog arrived, the search changed or the preferred languages did.
    Lingmo::base::Signal<> availableLanguagesChanged;
    Lingmo::base::Signal<> loadingChanged;
    Lingmo::base::Signal<> savingChanged;
    Lingmo::base::Signal<> errorMessageChanged;

    // The system's default language, until the user picks one.
    static std::string systemLanguage();
    static std::string defaultConfigPath();
    // From lingmo-services' settings, most preferred first.
    static std::vector<std::string> preferredLanguageCodes(const std::string& configPath);
    // The locale names are shown in.
    static std::string displayLanguage();
    static std::vector<LanguageEntry> catalog(const std::string& displayCode);

private:
    void reload();
    void setPreferredCodes(std::vector<std::string> codes);
    void saveLanguages(const std::vector<std::string>& codes);

    std::string m_configPath;
    Lingmo::base::dbus::Proxy m_service;
    Lingmo::base::dbus::Match m_changedMatch;
    Lingmo::base::dbus::PendingCall m_saveCall;
    Lingmo::base::Job m_catalogJob;
    std::vector<std::string> m_preferredCodes;
    std::vector<LanguageEntry> m_preferred;
    std::vector<LanguageEntry> m_catalog;
    std::string m_search;
    bool m_loaded = false;
    bool m_loading = false;
    bool m_saving = false;
    std::string m_errorMessage;
};

} // namespace settings
