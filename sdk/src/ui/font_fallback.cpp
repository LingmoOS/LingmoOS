// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/font_fallback.h"

#include <algorithm>
#include <cctype>
#include <fontconfig/fontconfig.h>

#include "include/core/SkFontMgr.h"
#include "include/core/SkTypeface.h"
#include "modules/skparagraph/include/FontCollection.h"

#include "ui/application.h"
#include "ui/text_context.h"

namespace Lingmo::ui {

// Characters remembered before the cache starts over: text shown over a long
// session, such as a terminal's, may go through any number of them.
static constexpr size_t kMaximumFound = 65536;

static std::string folded(std::string text)
{
    for (char& c : text)
        c = char(std::tolower(static_cast<unsigned char>(c)));
    return text;
}

FontFallback::FontFallback(std::string family, SkFontStyle style)
    : m_family(std::move(family))
    , m_style(style)
{
}

FontFallback::~FontFallback() = default;

void FontFallback::sort()
{
    m_sorted = true;
    FcPattern* pattern = FcPatternCreate();
    if (!pattern)
        return;
    FcPatternAddString(pattern, FC_FAMILY, reinterpret_cast<const FcChar8*>(m_family.c_str()));
    const int slant = m_style.slant() == SkFontStyle::kItalic_Slant    ? FC_SLANT_ITALIC
                      : m_style.slant() == SkFontStyle::kOblique_Slant ? FC_SLANT_OBLIQUE
                                                                       : FC_SLANT_ROMAN;
    FcPatternAddInteger(pattern, FC_SLANT, slant);
    // The session's language, so CJK takes the locale's variant.
    FcPattern* defaults = FcPatternCreate();
    FcDefaultSubstitute(defaults);
    FcChar8* language = nullptr;
    if (FcPatternGetString(defaults, FC_LANG, 0, &language) == FcResultMatch)
        FcPatternAddString(pattern, FC_LANG, language);
    FcPatternDestroy(defaults);
    FcConfigSubstitute(nullptr, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);
    FcResult result = FcResultMatch;
    FcFontSet* fonts = FcFontSort(nullptr, pattern, FcFalse, nullptr, &result);
    FcPatternDestroy(pattern);
    if (!fonts)
        return;
    std::vector<std::string> seen = {folded(m_family)};
    for (int i = 0; i < fonts->nfont; ++i) {
        FcChar8* value = nullptr;
        if (FcPatternGetString(fonts->fonts[i], FC_FAMILY, 0, &value) != FcResultMatch || !value)
            continue;
        const std::string name = reinterpret_cast<const char*>(value);
        const std::string key = folded(name);
        if (std::find(seen.begin(), seen.end(), key) != seen.end())
            continue;
        seen.push_back(key);
        m_families.push_back(name);
    }
    FcFontSetDestroy(fonts);
    m_typefaces.resize(m_families.size());
}

sk_sp<SkTypeface> FontFallback::typefaceFor(char32_t character)
{
    if (const auto it = m_found.find(character); it != m_found.end())
        return it->second;
    if (!m_sorted)
        sort();
    if (m_found.size() >= kMaximumFound)
        m_found.clear();
    sk_sp<SkFontMgr> manager = Application::instance()->text().fontCollection()->getFallbackManager();
    sk_sp<SkTypeface> found;
    for (size_t i = 0; i < m_families.size() && manager; ++i) {
        if (!m_typefaces[i]) {
            m_typefaces[i] = manager->matchFamilyStyle(m_families[i].c_str(), m_style);
            if (!m_typefaces[i])
                continue;
        }
        if (m_typefaces[i]->unicharToGlyph(SkUnichar(character)) != 0) {
            found = m_typefaces[i];
            break;
        }
    }
    m_found.emplace(character, found);
    return found;
}

} // namespace Lingmo::ui
