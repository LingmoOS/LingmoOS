/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "spatial_index_property_p.h"

// unit tests for the spatial index property table type
static_assert(sizeof(SpatialIndexProperty) == 6);

// timezone only
static_assert(SpatialIndexProperty(Tz::Pacific_Auckland).m_tz == Tz::Pacific_Auckland);
static_assert(SpatialIndexProperty(Tz::Pacific_Auckland).m_subdiv == 0);

// country only
static_assert(SpatialIndexProperty(Tz::Europe_Zurich, "CH").m_subdiv > 0);
static_assert((SpatialIndexProperty(Tz::Europe_Zurich, "CH").m_subdiv & 0xffff) == 0);

// subdivision
static_assert(SpatialIndexProperty(Tz::Europe_Paris, "FR-IDF").m_subdiv > 0);
static_assert((SpatialIndexProperty(Tz::Europe_Paris, "FR-IDF").m_subdiv & 0xffff) > 0);
