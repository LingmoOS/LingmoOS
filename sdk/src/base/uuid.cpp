// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/uuid.h"

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <sys/random.h>

namespace Lingmo::base {

Uuid Uuid::generate()
{
    std::array<uint8_t, 16> bytes;
    size_t filled = 0;
    while (filled < bytes.size()) {
        const ssize_t n = getrandom(bytes.data() + filled, bytes.size() - filled, 0);
        if (n > 0)
            filled += size_t(n);
        else if (errno != EINTR)
            std::abort();
    }
    bytes[6] = uint8_t((bytes[6] & 0x0F) | 0x40);
    bytes[8] = uint8_t((bytes[8] & 0x3F) | 0x80);
    return Uuid(bytes);
}

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

std::optional<Uuid> Uuid::parse(std::string_view text)
{
    if (text.size() == 38 && text.front() == '{' && text.back() == '}')
        text = text.substr(1, 36);
    if (text.size() != 36)
        return std::nullopt;
    std::array<uint8_t, 16> bytes;
    size_t byte = 0;
    for (size_t i = 0; i < text.size();) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (text[i] != '-')
                return std::nullopt;
            ++i;
            continue;
        }
        const int high = hexValue(text[i]);
        const int low = hexValue(text[i + 1]);
        if (high < 0 || low < 0)
            return std::nullopt;
        bytes[byte++] = uint8_t(high << 4 | low);
        i += 2;
    }
    return Uuid(bytes);
}

bool Uuid::isNil() const
{
    return std::all_of(m_bytes.begin(), m_bytes.end(), [](uint8_t b) { return b == 0; });
}

std::string Uuid::toString() const
{
    static constexpr char kDigits[] = "0123456789abcdef";
    std::string text;
    text.reserve(36);
    for (size_t i = 0; i < m_bytes.size(); ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10)
            text += '-';
        text += kDigits[m_bytes[i] >> 4];
        text += kDigits[m_bytes[i] & 0xF];
    }
    return text;
}

} // namespace Lingmo::base
