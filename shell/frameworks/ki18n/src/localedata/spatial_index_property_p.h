/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SPATIAL_INDEX_PROPERTY_P_H
#define SPATIAL_INDEX_PROPERTY_P_H

#include "data/timezone_names_p.h"
#include "isocodes_p.h"

#include <cstdint>

#pragma pack(push)
#pragma pack(1)

/** Entry in the spatial index property table.
 *  That is, this is a set of properties (timezone, country, country subdivision)
 *  associated with a tile in the spatial index, optimized for compact storage.
 */
class SpatialIndexProperty
{
public:
    template<std::size_t N>
    inline constexpr SpatialIndexProperty(Tz tz, const char (&code)[N])
        : m_tz(tz)
        , m_subdiv(N == 3 ? (IsoCodes::alpha2CodeToKey(code, 2) << 16) : IsoCodes::subdivisionCodeToKey(code, N - 1))
    {
    }

    inline constexpr SpatialIndexProperty(Tz tz)
        : m_tz(tz)
        , m_subdiv(0)
    {
    }

    Tz m_tz;
    uint32_t m_subdiv;
};

#pragma pack(pop)

#endif
