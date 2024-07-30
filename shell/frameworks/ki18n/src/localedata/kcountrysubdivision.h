/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOUNTRYSUBDIVISION_H
#define KCOUNTRYSUBDIVISION_H

#include "ki18nlocaledata_export.h"

#include <QMetaType>

class KCountry;

/**
 *  @class KCountrySubdivision kcountrysubdivision.h <KCountrySubdivision>
 *
 *  Information about an ISO 3166-2 country subdivision.
 *
 *  @note This requires the [iso-codes](https://salsa.debian.org/iso-codes-team/iso-codes/)
 *  data files and translation catalogs to be available at runtime.
 *  @see KCountry for the data sources.
 *
 *  @since 5.88
 */
class KI18NLOCALEDATA_EXPORT KCountrySubdivision
{
    Q_GADGET
    Q_PROPERTY(QString code READ code)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(KCountry country READ country)
    Q_PROPERTY(KCountrySubdivision parent READ parent)
    Q_PROPERTY(QList<KCountrySubdivision> subdivisions READ subdivisions)
    Q_PROPERTY(QStringList timeZoneIds READ timeZoneIdsStringList)

public:
    /** Creates an invalid/empty KCountrySubdivision instance.
     *  See the fromX() methods for creating a valid instance.
     */
    KCountrySubdivision();
    KCountrySubdivision(const KCountrySubdivision &);
    ~KCountrySubdivision();
    KCountrySubdivision &operator=(const KCountrySubdivision &);

    bool operator==(const KCountrySubdivision &other) const;
    bool operator!=(const KCountrySubdivision &other) const;

    /** Returns @c false if this is an empty/invalid/default constructed instance, @c true otherwise. */
    bool isValid() const;

    /** ISO 3166-2 country subdivision code. */
    QString code() const;
    /** Translated country subdivision name. */
    QString name() const;
    /** Country this subdivision belongs to. */
    KCountry country() const;
    /** Parent subdivision, if this is a subdivision of another subdivision.
     * Returns an invalid element for top-level subdivisions.
     */
    KCountrySubdivision parent() const;

    /** Timezones in use in this country subdivision. */
    // for subdivisions we have to generate that by polygon intersections in QGIS -> POC
    QList<const char *> timeZoneIds() const;
    /** Subdivisions of this subdivision, if any.
     *  This is only relevant for countries with multiple ISO 3166-2 subdivision levels.
     */
    QList<KCountrySubdivision> subdivisions() const;

    /** Create a KCountrySubdivision instance from an ISO 3166-2 code. */
    static KCountrySubdivision fromCode(QStringView code);
    /** Create a KCountrySubdivision instance from an ISO 3166-2 code. */
    static KCountrySubdivision fromCode(const char *code);
    /** Looks up the country subdivision at the given geographic coordinate.
     *  This can return an invalid object if the country subdivision could not be determined. This can happen in a number of cases:
     *  - on oceans
     *  - in polar regions
     *  - close to a land border
     *  - in disputed territories
     *  @note It is possible for KCountry::fromLocation() to return a valid result despite
     *  this method returning an invalid result.
     */
    static KCountrySubdivision fromLocation(float latitude, float longitude);

private:
    KI18NLOCALEDATA_NO_EXPORT QStringList timeZoneIdsStringList() const;

    friend class KCountry;
    uint32_t d;
};

Q_DECLARE_TYPEINFO(KCountrySubdivision, Q_RELOCATABLE_TYPE);

#endif // KCOUNTRYSUBDIVISION_H
