/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcountrysubdivision.h"
#include "isocodes_p.h"
#include "isocodescache_p.h"
#include "kcountry.h"
#include "ki18n_logging.h"
#include "klocalizedstring.h"
#include "spatial_index_p.h"
#include "timezonedata_p.h"

#include <cstring>

static_assert(sizeof(KCountrySubdivision) == 4);

KCountrySubdivision::KCountrySubdivision()
    : d(0)
{
}

KCountrySubdivision::KCountrySubdivision(const KCountrySubdivision &) = default;
KCountrySubdivision::~KCountrySubdivision() = default;
KCountrySubdivision &KCountrySubdivision::operator=(const KCountrySubdivision &) = default;

bool KCountrySubdivision::operator==(const KCountrySubdivision &other) const
{
    return d == other.d;
}

bool KCountrySubdivision::operator!=(const KCountrySubdivision &other) const
{
    return d != other.d;
}

bool KCountrySubdivision::isValid() const
{
    return d != 0;
}

QString KCountrySubdivision::code() const
{
    if (d == 0) {
        return {};
    }

    QString code;
    code.reserve(6);
    code += country().alpha2();
    code += QLatin1Char('-');

    auto key = d & 0xffff;
    while (key) {
        const auto c = IsoCodes::mapFromAlphaNumKey(key);
        if (c) {
            code.insert(3, QLatin1Char(c));
        }
        key /= IsoCodes::AlphaNumKeyFactor;
    }

    return code;
}

QString KCountrySubdivision::name() const
{
    if (d == 0) {
        return {};
    }

    auto cache = IsoCodesCache::instance();
    cache->loadIso3166_2();
    const auto it = std::lower_bound(cache->subdivisionNameMapBegin(), cache->subdivisionNameMapEnd(), d);
    if (it != cache->subdivisionNameMapEnd() && (*it).key == d) {
        return i18nd("iso_3166-2", cache->subdivisionStringTableLookup((*it).value));
    }
    return {};
}

KCountry KCountrySubdivision::country() const
{
    KCountry c;
    c.d = d >> 16;
    return c;
}

KCountrySubdivision KCountrySubdivision::parent() const
{
    KCountrySubdivision s;
    if (d == 0) {
        return s;
    }

    auto cache = IsoCodesCache::instance();
    cache->loadIso3166_2();

    const auto it = std::lower_bound(cache->subdivisionParentMapBegin(), cache->subdivisionParentMapEnd(), d);
    if (it != cache->subdivisionParentMapEnd() && (*it).key == d) {
        s.d = (d & 0xffff0000) | (uint32_t)(*it).value;
    }

    return s;
}

QList<const char *> KCountrySubdivision::timeZoneIds() const
{
    QList<const char *> tzs;
    if (d == 0) {
        return tzs;
    }

    const auto [subdivBegin, subdivEnd] = std::equal_range(TimezoneData::subdivisionTimezoneMapBegin(), TimezoneData::subdivisionTimezoneMapEnd(), d);
    if (subdivBegin != subdivEnd) {
        tzs.reserve(std::distance(subdivBegin, subdivEnd));
        for (auto it = subdivBegin; it != subdivEnd; ++it) {
            tzs.push_back(TimezoneData::ianaIdLookup((*it).value));
        }
        return tzs;
    }

    const auto countryIt = std::lower_bound(TimezoneData::countryTimezoneMapBegin(), TimezoneData::countryTimezoneMapEnd(), uint16_t(d >> 16));
    if (countryIt != TimezoneData::countryTimezoneMapEnd() && (*countryIt).key == (d >> 16)) {
        tzs.push_back(TimezoneData::ianaIdLookup((*countryIt).value));
    }

    return tzs;
}

QList<KCountrySubdivision> KCountrySubdivision::subdivisions() const
{
    if (d == 0) {
        return {};
    }

    QList<KCountrySubdivision> l;
    auto cache = IsoCodesCache::instance();
    cache->loadIso3166_2();
    // we don't have a parent->child map, instead we use the child->parent map
    // that is sorted by country (due to the country being in the two most significant bytes of its key),
    // so we don't need to do a full sequential search here
    auto it = std::lower_bound(cache->subdivisionParentMapBegin(), cache->subdivisionParentMapEnd(), d >> 16, [](auto lhs, auto rhs) {
        return (lhs.key >> 16) < rhs;
    });
    for (; it != cache->subdivisionParentMapEnd() && ((*it).key >> 16) == (d >> 16); ++it) {
        if ((*it).value == (d & 0xffff)) {
            KCountrySubdivision s;
            s.d = (*it).key;
            l.push_back(s);
        }
    }

    return l;
}

static uint32_t validatedSubdivisionKey(uint32_t key)
{
    if (!key) {
        return 0;
    }

    auto cache = IsoCodesCache::instance();
    cache->loadIso3166_2();

    const auto it = std::lower_bound(cache->subdivisionNameMapBegin(), cache->subdivisionNameMapEnd(), key);
    if (it != cache->subdivisionNameMapEnd() && (*it).key == key) {
        return key;
    }
    return 0;
}

KCountrySubdivision KCountrySubdivision::fromCode(QStringView code)
{
    KCountrySubdivision s;
    s.d = validatedSubdivisionKey(IsoCodes::subdivisionCodeToKey(code));
    return s;
}

KCountrySubdivision KCountrySubdivision::fromCode(const char *code)
{
    KCountrySubdivision s;
    if (code) {
        s.d = validatedSubdivisionKey(IsoCodes::subdivisionCodeToKey(code, std::strlen(code)));
    }
    return s;
}

KCountrySubdivision KCountrySubdivision::fromLocation(float latitude, float longitude)
{
    const auto entry = SpatialIndex::lookup(latitude, longitude);
    KCountrySubdivision s;
    if (entry.m_subdiv & 0xffff) {
        s.d = entry.m_subdiv;
    }
    return s;
}

QStringList KCountrySubdivision::timeZoneIdsStringList() const
{
    const auto tzIds = timeZoneIds();
    QStringList l;
    l.reserve(tzIds.size());
    std::transform(tzIds.begin(), tzIds.end(), std::back_inserter(l), [](const char *tzId) {
        return QString::fromUtf8(tzId);
    });
    return l;
}

#include "moc_kcountrysubdivision.cpp"
