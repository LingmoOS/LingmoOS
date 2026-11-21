// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/i18n.h"

#include <algorithm>
#include <libintl.h>

#include "base/locale.h"

namespace Lingmo::I18n {

namespace base = Lingmo::base;

void setDomain(const char* domain)
{
    bindDomain(domain);
    textdomain(domain);
}

void bindDomain(const char* domain)
{
    bindtextdomain(domain, LINGMO_LOCALE_DIR);
    bind_textdomain_codeset(domain, "UTF-8");
}

const char* tr(const char* text)
{
    return dtr(nullptr, text);
}

const char* trc(const char* context, const char* text)
{
    return dtrc(nullptr, context, text);
}

std::string trn(const char* singular, const char* plural, uint64_t n)
{
    return dtrn(nullptr, singular, plural, n);
}

// The catalogs map the empty text to their header.
const char* dtr(const char* domain, const char* text)
{
    return *text ? dgettext(domain, text) : text;
}

// The catalogs key a message with a context as the context, an EOT and the
// text, as msgfmt writes it.
const char* dtrc(const char* domain, const char* context, const char* text)
{
    const std::string key = std::string(context) + '\004' + text;
    const char* translation = dgettext(domain, key.c_str());
    return translation == key.c_str() ? text : translation;
}

std::string dtrn(const char* domain, const char* singular, const char* plural, uint64_t n)
{
    return format(dngettext(domain, singular, plural, n), {{"n", base::Locale::system().toString(int64_t(n))}});
}

std::string format(std::string_view pattern,
                   std::initializer_list<std::pair<std::string_view, std::string_view>> arguments)
{
    std::string result;
    result.reserve(pattern.size());
    size_t position = 0;
    while (position < pattern.size()) {
        const size_t open = pattern.find('{', position);
        if (open == std::string_view::npos)
            break;
        const size_t close = pattern.find('}', open + 1);
        if (close == std::string_view::npos)
            break;
        result.append(pattern, position, open - position);
        const std::string_view name = pattern.substr(open + 1, close - open - 1);
        const auto argument = std::find_if(arguments.begin(), arguments.end(),
                                           [name](const auto& entry) { return entry.first == name; });
        if (argument != arguments.end())
            result.append(argument->second);
        else
            result.append(pattern, open, close - open + 1);
        position = close + 1;
    }
    result.append(pattern, position);
    return result;
}

} // namespace Lingmo::I18n
