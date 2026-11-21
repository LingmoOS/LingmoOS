// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <string_view>
#include <vector>

// ISO 639 language codes as media files carry them, through ICU.
namespace languages
{

// The two-letter, bibliographic and terminology forms of a language given in
// any of them ("zh", "chi", "zho"), those that exist, without repeats; empty
// for a code that names no language.
std::vector<std::string> codes(std::string_view code);

// The language's name in itself ("日本語" for "jpn"); English is "English";
// an unknown code is returned as it is.
std::string displayName(std::string_view code);

// The display language's two-letter code, or empty under the C locale.
std::string systemLanguage();

} // namespace languages
