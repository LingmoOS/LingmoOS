/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ktimezone.h"
#include "data/timezone_name_table.cpp"
#include "kcountry.h"
#include "spatial_index_p.h"

#include <QTimeZone>

#include <cstring>

const char *KTimeZone::fromLocation(float latitude, float longitude)
{
    const auto entry = SpatialIndex::lookup(latitude, longitude);
    return timezone_name_table + entry.m_tz;
}

KCountry KTimeZone::country(const char *ianaId)
{
    // Asia/Bangkok is special as it's the only "regular" IANA tz that covers more than one country
    // (northern Vietnam and Thailand in this case), QTimeZone however reports it as Thailand.
    if (!ianaId || std::strcmp(ianaId, "") == 0 || std::strcmp(ianaId, "Asia/Bangkok") == 0) {
        return {};
    }

    return KCountry::fromQLocale(QTimeZone(ianaId).territory());
}
