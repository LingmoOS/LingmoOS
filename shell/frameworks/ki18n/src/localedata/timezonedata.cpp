/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "timezonedata_p.h"

#include "data/country_timezone_map.cpp"
#include "data/subdivision_timezone_map.cpp"
#include "data/timezone_name_table.cpp"

#include <cassert>

static_assert(isSortedLookupTable(country_timezone_map));
static_assert(isSortedLookupTable(subdivision_timezone_map));

const char *TimezoneData::ianaIdLookup(uint16_t offset)
{
    assert(offset < sizeof(timezone_name_table));
    return timezone_name_table + offset;
}

const MapEntry<uint16_t> *TimezoneData::countryTimezoneMapBegin()
{
    return std::begin(country_timezone_map);
}

const MapEntry<uint16_t> *TimezoneData::countryTimezoneMapEnd()
{
    return std::end(country_timezone_map);
}

const MapEntry<uint32_t> *TimezoneData::subdivisionTimezoneMapBegin()
{
    return std::begin(subdivision_timezone_map);
}

const MapEntry<uint32_t> *TimezoneData::subdivisionTimezoneMapEnd()
{
    return std::end(subdivision_timezone_map);
}
