// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>

#include "base/i18n.h"

namespace i18n = Lingmo::I18n;

// The toolkit's own text, in its own catalog whatever the program's domain.
namespace Lingmo::ui {

inline constexpr const char* kTranslationDomain = "lingmo-toolkit";

inline const char* tr(const char* text)
{
    return i18n::dtr(kTranslationDomain, text);
}

inline const char* trc(const char* context, const char* text)
{
    return i18n::dtrc(kTranslationDomain, context, text);
}

inline std::string trn(const char* singular, const char* plural, uint64_t n)
{
    return i18n::dtrn(kTranslationDomain, singular, plural, n);
}

} // namespace Lingmo::ui
