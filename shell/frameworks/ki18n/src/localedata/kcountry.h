/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOUNTRY_H
#define KCOUNTRY_H

#include "ki18nlocaledata_export.h"

#include <QLocale>
#include <QMetaType>

#include "kcountrysubdivision.h"

class KCountry;

namespace KTimeZone
{
KI18NLOCALEDATA_EXPORT KCountry country(const char *);
}

/**
 *  @class KCountry kcountry.h <KCountry>
 *
 *  Information about an ISO 3166-1 country.
 *
 *  The information provided here are aggregated from the following sources:
 *  - [iso-codes](https://salsa.debian.org/iso-codes-team/iso-codes/)
 *  - [timezone-boundary-builder](https://github.com/evansiroky/timezone-boundary-builder/)
 *  - [OSM](https://openstreetmap.org)
 *  - [CLDR](http://cldr.unicode.org/)
 *
 *  @note This requires the iso-codes data files and translation catalogs to be available at runtime.
 *
 *  @since 5.88
 */
class KI18NLOCALEDATA_EXPORT KCountry
{
    Q_GADGET
    Q_PROPERTY(QString alpha2 READ alpha2)
    Q_PROPERTY(QString alpha3 READ alpha3)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString emojiFlag READ emojiFlag)
    Q_PROPERTY(QString currencyCode READ currencyCode)
    Q_PROPERTY(QList<KCountrySubdivision> subdivisions READ subdivisions)
    Q_PROPERTY(QStringList timeZoneIds READ timeZoneIdsStringList)

public:
    /** Creates an invalid/empty KCountry instance.
     *  See the fromX() methods for creating a valid instance.
     */
    KCountry();
    KCountry(const KCountry &);
    ~KCountry();
    KCountry &operator=(const KCountry &);

    bool operator==(const KCountry &other) const;
    bool operator!=(const KCountry &other) const;

    /** Returns @c false if this is an empty/invalid/default constructed instance, @c true otherwise. */
    bool isValid() const;

    /** ISO 3166-1 alpha 2 country code. */
    QString alpha2() const;
    /** ISO 3166-1 alpha 3 country code. */
    QString alpha3() const;
    /** Translated country name. */
    QString name() const;
    /** Returns the Unicode flag emoji for this country. */
    QString emojiFlag() const;
    /** Returns the QLocale::Country value matching this country, or QLocale::AnyCountry if there is none. */
    QLocale::Country country() const; // TODO better name?

    /** Timezones in use in this country. */
    QList<const char *> timeZoneIds() const;
    /** Currency used in this country as ISO 4217 code. */
    QString currencyCode() const;
    /** Highest level of ISO 3166-2 country subdivisions.
     *  If there is only one level of subdivisions this lists all of them,
     *  for countries with multiple levels, this only includes the top-level
     *  subdivisions (ie. those having no parent subdivision).
     *  @note: This can be empty.
     */
    QList<KCountrySubdivision> subdivisions() const;

    /** Create a KCountry instance from an ISO 3166-1 alpha 2 code. */
    static KCountry fromAlpha2(QStringView alpha2Code);
    /** Create a KCountry instance from an ISO 3166-1 alpha 2 code. */
    static KCountry fromAlpha2(const char *alpha2Code);
    /** Create a KCountry instance from an ISO 3166-1 alpha 3 code. */
    static KCountry fromAlpha3(QStringView alpha3Code);
    /** Create a KCountry instance from an ISO 3166-1 alpha 3 code. */
    static KCountry fromAlpha3(const char *alpha3Code);
    /** Looks up the country at the given geographic coordinate.
     *  This can return an invalid object if the country could not be determined. This can happen in a number of cases:
     *  - on oceans
     *  - in polar regions
     *  - close to a land border
     *  - in disputed territories
     */
    static KCountry fromLocation(float latitude, float longitude);
    /** Returns a KCountry instance matching the given QLocale::Country code. */
    static KCountry fromQLocale(QLocale::Country country);
    /** Attempts to identify the country from the given name.
     *  The name can be in any language.
     */
    static KCountry fromName(QStringView name);

    /** List all countries. */
    static QList<KCountry> allCountries();

private:
    KI18NLOCALEDATA_NO_EXPORT QStringList timeZoneIdsStringList() const;

    friend class KCountrySubdivision;
    friend KI18NLOCALEDATA_EXPORT KCountry KTimeZone::country(const char *);
    uint16_t d;
};

Q_DECLARE_TYPEINFO(KCountry, Q_RELOCATABLE_TYPE);

#endif // KCOUNTRY_H
