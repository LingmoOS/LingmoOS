/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcountry.h"
#include "isocodes_p.h"
#include "isocodescache_p.h"
#include "kcatalog_p.h"
#include "klocalizedstring.h"
#include "logging.h"
#include "spatial_index_p.h"
#include "timezonedata_p.h"

#include <cstring>

using namespace Qt::Literals;

static_assert(sizeof(KCountry) == 2);

KCountry::KCountry()
    : d(0)
{
}

KCountry::KCountry(const KCountry &) = default;
KCountry::~KCountry() = default;

KCountry &KCountry::operator=(const KCountry &) = default;

bool KCountry::operator==(const KCountry &other) const
{
    return d == other.d;
}

bool KCountry::operator!=(const KCountry &other) const
{
    return d != other.d;
}

bool KCountry::isValid() const
{
    return d != 0;
}

QString KCountry::alpha2() const
{
    if (d == 0) {
        return {};
    }

    QString code(2, QLatin1Char('\0'));
    code[0] = QLatin1Char(d >> 8);
    code[1] = QLatin1Char(d & 0xff);
    return code;
}

QString KCountry::alpha3() const
{
    const auto cache = IsoCodesCache::instance();
    const auto it = std::find_if(cache->countryAlpha3MapBegin(), cache->countryAlpha3MapEnd(), [this](auto entry) {
        return entry.value == d;
    });
    if (it != cache->countryAlpha3MapEnd()) {
        uint16_t alpha3Key = (*it).key;
        QString code(3, QLatin1Char('\0'));
        code[2] = QLatin1Char(IsoCodes::mapFromAlphaNumKey(alpha3Key));
        alpha3Key /= IsoCodes::AlphaNumKeyFactor;
        code[1] = QLatin1Char(IsoCodes::mapFromAlphaNumKey(alpha3Key));
        alpha3Key /= IsoCodes::AlphaNumKeyFactor;
        code[0] = QLatin1Char(IsoCodes::mapFromAlphaNumKey(alpha3Key));
        return code;
    }
    return {};
}

QString KCountry::name() const
{
    if (d == 0) {
        return {};
    }

    auto cache = IsoCodesCache::instance();
    cache->loadIso3166_1();
    const auto it = std::lower_bound(cache->countryNameMapBegin(), cache->countryNameMapEnd(), d);
    if (it != cache->countryNameMapEnd() && (*it).key == d) {
        return i18nd("iso_3166-1", cache->countryStringTableLookup((*it).value));
    }
    return {};
}

QString KCountry::emojiFlag() const
{
    if (d == 0) {
        return {};
    }

    QString flag;
    char flagA[] = "\xF0\x9F\x87\xA6";
    flagA[3] = 0xA6 + ((d >> 8) - 'A');
    flag += QString::fromUtf8(flagA);
    flagA[3] = 0xA6 + ((d & 0xff) - 'A');
    flag += QString::fromUtf8(flagA);
    return flag;
}

QLocale::Country KCountry::country() const
{
    if (d == 0) {
        return QLocale::AnyCountry;
    }

    return QLocale::codeToTerritory(alpha2());
}

QList<const char *> KCountry::timeZoneIds() const
{
    QList<const char *> tzs;
    if (d == 0) {
        return tzs;
    }

    const auto countryIt = std::lower_bound(TimezoneData::countryTimezoneMapBegin(), TimezoneData::countryTimezoneMapEnd(), d);
    if (countryIt != TimezoneData::countryTimezoneMapEnd() && (*countryIt).key == d) {
        tzs.push_back(TimezoneData::ianaIdLookup((*countryIt).value));
        return tzs;
    }

    const auto [subdivBegin, subdivEnd] =
        std::equal_range(TimezoneData::subdivisionTimezoneMapBegin(), TimezoneData::subdivisionTimezoneMapEnd(), d, [](auto lhs, auto rhs) {
            if constexpr (std::is_same_v<decltype(lhs), uint16_t>)
                return lhs < (rhs.key >> 16);
            else
                return (lhs.key >> 16) < rhs;
        });
    for (auto it = subdivBegin; it != subdivEnd; ++it) {
        const auto tzId = TimezoneData::ianaIdLookup((*it).value);
        if (!tzs.contains(tzId)) {
            tzs.push_back(tzId);
        }
    }

    return tzs;
}

QString KCountry::currencyCode() const
{
    if (d == 0) {
        return {};
    }

    QString currency;
    const auto ls = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, country());
    for (const auto &l : ls) {
        if (currency.isEmpty()) {
            currency = l.currencySymbol(QLocale::CurrencyIsoCode);
        } else if (currency != l.currencySymbol(QLocale::CurrencyIsoCode)) {
            qCDebug(KI18NLD) << "conflicting currency information in QLocale for" << alpha2();
            return {};
        }
    }
    return currency;
}

QList<KCountrySubdivision> KCountry::subdivisions() const
{
    if (d == 0) {
        return {};
    }

    QList<KCountrySubdivision> l;
    auto cache = IsoCodesCache::instance();
    cache->loadIso3166_2();
    // we don't have a country->subdivisions map, instead we use the full list of subdivisions
    // (which is sorted by country due to the country being in the two most significant bytes of its key),
    // and check the child->parent subdivision map for root elements
    auto it = std::lower_bound(cache->subdivisionNameMapBegin(), cache->subdivisionNameMapEnd(), d, [](auto lhs, auto rhs) {
        return (lhs.key >> 16) < rhs;
    });

    auto [parentBegin, parentEnd] = std::equal_range(cache->subdivisionParentMapBegin(), cache->subdivisionParentMapEnd(), d, [](auto lhs, auto rhs) {
        if constexpr (std::is_same_v<decltype(lhs), uint16_t>)
            return lhs < (rhs.key >> 16);
        else
            return (lhs.key >> 16) < rhs;
    });

    for (; it != cache->subdivisionNameMapEnd() && ((*it).key >> 16) == d; ++it) {
        if (!std::binary_search(parentBegin, parentEnd, (*it).key)) {
            KCountrySubdivision s;
            s.d = (*it).key;
            l.push_back(s);
        }
    }

    return l;
}

static uint16_t validatedAlpha2Key(uint16_t alpha2Key)
{
    if (!alpha2Key) {
        return 0;
    }

    auto cache = IsoCodesCache::instance();
    cache->loadIso3166_1();
    const auto it = std::lower_bound(cache->countryNameMapBegin(), cache->countryNameMapEnd(), alpha2Key);
    if (it != cache->countryNameMapEnd() && (*it).key == alpha2Key) {
        return alpha2Key;
    }
    return 0;
}

KCountry KCountry::fromAlpha2(QStringView alpha2Code)
{
    KCountry c;
    c.d = validatedAlpha2Key(IsoCodes::alpha2CodeToKey(alpha2Code));
    return c;
}

KCountry KCountry::fromAlpha2(const char *alpha2Code)
{
    KCountry c;
    if (!alpha2Code) {
        return c;
    }
    c.d = validatedAlpha2Key(IsoCodes::alpha2CodeToKey(alpha2Code, std::strlen(alpha2Code)));
    return c;
}

static uint16_t alpha3Lookup(uint16_t alpha3Key)
{
    if (!alpha3Key) {
        return 0;
    }

    auto cache = IsoCodesCache::instance();
    cache->loadIso3166_1();
    const auto it = std::lower_bound(cache->countryAlpha3MapBegin(), cache->countryAlpha3MapEnd(), alpha3Key);
    if (it != cache->countryAlpha3MapEnd() && (*it).key == alpha3Key) {
        return (*it).value;
    }
    return 0;
}

KCountry KCountry::fromAlpha3(QStringView alpha3Code)
{
    KCountry c;
    c.d = alpha3Lookup(IsoCodes::alpha3CodeToKey(alpha3Code));
    return c;
}

KCountry KCountry::fromAlpha3(const char *alpha3Code)
{
    KCountry c;
    if (!alpha3Code) {
        return c;
    }
    c.d = alpha3Lookup(IsoCodes::alpha3CodeToKey(alpha3Code, std::strlen(alpha3Code)));
    return c;
}

KCountry KCountry::fromLocation(float latitude, float longitude)
{
    const auto entry = SpatialIndex::lookup(latitude, longitude);
    KCountry c;
    c.d = entry.m_subdiv >> 16;
    return c;
}

KCountry KCountry::fromQLocale(QLocale::Country country)
{
    return fromAlpha2(QLocale::territoryToCode(country).data());
}

static QString normalizeCountryName(QStringView name)
{
    QString res;
    res.reserve(name.size());
    for (const auto c : name) {
        // the following needs to be done fairly fine-grained, as this can easily mess up scripts
        // that rely on some non-letter characters to work
        // all values used below were obtained by similar code in KContacts, which used to do
        // a full offline pre-computation of this and checked for ambiguities introduced by too
        // aggressive normalization
        switch (c.category()) {
        // strip decorative elements that don't contribute to identification (parenthesis, dashes, quotes, etc)
        case QChar::Punctuation_Connector:
        case QChar::Punctuation_Dash:
        case QChar::Punctuation_Open:
        case QChar::Punctuation_Close:
        case QChar::Punctuation_InitialQuote:
        case QChar::Punctuation_FinalQuote:
        case QChar::Punctuation_Other:
            continue;
        default:
            break;
        }

        if (c.isSpace()) {
            if (!res.isEmpty() && !res.back().isSpace()) {
                res.push_back(' '_L1);
            }
            continue;
        }

        // if the character has a canonical decomposition skip the combining diacritic markers following it
        // this works particularly well for Latin, but messes up Hangul
        if (c.script() != QChar::Script_Hangul && c.decompositionTag() == QChar::Canonical) {
            res.push_back(c.decomposition().at(0).toCaseFolded());
        } else {
            res.push_back(c.toCaseFolded());
        }
    }

    return res.trimmed();
}

// check is @p needle is a space-separated substring of haystack
static bool isSeparatedSubstring(QStringView haystack, QStringView needle)
{
    auto idx = haystack.indexOf(needle);
    if (idx < 0) {
        return false;
    }
    if (idx > 0 && !haystack[idx - 1].isSpace()) {
        return false;
    }
    idx += needle.size();
    return idx >= haystack.size() || haystack[idx].isSpace();
}

static void checkSubstringMatch(QStringView lhs, QStringView rhs, uint16_t code, uint16_t &result)
{
    if (result == std::numeric_limits<uint16_t>::max() || result == code || rhs.isEmpty()) {
        return;
    }
    const auto matches = isSeparatedSubstring(lhs, rhs) || isSeparatedSubstring(rhs, lhs);

    if (!matches) {
        return;
    }
    result = result == 0 ? code : std::numeric_limits<uint16_t>::max();
}

KCountry KCountry::fromName(QStringView name)
{
    if (name.isEmpty()) {
        return {};
    }
    const auto normalizedName = normalizeCountryName(name);

    auto cache = IsoCodesCache::instance();
    cache->loadIso3166_1();

    uint16_t substrMatch = 0;

    // check untranslated names
    for (auto it = cache->countryNameMapBegin(); it != cache->countryNameMapEnd(); ++it) {
        const auto normalizedCountry = normalizeCountryName(QString::fromUtf8(cache->countryStringTableLookup((*it).value)));
        if (normalizedName == normalizedCountry) {
            KCountry c;
            c.d = (*it).key;
            return c;
        }
        checkSubstringMatch(normalizedName, normalizedCountry, (*it).key, substrMatch);
    }

    // check translated names
    const auto langs = KCatalog::availableCatalogLanguages("iso_3166-1");
    for (const auto &lang : langs) {
        const auto catalog = KCatalog("iso_3166-1", lang);
        for (auto it = cache->countryNameMapBegin(); it != cache->countryNameMapEnd(); ++it) {
            const auto normalizedCountry = normalizeCountryName(catalog.translate(cache->countryStringTableLookup((*it).value)));
            if (normalizedName == normalizedCountry) {
                KCountry c;
                c.d = (*it).key;
                return c;
            }
            checkSubstringMatch(normalizedName, normalizedCountry, (*it).key, substrMatch);
        }
    }

    // unique prefix/suffix match
    if (substrMatch != std::numeric_limits<uint16_t>::max() && substrMatch != 0) {
        KCountry c;
        c.d = substrMatch;
        return c;
    }

    // fallback to code lookups
    if (normalizedName.size() == 3) {
        return fromAlpha3(normalizedName);
    }
    if (normalizedName.size() == 2) {
        return fromAlpha2(normalizedName);
    }

    return {};
}

QList<KCountry> KCountry::allCountries()
{
    QList<KCountry> l;
    auto cache = IsoCodesCache::instance();
    cache->loadIso3166_1();
    l.reserve(cache->countryCount());
    std::transform(cache->countryNameMapBegin(), cache->countryNameMapEnd(), std::back_inserter(l), [](auto entry) {
        KCountry c;
        c.d = entry.key;
        return c;
    });
    return l;
}

QStringList KCountry::timeZoneIdsStringList() const
{
    const auto tzIds = timeZoneIds();
    QStringList l;
    l.reserve(tzIds.size());
    std::transform(tzIds.begin(), tzIds.end(), std::back_inserter(l), [](const char *tzId) {
        return QString::fromUtf8(tzId);
    });
    return l;
}

#include "moc_kcountry.cpp"
