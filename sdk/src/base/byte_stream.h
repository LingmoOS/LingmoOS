// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <bit>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

namespace Lingmo::base {

template<std::integral T>
constexpr T byteSwapped(T value)
{
    if constexpr (sizeof(T) == 1)
        return value;
    else if constexpr (sizeof(T) == 2)
        return T(__builtin_bswap16(uint16_t(value)));
    else if constexpr (sizeof(T) == 4)
        return T(__builtin_bswap32(uint32_t(value)));
    else
        return T(__builtin_bswap64(uint64_t(value)));
}

// Reads fixed-size integers in a chosen byte order from a byte string. A
// read past the end fails the stream: it returns zero and every later read
// fails too, so one check after a run of reads is enough.
class ByteReader
{
public:
    explicit ByteReader(std::string_view bytes, std::endian order = std::endian::little)
        : m_bytes(bytes), m_order(order) {}

    template<std::integral T>
    T read()
    {
        if (!m_ok || m_bytes.size() - m_position < sizeof(T)) {
            m_ok = false;
            return 0;
        }
        T value;
        std::memcpy(&value, m_bytes.data() + m_position, sizeof(T));
        m_position += sizeof(T);
        if (m_order != std::endian::native)
            value = byteSwapped(value);
        return value;
    }
    // `size` raw bytes; empty and failed when fewer are left.
    std::string_view readBytes(size_t size);
    void skip(size_t size) { readBytes(size); }
    // Moves to an absolute offset; past the end fails.
    void seek(size_t position);

    bool ok() const { return m_ok; }
    size_t position() const { return m_position; }
    size_t remaining() const { return m_bytes.size() - m_position; }
    bool atEnd() const { return m_position == m_bytes.size(); }

private:
    std::string_view m_bytes;
    std::endian m_order;
    size_t m_position = 0;
    bool m_ok = true;
};

// Appends fixed-size integers in a chosen byte order to a byte string.
class ByteWriter
{
public:
    explicit ByteWriter(std::endian order = std::endian::little) : m_order(order) {}

    template<std::integral T>
    ByteWriter& write(T value)
    {
        if (m_order != std::endian::native)
            value = byteSwapped(value);
        m_bytes.append(reinterpret_cast<const char*>(&value), sizeof(T));
        return *this;
    }
    ByteWriter& writeBytes(std::string_view bytes)
    {
        m_bytes.append(bytes);
        return *this;
    }

    const std::string& bytes() const { return m_bytes; }
    std::string take() { return std::move(m_bytes); }

private:
    std::endian m_order;
    std::string m_bytes;
};

inline std::string_view ByteReader::readBytes(size_t size)
{
    if (!m_ok || m_bytes.size() - m_position < size) {
        m_ok = false;
        return {};
    }
    const std::string_view bytes = m_bytes.substr(m_position, size);
    m_position += size;
    return bytes;
}

inline void ByteReader::seek(size_t position)
{
    if (position > m_bytes.size())
        m_ok = false;
    else
        m_position = position;
}

} // namespace Lingmo::base
