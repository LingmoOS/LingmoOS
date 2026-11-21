// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

namespace settings {

// The font families fontconfig knows: monospaced ones, and proportional ones
// that cover the language, each sorted by name.
struct FontList
{
    std::vector<std::string> general;
    std::vector<std::string> fixed;

    // Slow; runs off the UI thread. language is a tag such as "en-US".
    static FontList scan(const std::string& language);
};

} // namespace settings
