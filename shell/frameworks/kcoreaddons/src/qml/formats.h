/*
    SPDX-FileCopyrightText: 2014 Bhushan Shah <bhush94@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef FORMATS_H
#define FORMATS_H

#include <KFormat>
#include <QObject>

class Formats : public QObject
{
    Q_OBJECT

public:
    /**
     * Converts size from bytes to the appropriate string representation
     */
    Q_INVOKABLE QString formatByteSize(double size, int precision = 1) const;

    /**
     * Given a number of milliseconds, converts that to a string containing
     * the localized equivalent, e.g. 1:23:45
     */
    Q_INVOKABLE QString formatDuration(quint64 msecs, KFormat::DurationFormatOptions options = KFormat::DefaultDuration) const;

    /**
     * Given a number of milliseconds, converts that to a string containing
     * the localized equivalent to the requested decimal places.
     *
     * e.g. given formatDuration(60000), returns "1.0 minutes"
     */
    Q_INVOKABLE QString formatDecimalDuration(quint64 msecs, int decimalPlaces = 2) const;

    /**
     * Given a number of milliseconds, converts that to a spell-out string containing
     * the localized equivalent.
     *
     * e.g. given formatSpelloutDuration(60001) returns "1 minute"
     *      given formatSpelloutDuration(62005) returns "1 minute and 2 seconds"
     *      given formatSpelloutDuration(90060000) returns "1 day and 1 hour"
     *
     * Units not interesting to the user, for example seconds or minutes when the first
     * unit is day, are not returned because they are irrelevant. The same applies for
     * seconds when the first unit is hour.
     *
     */
    Q_INVOKABLE QString formatSpelloutDuration(quint64 msecs) const;

    /**
     * Returns a string formatted to a relative date style.
     *
     * If the date falls within one week before or after the current date
     * then a relative date string will be returned, such as:
     * * Yesterday
     * * Today
     * * Tomorrow
     * * Last Tuesday
     * * Next Wednesday
     *
     * If the date falls outside this period then the format is used
     */
    Q_INVOKABLE QString formatRelativeDate(const QDate &date, QLocale::FormatType format) const;

    /**
     * Returns a string formatted to a relative datetime style.
     *
     * If the dateTime falls within one week before or after the current date
     * then a relative date string will be returned, such as:
     * * Yesterday, 3:00pm
     * * Today, 3:00pm
     * * Tomorrow, 3:00pm
     * * Last Tuesday, 3:00pm
     * * Next Wednesday, 3:00pm
     *
     * If the datetime falls outside this period then the format is used
     */
    Q_INVOKABLE QString formatRelativeDateTime(const QDateTime &dateTime, QLocale::FormatType format) const;

private:
    const KFormat m_format;
};

#endif
