// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/url.h"

#include "base/strings.h"

namespace Lingmo::base::url {
namespace strings = Lingmo::base::strings;

static int hexValue(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

std::string percentDecoded(std::string_view text)
{
    std::string result;
    result.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '%' && i + 2 < text.size()) {
            const int high = hexValue(text[i + 1]);
            const int low = hexValue(text[i + 2]);
            if (high >= 0 && low >= 0) {
                result += char(high << 4 | low);
                i += 2;
                continue;
            }
        }
        result += text[i];
    }
    return result;
}

std::string percentEncoded(std::string_view text, std::string_view keep)
{
    static constexpr char kDigits[] = "0123456789ABCDEF";
    std::string result;
    result.reserve(text.size());
    for (const char c : text) {
        const bool unreserved = strings::isAsciiAlpha(c) || strings::isAsciiDigit(c) || c == '-' || c == '.'
                || c == '_' || c == '~' || keep.find(c) != std::string_view::npos;
        if (unreserved) {
            result += c;
        } else {
            result += '%';
            result += kDigits[static_cast<unsigned char>(c) >> 4];
            result += kDigits[static_cast<unsigned char>(c) & 0xF];
        }
    }
    return result;
}

std::optional<std::string> toLocalFile(std::string_view url)
{
    if (!strings::startsWith(url, "file:", strings::CaseSensitivity::Insensitive))
        return std::nullopt;
    url.remove_prefix(5);
    if (url.starts_with("//")) {
        url.remove_prefix(2);
        const size_t slash = url.find('/');
        const std::string_view host = url.substr(0, slash);
        if (!host.empty() && !strings::equalsIgnoreAsciiCase(host, "localhost"))
            return std::nullopt;
        url = slash == std::string_view::npos ? std::string_view("/") : url.substr(slash);
    }
    // A query or fragment is not part of the path.
    url = url.substr(0, url.find_first_of("?#"));
    if (url.empty() || url.front() != '/')
        return std::nullopt;
    return percentDecoded(url);
}

bool isLocalFile(std::string_view url)
{
    return toLocalFile(url).has_value();
}

std::string fromLocalFile(std::string_view path)
{
    return "file://" + percentEncoded(path, "/!$&'()*+,;=:@");
}

std::vector<std::string> parseUriList(std::string_view text)
{
    std::vector<std::string> urls;
    while (!text.empty()) {
        const size_t end = text.find('\n');
        std::string_view line = text.substr(0, end);
        text = end == std::string_view::npos ? std::string_view() : text.substr(end + 1);
        const size_t first = line.find_first_not_of(" \t\r\v\f");
        if (first == std::string_view::npos)
            continue;
        line = line.substr(first, line.find_last_not_of(" \t\r\v\f") - first + 1);
        if (line.front() != '#')
            urls.emplace_back(line);
    }
    return urls;
}

} // namespace Lingmo::base::url
