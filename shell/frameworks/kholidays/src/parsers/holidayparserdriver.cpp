/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2010 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "astroseasons.h"
#include "holiday_p.h"
#include "holidayparserdriver_p.h"

#include <kholidays_debug.h>

using namespace KHolidays;

HolidayParserDriver::HolidayParserDriver(const QString &filePath)
    : m_parseYear(0)
    , m_parseStartYear(0)
    , m_parseEndYear(0)
{
    m_filePath = filePath;
}

HolidayParserDriver::~HolidayParserDriver()
{
}

QString HolidayParserDriver::fileCountryCode() const
{
    return m_fileCountryCode;
}

QString HolidayParserDriver::fileLanguageCode() const
{
    return m_fileLanguageCode;
}

QString HolidayParserDriver::fileName() const
{
    return m_fileName;
}

QString HolidayParserDriver::fileDescription() const
{
    return m_fileDescription;
}

Holiday::List HolidayParserDriver::parseHolidays(const QDate &startDate, const QDate &endDate, const QString &category)
{
    parseHolidays(startDate, endDate);
    Holiday::List m_resultListTemp = m_resultList;
    m_resultList.clear();

    for (const KHolidays::Holiday &holidayCat : m_resultListTemp) {
        for (const QString &mCategoryList : holidayCat.categoryList()) {
            if (mCategoryList == category) {
                m_resultList.append(holidayCat);
                break;
            }
        }
    }

    return m_resultList;
}

Holiday::List HolidayParserDriver::parseRawHolidays(const QDate &startDate, const QDate &endDate)
{
    m_resultList.clear();
    if (startDate.isNull() || endDate.isNull()) {
        return m_resultList;
    }
    m_requestStart = startDate;
    m_requestEnd = endDate;
    parse();
    return m_resultList;
}

Holiday::List HolidayParserDriver::parseHolidays(const QDate &startDate, const QDate &endDate)
{
    parseRawHolidays(startDate, endDate);

    for (int year = startDate.year(); year <= endDate.year(); ++year) {
        for (auto s : {AstroSeasons::JuneSolstice, AstroSeasons::DecemberSolstice, AstroSeasons::MarchEquinox, AstroSeasons::SeptemberEquinox}) {
            const auto dt = AstroSeasons::seasonDate(s, year);
            if (dt >= startDate && dt <= endDate) {
                Holiday season;
                season.d->mDayType = Holiday::Workday;
                season.d->mObservedDate = dt;
                season.d->mDuration = 1;
                season.d->mName = AstroSeasons::seasonName(s);
                season.d->mCategoryList.append(QLatin1String("seasonal"));
                m_resultList.append(season);
            }
        }
    }

    std::sort(m_resultList.begin(), m_resultList.end());
    return m_resultList;
}

Holiday::List HolidayParserDriver::parseHolidays(const QDate &requestDate)
{
    return parseHolidays(requestDate, requestDate);
}

Holiday::List HolidayParserDriver::parseHolidays(int calendarYear, QCalendarSystem::CalendarSystem calendar)
{
    m_resultList.clear();
    setParseCalendar(calendar);
    if (!m_parseCalendar.isValid(calendarYear, 1, 1)) {
        return m_resultList;
    }

    return parseHolidays(m_parseCalendar.firstDayOfYear(calendarYear), //
                         m_parseCalendar.lastDayOfYear(calendarYear));
}

void HolidayParserDriver::error(const QString &errorMessage)
{
    qCDebug(KHOLIDAYS_LOG) << errorMessage;
}

void HolidayParserDriver::parse()
{
}

void HolidayParserDriver::parseMetadata()
{
}

void HolidayParserDriver::setParseCalendar(QCalendarSystem::CalendarSystem calendar)
{
    m_parseCalendar = QCalendarSystem(calendar);
}

void HolidayParserDriver::setParseStartEnd()
{
    // Set start year and end year to generate holidays for
    // TODO Maybe make +/- 1 more year to allow spanned holidays from previous/following years
    // Make sure requested date range falls within valid date range for current calendar system
    if (m_requestStart > m_parseCalendar.latestValidDate() //
        || m_requestEnd < m_parseCalendar.earliestValidDate()) {
        // Completely out of range, don't parse
        m_parseStartYear = 0;
        m_parseEndYear = m_parseStartYear - 1;
    } else {
        if (m_requestStart < m_parseCalendar.earliestValidDate()) {
            m_parseStartYear = m_parseCalendar.year(m_parseCalendar.earliestValidDate());
        } else {
            m_parseStartYear = m_parseCalendar.year(m_requestStart);
        }

        if (m_requestEnd > m_parseCalendar.latestValidDate()) {
            m_parseEndYear = m_parseCalendar.year(m_parseCalendar.latestValidDate());
        } else {
            m_parseEndYear = m_parseCalendar.year(m_requestEnd);
        }
    }
}
