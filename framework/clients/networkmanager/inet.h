// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>

// IP addresses as text and as NetworkManager's numbers.
namespace Lingmo::Networkmanager::inet {

// Four decimal parts only, not the shorter inet_aton() forms; host order.
std::optional<uint32_t> parseIpv4(const std::string& text);
std::optional<std::array<uint8_t, 16>> parseIpv6(const std::string& text);

std::string formatIpv4(uint32_t address);
std::string formatIpv6(const std::array<uint8_t, 16>& address);
// An "ay" of 16 bytes; empty when it is not one.
std::string formatIpv6Bytes(const std::string& bytes);

// 4 or 6, or 0 for text that is no address.
int family(const std::string& text);
// The address as it is written canonically, empty for text that is no address.
std::string normalized(const std::string& text);
bool isLinkLocal(const std::string& text);

} // namespace Lingmo::Networkmanager::inet
