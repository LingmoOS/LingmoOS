// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "common/environment.h"

// The locale variables an application is started with, from the languages the
// user picked in the services' settings.
namespace Lingmo::Core::language {

// The system's default language, until the user picks one.
std::string systemLanguage();
std::vector<std::string> preferredLanguages();
// POSIX names (ll_CC[@script]) for language codes such as zh_Hans_CN.
std::vector<std::string> translationLanguages(const std::vector<std::string>& codes);
// The UTF-8 locale a language code's dates and numbers need, such as
// "ja_JP.UTF-8" for "ja".
std::string localeFor(const std::string& code);
bool isAvailable(const std::string& name);
void apply(Environment& environment, const std::vector<std::string>& codes);
void apply(Environment& environment);

} // namespace Lingmo::Core::language
