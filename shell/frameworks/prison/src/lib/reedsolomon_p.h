/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_REEDSOLOMON_P_H
#define PRISON_REEDSOLOMON_P_H

#include <memory>

namespace Prison
{
class BitVector;

/** Reed Solomon checksum generator. */
class ReedSolomon
{
public:
    enum GF {
        GF16 = 0x13,
        GF64 = 0x43,
        GF256 = 0x12d,
        GF1024 = 0x409,
        GF4096 = 0x1069,
    };

    /** Initialize a Reed Solomon encoder with the Galois Field
     *  described by the bit pattern of @p polynom, for generating
     *  @p symbolCount error correction symbols.
     */
    explicit ReedSolomon(int polynom, int symbolCount);
    ReedSolomon(const ReedSolomon &) = delete;
    ~ReedSolomon();

    /** Encode the content of @p input and return the resulting
     *  code words.
     */
    BitVector encode(const BitVector &input) const;

private:
    std::unique_ptr<int[]> m_logTable;
    std::unique_ptr<int[]> m_antiLogTable;
    std::unique_ptr<int[]> m_polynom;
    int m_symCount = 0;
    int m_symSize = 0;
};

}

#endif // PRISON_REEDSOLOMON_P_H
