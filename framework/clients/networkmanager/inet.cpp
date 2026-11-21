// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/networkmanager/inet.h"

#include <arpa/inet.h>
#include <cctype>
#include <cstring>

namespace Lingmo::Networkmanager::inet {

std::optional<uint32_t> parseIpv4(const std::string& text)
{
    uint32_t result = 0;
    size_t position = 0;
    for (int part = 0; part < 4; ++part) {
        if (part > 0) {
            if (position >= text.size() || text[position] != '.')
                return std::nullopt;
            ++position;
        }
        size_t digits = 0;
        uint32_t byte = 0;
        while (position < text.size() && std::isdigit(static_cast<unsigned char>(text[position])) && digits < 3) {
            byte = byte * 10 + uint32_t(text[position] - '0');
            ++position;
            ++digits;
        }
        if (digits == 0 || byte > 255)
            return std::nullopt;
        result = (result << 8) | byte;
    }
    if (position != text.size())
        return std::nullopt;
    return result;
}

std::optional<std::array<uint8_t, 16>> parseIpv6(const std::string& text)
{
    std::array<uint8_t, 16> address{};
    if (text.find(':') == std::string::npos || inet_pton(AF_INET6, text.c_str(), address.data()) != 1)
        return std::nullopt;
    return address;
}

std::string formatIpv4(uint32_t address)
{
    return std::to_string(address >> 24) + "." + std::to_string((address >> 16) & 0xff) + "."
           + std::to_string((address >> 8) & 0xff) + "." + std::to_string(address & 0xff);
}

std::string formatIpv6(const std::array<uint8_t, 16>& address)
{
    char text[INET6_ADDRSTRLEN] = {};
    if (!inet_ntop(AF_INET6, address.data(), text, sizeof(text)))
        return {};
    return text;
}

std::string formatIpv6Bytes(const std::string& bytes)
{
    if (bytes.size() != 16)
        return {};
    std::array<uint8_t, 16> address{};
    std::memcpy(address.data(), bytes.data(), 16);
    return formatIpv6(address);
}

int family(const std::string& text)
{
    if (parseIpv4(text))
        return 4;
    if (parseIpv6(text))
        return 6;
    return 0;
}

std::string normalized(const std::string& text)
{
    if (const auto v4 = parseIpv4(text))
        return formatIpv4(*v4);
    if (const auto v6 = parseIpv6(text))
        return formatIpv6(*v6);
    return {};
}

bool isLinkLocal(const std::string& text)
{
    if (const auto v4 = parseIpv4(text))
        return (*v4 & 0xffff0000u) == 0xa9fe0000u;
    if (const auto v6 = parseIpv6(text))
        return (*v6)[0] == 0xfe && ((*v6)[1] & 0xc0) == 0x80;
    return false;
}

} // namespace Lingmo::Networkmanager::inet
