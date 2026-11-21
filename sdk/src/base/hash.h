// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace Lingmo::base {

// SHA-1 and SHA-256 (FIPS 180-4), fed in pieces. For keys and checksums, not
// for anything secret.
class CryptographicHash
{
public:
    enum class Algorithm { Sha1, Sha256 };

    explicit CryptographicHash(Algorithm algorithm);

    void addData(std::string_view data);
    void addData(const void* data, size_t size) { addData(std::string_view(static_cast<const char*>(data), size)); }
    // The raw digest, 20 or 32 bytes; no data may follow.
    std::string result();
    void reset();

    static std::string hash(std::string_view data, Algorithm algorithm);
    // Lower-case hex of hash().
    static std::string hashHex(std::string_view data, Algorithm algorithm);

private:
    void compress(const uint8_t* block);

    Algorithm m_algorithm;
    std::array<uint32_t, 8> m_state{};
    std::array<uint8_t, 64> m_buffer{};
    size_t m_buffered = 0;
    uint64_t m_length = 0;
};

} // namespace Lingmo::base
