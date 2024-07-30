/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "spatial_index_entry_p.h"

// unit tests for the spatial index entry template
static_assert(sizeof(SpatialIndexEntry) == 5);

static_assert(SpatialIndexEntry(0, 1023).z() == 0);
static_assert(SpatialIndexEntry(0, 1023).propertyIndex() == 1023);

static_assert(SpatialIndexEntry(4194303, 1023).z() == 4194303);
static_assert(SpatialIndexEntry(4194303, 1023).propertyIndex() == 1023);
