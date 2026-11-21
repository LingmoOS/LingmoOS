// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>

#include "base/i18n.h"

// The framework's own text, in its own catalog whatever the program's domain.
namespace Lingmo::framework {

inline constexpr const char* kTranslationDomain = "lingmo-framework";

inline const char* translationDomain()
{
    static const bool bound = (i18n::bindDomain(kTranslationDomain), true);
    (void)bound;
    return kTranslationDomain;
}

inline const char* tr(const char* text)
{
    return i18n::dtr(translationDomain(), text);
}

inline const char* trc(const char* context, const char* text)
{
    return i18n::dtrc(translationDomain(), context, text);
}

inline std::string trn(const char* singular, const char* plural, uint64_t n)
{
    return i18n::dtrn(translationDomain(), singular, plural, n);
}

} // namespace Lingmo::framework
