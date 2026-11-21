// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>

// User-visible text, translated through the gettext catalogs that
// lingmo_add_translations() installs. Each program sets its own domain
// once; a library looks its text up in its own domain with the d* forms.
// The source strings are US English.
//
// Placeholders are named, "{name}", so that a translation can move them;
// format() fills them in, and the plural forms fill in "{n}" themselves.
namespace Lingmo::I18n {

// Binds `domain` to the installed catalogs and makes it the one tr() reads.
void setDomain(const char* domain);
// Binds `domain` without making it current, for a library's own catalog.
void bindDomain(const char* domain);

const char* tr(const char* text);
// The same text can need different translations in different places; the
// context tells them apart and is never shown.
const char* trc(const char* context, const char* text);
std::string trn(const char* singular, const char* plural, uint64_t n);

const char* dtr(const char* domain, const char* text);
const char* dtrc(const char* domain, const char* context, const char* text);
std::string dtrn(const char* domain, const char* singular, const char* plural, uint64_t n);

// Marks text that is translated later, where it is shown, for tables built
// before the domain is set.
constexpr const char* trNoop(const char* text)
{
    return text;
}

// Replaces each "{name}" in `pattern` with its argument; unknown names stay.
std::string format(std::string_view pattern,
                   std::initializer_list<std::pair<std::string_view, std::string_view>> arguments);

} // namespace Lingmo::I18n
