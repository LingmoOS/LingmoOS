/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SPATIALINDEX_H
#define SPATIALINDEX_H

#include "spatial_index_property_p.h"

/** Spatial index lookup functions . */
namespace SpatialIndex
{
SpatialIndexProperty lookup(float lat, float lon);
}

#endif // SPATIALINDEX_H
