/*
    SPDX-FileCopyrightText: 2021 Gary Wang <wzc782970009@gmail.com>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <unicode/calendar.h>
#include <unicode/smpdtfmt.h>

#include <QCalendar>

class QString;

class ICUCalendarPrivate
{
public:
    /**
     * Initialize the Gregorian Calendar, which will be used in date conversion.
     */
    explicit ICUCalendarPrivate();
    virtual ~ICUCalendarPrivate();

    /**
     * Returns the value for a given time field in the alternate calendar.
     */
    int32_t year() const;
    int32_t month() const;
    int32_t day() const;

    /**
     * Returns the date from the alternate calendar.
     *
     * @return the date from the alternate calendar
     */
    QCalendar::YearMonthDay date() const;

    /**
     * Sets the date in the Gregorian Calendar, and convert the date to
     * the alternate calendar.
     *
     * @param date the Gregorian Calendar's date to be converted
     * @return @c true if the date is successfully set, @c false otherwise
     */
    bool setDate(const QDate &date);

    /**
     * Sets the alternate calendar's current time with the given time.
     *
     * @param time the Gregorian Calendar's time as milliseconds
     * @return @c true if the time is successfully set, @c false otherwise
     */
    bool setTime(double time);

protected:
    /**
     * Alternate calendar
     */
    std::unique_ptr<icu::Calendar> m_calendar;
    /**
     * Standard ICU4C error code.
     */
    mutable UErrorCode m_errorCode;

private:
    /**
     * Gregorian Calendar
     */
    const std::unique_ptr<icu::Calendar> m_GregorianCalendar;
};
