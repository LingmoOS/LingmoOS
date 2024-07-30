/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "data/spatial_index_data.cpp"
#include "data/spatial_index_parameters_p.h"
#include "data/spatial_index_properties.cpp"
#include "spatial_index_p.h"

#include <cmath>
#include <cstdint>

constexpr const float XEnd = XStart + XRange;
constexpr const float YEnd = YStart + YRange;

// verify the null entry is first in the property table
static_assert(spatial_index_properties[0].m_tz == Tz::Undefined);
static_assert(spatial_index_properties[0].m_subdiv == 0);

// z index conversions
constexpr uint32_t latlonToZ(float lat, float lon)
{
    const uint32_t x = ((lon - XStart) / XRange) * (1 << ZDepth);
    const uint32_t y = ((lat - YStart) / YRange) * (1 << ZDepth);
    uint32_t z = 0;
    for (int i = ZDepth - 1; i >= 0; --i) {
        z <<= 1;
        z += (y & (1 << i)) ? 1 : 0;
        z <<= 1;
        z += (x & (1 << i)) ? 1 : 0;
    }
    return z;
}

// "unit tests" for the z index conversion
static_assert(latlonToZ(YStart, XStart) == 0);
static_assert(latlonToZ(YEnd - 1.0f / (1 << ZDepth), XEnd - 1.0f / (1 << ZDepth)) == (1 << (2 * ZDepth)) - 1);
static_assert(latlonToZ(YStart + YRange / 2.0f, 0.0f) == ((1 << (2 * ZDepth - 1)) | (1 << (2 * ZDepth - 2))));

SpatialIndexProperty SpatialIndex::lookup(float lat, float lon)
{
    if (std::isnan(lat) || std::isnan(lon) || lon < XStart || lon >= XEnd || lat < YStart || lat >= YEnd) {
        return spatial_index_properties[0];
    }

    const auto z = latlonToZ(lat, lon);
    const auto it = std::upper_bound(std::begin(spatial_index), std::end(spatial_index), z);
    if (it == std::begin(spatial_index)) {
        return spatial_index_properties[0];
    }

    return spatial_index_properties[(*std::prev(it)).propertyIndex()];
}
