/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SPATIAL_INDEX_ENTRY_P_H
#define SPATIAL_INDEX_ENTRY_P_H

#include <cstdint>

#pragma pack(push)
#pragma pack(1)

/** Entry in the spatial index.
 *  This is essentially just a pair of numbers, the first being the position
 *  on the z-order curve and defines the order, the second being the index into the
 *  property table.
 *
 *  These entries exist in very large quantities, so compact storage is important.
 */
class SpatialIndexEntry
{
public:
    inline constexpr SpatialIndexEntry(uint32_t z, uint32_t propertyIdx)
        : m_z(z)
        , m_unused(0)
        , m_propHigh(propertyIdx >> 8)
        , m_propLow(propertyIdx)
    {
    }

    inline constexpr uint32_t z() const
    {
        return m_z;
    }

    inline constexpr uint32_t propertyIndex() const
    {
        return m_propHigh << 8 | m_propLow;
    }

private:
    uint32_t m_z : 22;
    [[maybe_unused]] uint32_t m_unused : 6;
    uint32_t m_propHigh : 4;
    uint8_t m_propLow;
};

#pragma pack(pop)

inline constexpr bool operator<(SpatialIndexEntry lhs, SpatialIndexEntry rhs)
{
    return lhs.z() < rhs.z();
}

inline constexpr bool operator<(SpatialIndexEntry lhs, uint32_t rhs)
{
    return lhs.z() < rhs;
}

inline constexpr bool operator<(uint32_t lhs, SpatialIndexEntry rhs)
{
    return lhs < rhs.z();
}

#endif
