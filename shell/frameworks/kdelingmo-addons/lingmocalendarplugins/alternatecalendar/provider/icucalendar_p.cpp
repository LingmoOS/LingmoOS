/*
    SPDX-FileCopyrightText: 2021 Gary Wang <wzc782970009@gmail.com>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "icucalendar_p.h"

#include <QDate>
#include <QString>

ICUCalendarPrivate::ICUCalendarPrivate()
    : m_errorCode(U_ZERO_ERROR)
    , m_GregorianCalendar(icu::Calendar::createInstance("en_US@calendar=gregorian", m_errorCode))
{
}

ICUCalendarPrivate::~ICUCalendarPrivate()
{
}

int32_t ICUCalendarPrivate::year() const
{
    const int32_t year = m_calendar->get(UCAL_YEAR, m_errorCode);

    if (U_FAILURE(m_errorCode)) {
        return -1;
    }

    return year;
}

int32_t ICUCalendarPrivate::month() const
{
    const int32_t month = m_calendar->get(UCAL_MONTH, m_errorCode);

    if (U_FAILURE(m_errorCode)) {
        return -1;
    }

    return month + 1;
}

int32_t ICUCalendarPrivate::day() const
{
    const int32_t day = m_calendar->get(UCAL_DATE, m_errorCode);

    if (U_FAILURE(m_errorCode)) {
        return -1;
    }

    return day;
}

QCalendar::YearMonthDay ICUCalendarPrivate::date() const
{
    return QCalendar::YearMonthDay(year(), month(), day());
}

bool ICUCalendarPrivate::setDate(const QDate &date)
{
    // icu: Month value is 0-based. e.g., 0 for January.
    m_GregorianCalendar->set(date.year(), date.month() - 1, date.day());

    const UDate time = m_GregorianCalendar->getTime(m_errorCode);

    if (U_FAILURE(m_errorCode)) {
        return false;
    }

    m_calendar->setTime(time, m_errorCode);

    return !U_FAILURE(m_errorCode);
}

bool ICUCalendarPrivate::setTime(double time)
{
    m_calendar->setTime(time, m_errorCode);

    return !U_FAILURE(m_errorCode);
}
