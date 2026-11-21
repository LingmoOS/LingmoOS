// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace Lingmo::base {

class Uuid
{
public:
    // The nil UUID.
    Uuid() = default;
    explicit Uuid(const std::array<uint8_t, 16>& bytes) : m_bytes(bytes) {}

    // Version 4, from the kernel's random source.
    static Uuid generate();
    // 36 characters with hyphens, with or without braces, either case.
    static std::optional<Uuid> parse(std::string_view text);

    bool isNil() const;
    int version() const { return m_bytes[6] >> 4; }
    const std::array<uint8_t, 16>& bytes() const { return m_bytes; }
    // Lower case, hyphens, no braces: "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx".
    std::string toString() const;

    friend bool operator==(const Uuid&, const Uuid&) = default;
    friend auto operator<=>(const Uuid&, const Uuid&) = default;

private:
    std::array<uint8_t, 16> m_bytes{};
};

} // namespace Lingmo::base
