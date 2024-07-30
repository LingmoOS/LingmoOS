/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_BITVECTOR_P_H
#define PRISON_BITVECTOR_P_H

#include <QByteArray>
#include <QDebug>

namespace Prison
{
class BitVector;
}
QDebug operator<<(QDebug dbg, const Prison::BitVector &v);

namespace Prison
{
/** Vector for working with a set of bits without byte alignment. */
class BitVector
{
public:
    BitVector();
    ~BitVector();

    class iterator
    {
    public:
        inline bool operator!=(const iterator &other)
        {
            return m_index != other.m_index;
        }
        inline bool operator*() const
        {
            return m_vector->at(m_index);
        }
        inline iterator operator++()
        {
            ++m_index;
            return *this;
        }

    private:
        friend class BitVector;
        const BitVector *m_vector;
        int m_index;
    };

    /** Append the lowest @p bits of @p data with the least significant bit first. */
    void appendLSB(int data, int bits);
    /** Append the lowest @p bits of @p data with the most significant bit first. */
    void appendMSB(int data, int bits);
    void appendBit(bool bit);
    void append(const BitVector &other);
    /** Returns the bit at index @p index. */
    bool at(int index) const;
    void clear();
    void reserve(int size);
    int size() const;
    /** Returns the value starting at @p index of size @p size. */
    int valueAtMSB(int index, int size) const;
    iterator begin() const;
    iterator end() const;

    bool operator==(const BitVector &other) const;
    bool operator!=(const BitVector &other) const;

private:
    friend QDebug(::operator<<)(QDebug dbg, const Prison::BitVector &v);
    QByteArray m_data;
    int m_size = 0;
};

}

#endif // PRISON_BITVECTOR_P_H
