// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

// MD5 (RFC 1321), the hash the emulator comparison prints, so its output
// lines up with the earlier builds' line for line.
class Md5
{
public:
    void addData(const void* data, size_t length)
    {
        const auto* bytes = static_cast<const uint8_t*>(data);
        m_length += length;
        while (length > 0) {
            const size_t take = std::min(length, size_t(64) - m_used);
            std::memcpy(m_block + m_used, bytes, take);
            m_used += take;
            bytes += take;
            length -= take;
            if (m_used == 64) {
                transform(m_block);
                m_used = 0;
            }
        }
    }

    void addData(std::string_view text) { addData(text.data(), text.size()); }

    std::string hexResult()
    {
        const uint64_t bits = m_length * 8;
        const uint8_t pad = 0x80;
        addData(&pad, 1);
        const uint8_t zero = 0;
        while (m_used != 56)
            addData(&zero, 1);
        uint8_t lengthBytes[8];
        for (int i = 0; i < 8; ++i)
            lengthBytes[i] = uint8_t(bits >> (8 * i));
        addData(lengthBytes, 8);

        static const char digits[] = "0123456789abcdef";
        std::string hex;
        for (uint32_t word : m_state) {
            for (int i = 0; i < 4; ++i) {
                const uint8_t byte = uint8_t(word >> (8 * i));
                hex += digits[byte >> 4];
                hex += digits[byte & 15];
            }
        }
        return hex;
    }

private:
    static uint32_t rotate(uint32_t x, int c) { return (x << c) | (x >> (32 - c)); }

    void transform(const uint8_t* block)
    {
        static const uint32_t k[64] = {
            0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
            0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
            0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
            0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
            0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
            0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
            0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
            0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
        };
        static const int r[64] = {
            7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
            5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
            4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
            6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21,
        };
        uint32_t w[16];
        for (int i = 0; i < 16; ++i)
            w[i] = uint32_t(block[i * 4]) | uint32_t(block[i * 4 + 1]) << 8 | uint32_t(block[i * 4 + 2]) << 16
                    | uint32_t(block[i * 4 + 3]) << 24;

        uint32_t a = m_state[0], b = m_state[1], c = m_state[2], d = m_state[3];
        for (int i = 0; i < 64; ++i) {
            uint32_t f;
            int g;
            if (i < 16) {
                f = (b & c) | (~b & d);
                g = i;
            } else if (i < 32) {
                f = (d & b) | (~d & c);
                g = (5 * i + 1) % 16;
            } else if (i < 48) {
                f = b ^ c ^ d;
                g = (3 * i + 5) % 16;
            } else {
                f = c ^ (b | ~d);
                g = (7 * i) % 16;
            }
            const uint32_t next = d;
            d = c;
            c = b;
            b = b + rotate(a + f + k[i] + w[g], r[i]);
            a = next;
        }
        m_state[0] += a;
        m_state[1] += b;
        m_state[2] += c;
        m_state[3] += d;
    }

    uint32_t m_state[4] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
    uint8_t m_block[64] = {};
    size_t m_used = 0;
    uint64_t m_length = 0;
};
