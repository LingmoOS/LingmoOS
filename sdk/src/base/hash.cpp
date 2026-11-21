// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/hash.h"

#include <algorithm>
#include <bit>
#include <cstring>

#include "base/strings.h"

namespace Lingmo::base {
namespace strings = Lingmo::base::strings;

static constexpr std::array<uint32_t, 64> kSha256Constants = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

static uint32_t readBigEndian(const uint8_t* bytes)
{
    return uint32_t(bytes[0]) << 24 | uint32_t(bytes[1]) << 16 | uint32_t(bytes[2]) << 8 | bytes[3];
}

CryptographicHash::CryptographicHash(Algorithm algorithm)
    : m_algorithm(algorithm)
{
    reset();
}

void CryptographicHash::reset()
{
    if (m_algorithm == Algorithm::Sha1)
        m_state = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0, 0, 0, 0};
    else
        m_state = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    m_buffered = 0;
    m_length = 0;
}

void CryptographicHash::compress(const uint8_t* block)
{
    if (m_algorithm == Algorithm::Sha1) {
        uint32_t w[80];
        for (int i = 0; i < 16; ++i)
            w[i] = readBigEndian(block + 4 * i);
        for (int i = 16; i < 80; ++i)
            w[i] = std::rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        uint32_t a = m_state[0], b = m_state[1], c = m_state[2], d = m_state[3], e = m_state[4];
        for (int i = 0; i < 80; ++i) {
            uint32_t f;
            uint32_t k;
            if (i < 20) {
                f = (b & c) | (~b & d);
                k = 0x5A827999;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            const uint32_t temp = std::rotl(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = std::rotl(b, 30);
            b = a;
            a = temp;
        }
        m_state[0] += a;
        m_state[1] += b;
        m_state[2] += c;
        m_state[3] += d;
        m_state[4] += e;
        return;
    }

    uint32_t w[64];
    for (int i = 0; i < 16; ++i)
        w[i] = readBigEndian(block + 4 * i);
    for (int i = 16; i < 64; ++i) {
        const uint32_t s0 = std::rotr(w[i - 15], 7) ^ std::rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        const uint32_t s1 = std::rotr(w[i - 2], 17) ^ std::rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    std::array<uint32_t, 8> v = m_state;
    for (int i = 0; i < 64; ++i) {
        const uint32_t s1 = std::rotr(v[4], 6) ^ std::rotr(v[4], 11) ^ std::rotr(v[4], 25);
        const uint32_t ch = (v[4] & v[5]) ^ (~v[4] & v[6]);
        const uint32_t temp1 = v[7] + s1 + ch + kSha256Constants[size_t(i)] + w[i];
        const uint32_t s0 = std::rotr(v[0], 2) ^ std::rotr(v[0], 13) ^ std::rotr(v[0], 22);
        const uint32_t maj = (v[0] & v[1]) ^ (v[0] & v[2]) ^ (v[1] & v[2]);
        const uint32_t temp2 = s0 + maj;
        v[7] = v[6];
        v[6] = v[5];
        v[5] = v[4];
        v[4] = v[3] + temp1;
        v[3] = v[2];
        v[2] = v[1];
        v[1] = v[0];
        v[0] = temp1 + temp2;
    }
    for (size_t i = 0; i < 8; ++i)
        m_state[i] += v[i];
}

void CryptographicHash::addData(std::string_view data)
{
    m_length += data.size();
    const auto* bytes = reinterpret_cast<const uint8_t*>(data.data());
    size_t size = data.size();
    if (m_buffered) {
        const size_t take = std::min(size, m_buffer.size() - m_buffered);
        std::memcpy(m_buffer.data() + m_buffered, bytes, take);
        m_buffered += take;
        bytes += take;
        size -= take;
        if (m_buffered < m_buffer.size())
            return;
        compress(m_buffer.data());
        m_buffered = 0;
    }
    while (size >= 64) {
        compress(bytes);
        bytes += 64;
        size -= 64;
    }
    std::memcpy(m_buffer.data(), bytes, size);
    m_buffered = size;
}

std::string CryptographicHash::result()
{
    const uint64_t bits = m_length * 8;
    uint8_t padding[72] = {0x80};
    const size_t padLength = (m_buffered < 56 ? 56 : 120) - m_buffered;
    for (int i = 0; i < 8; ++i)
        padding[padLength + size_t(i)] = uint8_t(bits >> (56 - 8 * i));
    const uint64_t length = m_length;
    addData(padding, padLength + 8);
    m_length = length;

    const size_t words = m_algorithm == Algorithm::Sha1 ? 5 : 8;
    std::string digest;
    for (size_t i = 0; i < words; ++i) {
        for (int shift = 24; shift >= 0; shift -= 8)
            digest += char(m_state[i] >> shift);
    }
    return digest;
}

std::string CryptographicHash::hash(std::string_view data, Algorithm algorithm)
{
    CryptographicHash hash(algorithm);
    hash.addData(data);
    return hash.result();
}

std::string CryptographicHash::hashHex(std::string_view data, Algorithm algorithm)
{
    return strings::toHex(hash(data, algorithm));
}

} // namespace Lingmo::base
