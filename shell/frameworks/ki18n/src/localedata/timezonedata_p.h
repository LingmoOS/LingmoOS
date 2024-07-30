/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TIMEZONEDATA_P_H
#define TIMEZONEDATA_P_H

#include "mapentry_p.h"

#include <cstdint>

/** Utility functions to deal with the compiled-in timezone data. */
namespace TimezoneData
{
const char *ianaIdLookup(uint16_t offset);

const MapEntry<uint16_t> *countryTimezoneMapBegin();
const MapEntry<uint16_t> *countryTimezoneMapEnd();

const MapEntry<uint32_t> *subdivisionTimezoneMapBegin();
const MapEntry<uint32_t> *subdivisionTimezoneMapEnd();

}

#endif // TIMEZONEDATA_P_H
