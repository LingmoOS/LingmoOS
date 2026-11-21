// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/networkmanager/text.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <sys/random.h>

namespace Lingmo::Networkmanager::text {

static bool isSpace(char c)
{
    return std::isspace(static_cast<unsigned char>(c));
}

std::string trimmed(std::string_view text)
{
    size_t begin = 0;
    size_t end = text.size();
    while (begin < end && isSpace(text[begin]))
        ++begin;
    while (end > begin && isSpace(text[end - 1]))
        --end;
    return std::string(text.substr(begin, end - begin));
}

std::string lower(std::string_view text)
{
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
    return result;
}

bool equalsIgnoringCase(std::string_view a, std::string_view b)
{
    return a.size() == b.size() && lower(a) == lower(b);
}

bool startsWith(std::string_view text, std::string_view prefix)
{
    return text.substr(0, prefix.size()) == prefix;
}

bool endsWith(std::string_view text, std::string_view suffix)
{
    return text.size() >= suffix.size() && text.substr(text.size() - suffix.size()) == suffix;
}

std::vector<std::string> split(std::string_view text, char separator)
{
    std::vector<std::string> parts;
    size_t start = 0;
    while (start <= text.size()) {
        const size_t end = std::min(text.find(separator, start), text.size());
        if (end > start)
            parts.emplace_back(text.substr(start, end - start));
        start = end + 1;
    }
    return parts;
}

std::string join(const std::vector<std::string>& parts, std::string_view separator)
{
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0)
            result += separator;
        result += parts[i];
    }
    return result;
}

std::string arg(std::string_view pattern, const std::vector<std::string>& values)
{
    std::string result;
    for (size_t i = 0; i < pattern.size(); ++i) {
        if (pattern[i] == '%' && i + 1 < pattern.size() && std::isdigit(static_cast<unsigned char>(pattern[i + 1]))) {
            const size_t index = size_t(pattern[i + 1] - '1');
            if (index < values.size()) {
                result += values[index];
                ++i;
                continue;
            }
        }
        result += pattern[i];
    }
    return result;
}

std::string newUuid()
{
    unsigned char bytes[16] = {};
    size_t filled = 0;
    while (filled < sizeof(bytes)) {
        const ssize_t read = getrandom(bytes + filled, sizeof(bytes) - filled, 0);
        if (read > 0)
            filled += size_t(read);
    }
    bytes[6] = (bytes[6] & 0x0f) | 0x40;
    bytes[8] = (bytes[8] & 0x3f) | 0x80;
    char text[37];
    std::snprintf(text, sizeof(text), "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                  bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7], bytes[8],
                  bytes[9], bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]);
    return text;
}

} // namespace Lingmo::Networkmanager::text
