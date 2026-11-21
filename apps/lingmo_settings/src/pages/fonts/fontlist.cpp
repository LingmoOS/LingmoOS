// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/fonts/fontlist.h"

#include <algorithm>
#include <fontconfig/fontconfig.h>

namespace settings {

static std::string fcString(FcPattern* pattern, const char* object, int index)
{
    FcChar8* value = nullptr;
    if (FcPatternGetString(pattern, object, index, &value) == FcResultMatch && value)
        return reinterpret_cast<const char*>(value);
    return {};
}

// The family's English name, or else its first one in a real language.
static std::string fcLangString(FcPattern* pattern, const char* object, const char* languageObject)
{
    int languageIndex = -1;
    for (int i = 0;; ++i) {
        const std::string language = fcString(pattern, languageObject, i);
        if (language.empty())
            break;
        if (language == "en")
            return fcString(pattern, object, i);
        if (language != "xx" && languageIndex == -1)
            languageIndex = i;
    }
    return fcString(pattern, object, languageIndex > 0 ? languageIndex : 0);
}

FontList FontList::scan(const std::string& language)
{
    FontList list;
    FcPattern* pattern = FcPatternCreate();
    if (!pattern)
        return list;
    FcObjectSet* objects = FcObjectSetBuild(FC_FAMILY, FC_FAMILYLANG, FC_CHARSET, FC_STYLE, FC_LANG, FC_SPACING,
                                            nullptr);
    if (!objects) {
        FcPatternDestroy(pattern);
        return list;
    }
    FcFontSet* fonts = FcFontList(nullptr, pattern, objects);
    FcObjectSetDestroy(objects);
    FcPatternDestroy(pattern);
    if (!fonts)
        return list;

    const auto contains = [](const std::vector<std::string>& names, const std::string& name) {
        return std::find(names.begin(), names.end(), name) != names.end();
    };
    for (int i = 0; i < fonts->nfont; ++i) {
        FcPattern* font = fonts->fonts[i];
        FcCharSet* charset = nullptr;
        if (FcPatternGetCharSet(font, FC_CHARSET, 0, &charset) != FcResultMatch || FcCharSetCount(charset) == 0)
            continue;
        const std::string family = fcLangString(font, FC_FAMILY, FC_FAMILYLANG);
        if (family.empty())
            continue;
        // Not FcPatternFormat("%{lang}"): Chrome's bundled fontconfig links its
        // newer caches as ours, and printing their lang sets indexes past our
        // language table.
        FcLangSet* languages = nullptr;
        const bool supportsLanguage = FcPatternGetLangSet(font, FC_LANG, 0, &languages) == FcResultMatch
                && FcLangSetHasLang(languages, reinterpret_cast<const FcChar8*>(language.c_str()))
                        != FcLangDifferentLang;
        int spacing = FC_PROPORTIONAL;
        if (FcPatternGetInteger(font, FC_SPACING, 0, &spacing) != FcResultMatch)
            spacing = FC_PROPORTIONAL;
        if (spacing >= FC_MONO && !contains(list.fixed, family))
            list.fixed.push_back(family);
        else if (!contains(list.general, family) && supportsLanguage)
            list.general.push_back(family);
    }
    FcFontSetDestroy(fonts);
    std::sort(list.fixed.begin(), list.fixed.end());
    std::sort(list.general.begin(), list.general.end());
    return list;
}

} // namespace settings
