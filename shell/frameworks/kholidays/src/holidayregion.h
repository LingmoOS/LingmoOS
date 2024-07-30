/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2004 Allen Winter <winter@kde.org>
    SPDX-FileCopyrightText: 2008 David Jarvie <djarvie@kde.org>
    SPDX-FileCopyrightText: 2010 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KHOLIDAYS_HOLIDAYREGION_H
#define KHOLIDAYS_HOLIDAYREGION_H

#include "kholidays_export.h"

#include <QSharedDataPointer>
#include <QString>
#include <QStringList>

#include "holiday.h"

class QDate;
class QFileInfo;

namespace KHolidays
{
class HolidayRegionPrivate;

/** Represents a holiday region. */
class KHOLIDAYS_EXPORT HolidayRegion
{
public:
    /**
     * Creates a new Holiday Region object for a given standard Region Code.
     *
     * @param regionCode The code for the Holiday Region.
     *                   If null or unknown, an empty instance will be created.
     */
    explicit HolidayRegion(const QString &regionCode = QString());

    /**
     * Creates a new Holiday Region object from a given holiday file.
     * If file doesn't exist, an empty instance will be created.
     *
     * @param regionFile The code for the Holiday Region.
     */
    explicit HolidayRegion(const QFileInfo &regionFile);

    /** Copy constructor.
     *  @since 5.77
     */
    HolidayRegion(const HolidayRegion &);
    /** Move constructor.
     *  @since 5.77
     */
    HolidayRegion(HolidayRegion &&);

    /**
     * Destroys the holidays object.
     */
    ~HolidayRegion();

    /** Assignment operator.
     *  @since 5.77
     */
    HolidayRegion &operator=(const HolidayRegion &);
    /** Move Assignment operator.
     *  @since 5.77
     */
    HolidayRegion &operator=(HolidayRegion &&);

    /**
     * @since 4.5
     *
     *  Return a list of all available Holiday Region codes.
     *
     *  One of these can then be passed to the constructor for a new HolidayRegion
     *  object, or to name() or language() to obtain the name and language of the region.
     *
     * @see name()
     * @see languageCode()
     */
    static QStringList regionCodes();

    /**
     * @since 4.6
     *
     * Return a reasonable default Holiday Region code
     *
     * If a required country/language is not provided then the current KDE
     * country/language is used.
     *
     * @param country The country or region to find a default Holiday Region for.
     *   This can be either an ISO 3166-1 or ISO 3166-2 code.
     * @param language The language to find a default Holiday Region for
     * @return the full region code of the default file
     * @warning This methods is expensive as it involves parsing all holiday definition files.
     */
    static QString defaultRegionCode(const QString &country = QString(), const QString &language = QString());

    /**
     * @since 4.5
     *
     * Returns the unique Holiday Region code.
     *
     * Clients should not infer any meaning from the format of the code.
     *
     * @return region code, or null if the instance was constructed with
     *         an unknown region
     */
    QString regionCode() const;

    /**
     * @since 4.5
     *
     * Return the ISO 3166 country code of the file
     *
     * May be either just a country code ("US" = USA) or may include a regional
     * identifier ("US-CA" = California).  Returns "XX" if not a country.
     *
     * See https://en.wikipedia.org/wiki/ISO_3166-2
     *
     * @return the full region code of the file
     */
    QString countryCode() const;

    /**
     * @since 4.5
     *
     * Return the ISO 3166 country code of a given Holiday Region
     *
     * May be either just a country code ("US" = USA) or may include a regional
     * identifier ("US-CA" = California).  Returns "XX" if not a country.
     *
     * See https://en.wikipedia.org/wiki/ISO_3166-2
     *
     * @param regionCode The code for the Holiday Region.
     * @return the full region code of the file
     * @warning This methods is expensive as it involves parsing the corresponding holiday
     *   definition file. Prefer HolidayRegion::countryCode().
     */
    static QString countryCode(const QString &regionCode);

    /**
     * @since 4.5
     *
     * Return the ISO 639-1 language code of the file
     *
     * May be either just a language code ("en" = US English) or may include a country
     * identifier ("en_GB" = British English).
     *
     * @return the language code of the file
     */
    QString languageCode() const;

    /**
     * @since 4.5
     *
     * Return the ISO 639-1 language code of a given Holiday Region
     *
     * May be either just a language code ("en" = US English) or may include a country
     * identifier ("en_GB" = British English).
     *
     * @param regionCode The code for the Holiday Region.
     * @return the language code of the file
     * @warning This methods is expensive as it involves parsing the corresponding holiday
     *   definition file. Prefer HolidayRegion::languageCode().
     */
    static QString languageCode(const QString &regionCode);

    /**
     * @since 4.5
     *
     * Return the name of the Holiday Region.
     * This may be a country, region, or type.
     *
     * @return the short name code of the file
     */
    QString name() const;

    /**
     * @since 4.5
     *
     * Return the name of a given Holiday Region
     *
     * @param regionCode The code for the Holiday Region.
     * @return the name of the Holiday Region
     */
    static QString name(const QString &regionCode);

    /**
     * @since 4.5
     *
     * Return the description of the Holiday Region if available
     *
     * @return the description of the Holiday Region
     */
    QString description() const;

    /**
     * @since 4.5
     *
     * Return the description of a given Holiday Region if available
     *
     * @return the description of the Holiday Region
     */
    static QString description(const QString &regionCode);

    /**
     * @since 5.95
     *
     * Returns the list of holidays that occur between @p startDate and @p endDate.
     *
     */
    Holiday::List rawHolidaysWithAstroSeasons(const QDate &startDate, const QDate &endDate) const;

    /**
     * @since 5.97
     *
     * Returns the list of holidays that occur between @p startDate and @p endDate.
     */
    Holiday::List rawHolidays(const QDate &startDate, const QDate &endDate) const;

    /**
     * @since 5.95
     *
     * Returns the list of holidays that occur on a @p date.
     */
    Holiday::List rawHolidaysWithAstroSeasons(const QDate &date) const;

    /**
     * @since 5.95
     *
     * Returns the list of holidays that occur in a Gregorian calendar year @p calendarYear.
     */
    Holiday::List rawHolidaysWithAstroSeasons(int calendarYear) const;

    /**
     * @since 5.95
     *
     * Returns the list of holidays that occur between @p startDate and @p endDate and with @p category.
     */
    Holiday::List rawHolidays(const QDate &startDate, const QDate &endDate, const QString &category) const;

    /**
     * Checks whether there is any holiday defined for a @p date.
     */
    bool isHoliday(const QDate &date) const;

    /**
     * Returns whether the instance contains any holiday data.
     */
    bool isValid() const;

    /**
     * @since 4.5
     *
     * Returns whether the Region Code is valid.
     */
    static bool isValid(const QString &regionCode);

private:
    QExplicitlySharedDataPointer<HolidayRegionPrivate> d;
};

}

#endif // KHOLIDAYS_HOLIDAYREGION_H
