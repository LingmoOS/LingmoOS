/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "bitvector_p.h"

using namespace Prison;

BitVector::BitVector() = default;
BitVector::~BitVector() = default;

void BitVector::appendLSB(int data, int bits)
{
    for (int i = 0; i < bits; ++i) {
        appendBit(data & (1 << i));
    }
}

void BitVector::appendMSB(int data, int bits)
{
    for (int i = bits - 1; i >= 0; --i) {
        appendBit(data & (1 << i));
    }
}

void BitVector::appendBit(bool bit)
{
    const auto subIdx = m_size % 8;
    if (subIdx == 0) {
        m_data.append('\0');
    }
    if (bit) {
        m_data.data()[m_data.size() - 1] |= (1 << subIdx);
    }
    ++m_size;
}

void BitVector::append(const BitVector &other)
{
    for (int i = 0; i < other.size(); ++i) {
        appendBit(other.at(i));
    }
}

bool BitVector::at(int index) const
{
    const auto majIdx = index / 8;
    const auto minIdx = index % 8;
    return (m_data.at(majIdx) & (1 << minIdx)) >> minIdx;
}

void BitVector::clear()
{
    m_data.clear();
    m_size = 0;
}

void BitVector::reserve(int size)
{
    m_data.reserve((size / 8) + 1);
}

int BitVector::size() const
{
    return m_size;
}

int BitVector::valueAtMSB(int index, int size) const
{
    int res = 0;
    for (int i = 0; i < size; ++i) {
        res = res << 1;
        res |= (at(index + i) ? 1 : 0);
    }
    return res;
}

BitVector::iterator BitVector::begin() const
{
    iterator it;
    it.m_index = 0;
    it.m_vector = this;
    return it;
}

BitVector::iterator BitVector::end() const
{
    iterator it;
    it.m_index = m_size;
    it.m_vector = this;
    return it;
}

bool BitVector::operator==(const BitVector &other) const
{
    return m_size == other.m_size && m_data == other.m_data;
}

bool BitVector::operator!=(const Prison::BitVector &other) const
{
    return m_size != other.m_size || m_data != other.m_data;
}

QDebug operator<<(QDebug dbg, const Prison::BitVector &v)
{
    dbg << v.m_data.toHex();
    return dbg;
}
