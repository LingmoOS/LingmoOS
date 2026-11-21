// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"

class SkTypeface;

namespace Lingmo::ui {

// The fonts a character the family lacks is drawn in: every family sorted by
// fontconfig's FcFontSort of the family's pattern (its slant and the session's
// language), and the first family in that order with a glyph for the character
// draws it.
class FontFallback
{
public:
    FontFallback(std::string family, SkFontStyle style);
    ~FontFallback();

    // Null when no installed family has the character.
    sk_sp<SkTypeface> typefaceFor(char32_t character);

private:
    void sort();

    std::string m_family;
    SkFontStyle m_style;
    bool m_sorted = false;
    std::vector<std::string> m_families;
    std::vector<sk_sp<SkTypeface>> m_typefaces;
    std::unordered_map<char32_t, sk_sp<SkTypeface>> m_found;
};

} // namespace Lingmo::ui
