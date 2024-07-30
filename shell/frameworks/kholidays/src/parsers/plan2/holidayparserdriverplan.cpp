/*
    Original version from plan:
        SPDX-FileCopyrightText: Thomas Driemeyer <thomas@bitrot.de>

    Adapted for use in KOrganizer:
        SPDX-FileCopyrightText: Preston Brown <pbrown@kde.org>
        SPDX-FileCopyrightText: Reinhold Kainhofer <reinhold@kainhofer.com>

    Portions contributed:
        SPDX-FileCopyrightText: Peter Littlefield <plittle@sofkin.ca>
        SPDX-FileCopyrightText: Armin Liebl <liebla@informatik.tu-muenchen.de>
        SPDX-FileCopyrightText: Efthimios Mavrogeorgiadis <emav@enl.auth.gr>
        SPDX-FileCopyrightText: Erwin Hugo Achermann <acherman@inf.ethz.ch>

    Major rewrite using Bison C++ skeleton:
        SPDX-FileCopyrightText: 2010 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "holiday_p.h"
#include "holidayparserdriverplan_p.h"
#include "holidayparserplan.hpp"
#include "holidayscannerplan_p.h"
#include <kholidays_debug.h>

#include <QFileInfo>

#include <sstream>

#define LAST 99999
#define ANY (-99999)
#define BEFORE (-1)
#define AFTER 1

using namespace KHolidays;

HolidayParserDriverPlan::HolidayParserDriverPlan(const QString &planFilePath)
    : HolidayParserDriver(planFilePath)
    , m_traceParsing(false)
    , m_traceScanning(false)
    , m_parseMetadataOnly(false)
    , m_eventYear(0)
    , m_eventMonth(0)
    , m_eventDay(0)
{
    QFile holidayFile(filePath());
    if (holidayFile.open(QIODevice::ReadOnly)) {
        m_scanData = holidayFile.readAll();
        holidayFile.close();
    }
    m_scanner = new HolidayScannerPlan();
    m_scanner->set_debug(m_traceScanning);
    m_parser = new HolidayParserPlan(*this);
    m_parser->set_debug_level(m_traceParsing);
    m_fileToParse = new std::string(filePath().toLocal8Bit().data());
    parseMetadata();
}

HolidayParserDriverPlan::~HolidayParserDriverPlan()
{
    delete m_parser;
    delete m_scanner;
    delete m_fileToParse;
}

// TODO Figure why it doesn't compile
void HolidayParserDriverPlan::error(const KHolidays::location &errorLocation, const QString &errorMessage)
{
    Q_UNUSED(errorLocation);
    // std::cerr << errorLocation << " : " << errorMessage;  //Doesn't work???
    // qDebug() << errorLocation << " : " << errorMessage;  //Doesn't work???
    qCDebug(KHOLIDAYS_LOG) << errorMessage;
}

void HolidayParserDriverPlan::error(const QString &errorMessage)
{
    qCDebug(KHOLIDAYS_LOG) << errorMessage;
}

void HolidayParserDriverPlan::parse()
{
    // Parse the file using every calendar system in the file
    for (const QString &calendarType : std::as_const(m_fileCalendarTypes)) {
        // Cater for events defined in other Calendar Systems where request year could cover 2 or 3 event years
        // Perhaps also parse year before and year after to allow events to span years or shift to other year?
        setParseCalendar(calendarType);
        setParseStartEnd();

        // Generate all events for this calendar in the required year(s)
        for (m_parseYear = m_parseStartYear; m_parseYear <= m_parseEndYear; ++m_parseYear) {
            m_parseYearStart = m_parseCalendar.firstDayOfYear(m_parseYear);
            m_parseYearEaster = easter(m_parseYear);
            m_parseYearPascha = pascha(m_parseYear);

            std::istringstream iss2(std::string(m_scanData.data()));
            m_scanner->yyrestart(&iss2);

            m_parser->parse();
        }
    }
}

void HolidayParserDriverPlan::parseMetadata()
{
    m_parseMetadataOnly = true;
    m_fileCountryCode.clear();
    m_fileLanguageCode.clear();
    m_fileName.clear();
    m_fileDescription.clear();
    m_fileCalendarTypes.clear();
    m_fileCalendarTypes.append(QStringLiteral("gregorian"));

    // Default to files internal metadata
    setParseCalendar(QStringLiteral("gregorian"));
    m_parseYear = QDate::currentDate().year();
    std::istringstream iss2(std::string(m_scanData.data()));
    m_scanner->yyrestart(&iss2);
    m_parser->parse();
    m_resultList.clear();

    // If not populated, then use filename metadata, this may change later
    // metadata is encoded in filename in form holiday_<region>_<type>_<language>_<name>
    // with region, type and language sub groups separated by -, and with name optional
    QFileInfo file(m_filePath);
    if (file.exists()) {
        QStringList metadata = file.fileName().split(QLatin1Char('_'));
        if (metadata[0] == QLatin1String("holiday") && metadata.count() > 2) {
            if (m_fileCountryCode.isEmpty()) {
                setFileCountryCode(metadata[1].toUpper());
            }
            if (m_fileLanguageCode.isEmpty()) {
                QStringList language = metadata[2].split(QLatin1Char('-'));
                m_fileLanguageCode = language[0];
                if (language.count() > 1) {
                    setFileLanguageCode(language[0].append(QLatin1Char('_')).append(language[1].toUpper()));
                } else {
                    setFileLanguageCode(language[0]);
                }
            }
            if (m_fileLanguageCode.isEmpty() && metadata.count() > 3) {
                m_fileName = metadata[3];
            }
        }
    }

    m_parseMetadataOnly = false;
}

void HolidayParserDriverPlan::setParseCalendar(QCalendarSystem::CalendarSystem calendar)
{
    m_parseCalendarType = systemToType(calendar);
    HolidayParserDriver::setParseCalendar(calendar);
}

QString HolidayParserDriverPlan::filePath()
{
    return m_filePath;
}

std::string *HolidayParserDriverPlan::fileToParse() const
{
    return m_fileToParse;
}

/*****************************************
  Calendar and Date convenience routines
******************************************/

// Adjust month numbering for Hebrew civil calendar leap month
int HolidayParserDriverPlan::adjustedMonthNumber(int month)
{
    if (m_eventCalendarType != QLatin1String("hebrew") || // Only adjust Hebrew months
        m_parseCalendarType != QLatin1String("hebrew") || !m_parseCalendar.isLeapYear(m_parseYear) || // Only adjust in leap year
        month < 6) { // Only adjust from Adar onwards
        return month;
    }

    if (month == 13) { // Adar I
        return 6;
    }

    if (month == 14) { // Adar II
        return 7;
    }

    return month + 1; // Inserting Adar I moves other months up by 1
}

bool HolidayParserDriverPlan::isLeapYear(int year)
{
    return m_parseCalendar.isLeapYear(year);
}

int HolidayParserDriverPlan::parseYear()
{
    return m_parseYear;
}

int HolidayParserDriverPlan::julianDay(int year, int month, int day)
{
    return m_parseCalendar.date(year, month, day).toJulianDay();
}

void HolidayParserDriverPlan::julianDayToDate(int jd, int *year, int *month, int *day)
{
    QDate tempDate = QDate::fromJulianDay(jd);

    if (year) {
        *year = m_parseCalendar.year(tempDate);
    }
    if (month) {
        *month = m_parseCalendar.month(tempDate);
    }
    if (day) {
        *day = m_parseCalendar.day(tempDate);
    }
}

QDate HolidayParserDriverPlan::easter(int year)
{
    if (m_parseCalendar.calendarSystem() != QCalendarSystem::GregorianCalendar) {
        return QDate();
    }
    if (year < 0) {
        return QDate();
    }

    // Algorithm taken from Tondering
    // https://www.tondering.dk/claus/cal/easter.php
    int g = year % 19;
    int c = year / 100;
    int h = (c - (c / 4) - (((8 * c) + 13) / 25) + (19 * g) + 15) % 30;
    int i = h - ((h / 28) * (1 - ((29 / (h + 1)) * ((21 - g) / 11))));
    int j = (year + (year / 4) + i + 2 - c + (c / 4)) % 7;
    int l = i - j;
    int month = 3 + ((l + 40) / 44);
    int day = l + 28 - (31 * (month / 4));

    return QDate::fromJulianDay(julianDay(year, month, day));
}

QDate HolidayParserDriverPlan::pascha(int year)
{
    if (year < 0) {
        return QDate();
    }

    if (m_parseCalendar.calendarSystem() == QCalendarSystem::GregorianCalendar || m_parseCalendar.calendarSystem() == QCalendarSystem::JulianCalendar) {
        // Algorithm taken from Tondering
        // https://www.tondering.dk/claus/cal/easter.php#orthodoxeast
        // Gives Orthodox Easter in the Julian Calendar, need to convert afterwards to Gregorian if needed
        int g = year % 19;
        int i = ((19 * g) + 15) % 30;
        int j = (year + (year / 4) + i) % 7;
        int l = i - j;
        int month = 3 + ((l + 40) / 44);
        int day = l + 28 - (31 * (month / 4));

        if (m_parseCalendar.calendarSystem() == QCalendarSystem::JulianCalendar) {
            return QDate::fromJulianDay(julianDay(year, month, day));
        }

        if (m_parseCalendar.calendarSystem() == QCalendarSystem::GregorianCalendar) {
            setParseCalendar(QStringLiteral("julian"));
            int paschaJd = julianDay(year, month, day);
            setParseCalendar(QStringLiteral("gregorian"));
            return QDate::fromJulianDay(paschaJd);
        }
    }

    return QDate();
}

QCalendarSystem::CalendarSystem HolidayParserDriverPlan::typeToSystem(const QString &calendarType)
{
    if (calendarType == QStringLiteral("gregorian")) {
        return QCalendarSystem::GregorianCalendar;
    } else if (calendarType == QStringLiteral("hebrew")) {
        return QCalendarSystem::HebrewCalendar;
    } else if (calendarType == QStringLiteral("hijri")) {
        return QCalendarSystem::IslamicCivilCalendar;
    } else if (calendarType == QStringLiteral("jalali")) {
        return QCalendarSystem::PersianCalendar;
    } else if (calendarType == QStringLiteral("julian")) {
        return QCalendarSystem::JulianCalendar;
    } else if (calendarType == QStringLiteral("coptic")) {
        return QCalendarSystem::CopticCalendar;
    } else if (calendarType == QStringLiteral("ethiopian")) {
        return QCalendarSystem::EthiopicCalendar;
    } else if (calendarType == QStringLiteral("indiannational")) {
        return QCalendarSystem::IndianNationalCalendar;
    }
    return QCalendarSystem::GregorianCalendar;
}

QString HolidayParserDriverPlan::systemToType(QCalendarSystem::CalendarSystem calendar)
{
    switch (calendar) {
    case QCalendarSystem::GregorianCalendar:
        return QStringLiteral("gregorian");
    case QCalendarSystem::HebrewCalendar:
        return QStringLiteral("hebrew");
    case QCalendarSystem::IslamicCivilCalendar:
        return QStringLiteral("hijri");
    case QCalendarSystem::PersianCalendar:
        return QStringLiteral("jalali");
    case QCalendarSystem::JulianCalendar:
        return QStringLiteral("julian");
    case QCalendarSystem::CopticCalendar:
        return QStringLiteral("coptic");
    case QCalendarSystem::EthiopicCalendar:
        return QStringLiteral("ethiopian");
    case QCalendarSystem::IndianNationalCalendar:
        return QStringLiteral("indiannational");
    default:
        return QStringLiteral("gregorian");
    }
}

/*************************
 * Calculate jd routines *
 *************************/

// Return the jd of an existing event, assumes unique names and correct order in file
int HolidayParserDriverPlan::julianDayFromEventName(const QString &eventName)
{
    for (const KHolidays::Holiday &thisHoliday : std::as_const(m_resultList)) {
        if (thisHoliday.name() == eventName) {
            return thisHoliday.observedStartDate().toJulianDay();
        }
    }
    return -1;
}

// Return jd of Easter if Gregorian
int HolidayParserDriverPlan::julianDayFromEaster()
{
    if (m_eventCalendarType == QLatin1String("gregorian")) {
        return m_parseYearEaster.toJulianDay();
    } else {
        error(QStringLiteral("Can only use Easter in Gregorian event rule"));
        return -1;
    }
}

// Return jd of Orthodox Easter if Gregorian or Julian
int HolidayParserDriverPlan::julianDayFromPascha()
{
    if (m_eventCalendarType == QLatin1String("gregorian") || m_eventCalendarType == QLatin1String("julian")) {
        return m_parseYearPascha.toJulianDay();
    } else {
        error(QStringLiteral("Can only use Easter in Gregorian or Julian event rule"));
        return -1;
    }
}

// Return jd of weekday from a month/day in parse year
int HolidayParserDriverPlan::julianDayFromMonthDay(int month, int day)
{
    return julianDay(m_parseYear, month, day);
}

// Return jd of weekday relative to a Julian Day number
int HolidayParserDriverPlan::julianDayFromRelativeWeekday(int occurrence, int weekday, int jd)
{
    if (occurrence == ANY) { /* Should never get this, convert to AFTER instead */
        occurrence = AFTER;
    }

    int thisWeekday = m_parseCalendar.dayOfWeek(QDate::fromJulianDay(jd));

    /* AFTER actually means on or after */
    /* BEFORE actually means on or before */
    if (occurrence > 0) {
        occurrence = occurrence - 1;
    } else if (occurrence < 0 && weekday == thisWeekday) {
        occurrence = occurrence + 1;
    }

    if (weekday < thisWeekday) {
        occurrence = occurrence + 1;
    }

    return jd + weekday - thisWeekday + (occurrence * 7);
}

// Return jd of weekday occurrence in a given month and day in the parse year
int HolidayParserDriverPlan::julianDayFromWeekdayInMonth(int occurrence, int weekday, int month)
{
    if (occurrence == LAST) { // Is weekday on or before last day of month
        return julianDayFromRelativeWeekday(BEFORE, weekday, julianDay(m_parseYear, month, m_parseCalendar.daysInMonth(m_parseYear, month)));
    } else { // Is nth weekday on or after first day of month
        return julianDayFromRelativeWeekday(occurrence, weekday, julianDay(m_parseYear, month, 1));
    }
}

/****************************************************
 * Set parsed event variables convenience functions *
 ****************************************************/

void HolidayParserDriverPlan::setParseCalendar(const QString &calendarType)
{
    m_parseCalendarType = calendarType;
    setParseCalendar(typeToSystem(calendarType));
}

void HolidayParserDriverPlan::setFileCountryCode(const QString &countryCode)
{
    m_fileCountryCode = countryCode;
}

void HolidayParserDriverPlan::setFileLanguageCode(const QString &languageCode)
{
    m_fileLanguageCode = languageCode;
}

void HolidayParserDriverPlan::setFileName(const QString &name)
{
    m_fileName = name;
}

void HolidayParserDriverPlan::setFileDescription(const QString &description)
{
    m_fileDescription = description;
}

void HolidayParserDriverPlan::setEventName(const QString &eventName)
{
    // Assume if setting an event name then is start of new event line, so clear categories
    m_eventCategories.clear();
    m_eventName = eventName;
}

void HolidayParserDriverPlan::setEventCategory(const QString &category)
{
    m_eventCategories.append(category);
}

void HolidayParserDriverPlan::setEventCalendarType(const QString &calendarType)
{
    m_eventCalendarType = calendarType;
    if (m_parseMetadataOnly && !m_fileCalendarTypes.contains(calendarType)) {
        m_fileCalendarTypes.append(calendarType);
    }
}

void HolidayParserDriverPlan::setEventDate(int eventYear, int eventMonth, int eventDay)
{
    m_eventYear = eventYear;
    m_eventMonth = eventMonth;
    m_eventDay = eventDay;
}

void HolidayParserDriverPlan::setEventDate(int jd)
{
    julianDayToDate(jd, &m_eventYear, &m_eventMonth, &m_eventDay);
}

/********************************************
 * Set event date from event rules routines *
 ********************************************/

/*
 * Set event by weekday (Monday..Sunday). The rule expression is
 * "every <occurrence> <weekday> of <month> plus <offset> days length <duration> days".
 * Occurrence and month can be ANY or LAST, offset and duration are optional.
 */

void HolidayParserDriverPlan::setFromWeekdayInMonth(int occurrence, int weekday, int month, int offset, int duration)
{
    // Don't set if only parsing metadata or calendar for event rule is not the current parse calendar
    if (m_parseMetadataOnly || m_eventCalendarType != m_parseCalendarType) {
        return;
    }

    int startMonth, endMonth;
    if (month == LAST) {
        startMonth = m_parseCalendar.monthsInYear(m_parseYear);
        endMonth = startMonth;
    } else if (month == ANY) {
        startMonth = 1;
        endMonth = m_parseCalendar.monthsInYear(m_parseYear);
    } else {
        startMonth = month;
        endMonth = month;
    }

    // Generate all events in the required event month(s)
    for (int thisMonth = startMonth; thisMonth <= endMonth; ++thisMonth) {
        if (m_parseCalendar.isValid(m_parseYear, thisMonth, 1)) {
            int startOccurrence, endOccurrence;
            if (occurrence == ANY) { // Generate 1st through 5th weekdays, assumes no month with > 35 days
                startOccurrence = 1;
                endOccurrence = 5;
            } else { // Generate just nth or LAST weekday
                startOccurrence = occurrence;
                endOccurrence = occurrence;
            }

            int jdMonthStart = julianDay(m_parseYear, thisMonth, 1);
            int jdMonthEnd = julianDay(m_parseYear, thisMonth, m_parseCalendar.daysInMonth(m_parseYear, thisMonth));

            // Generate each required occurrence of weekday in month, check occurrence actually falls in month
            for (int thisOccurrence = startOccurrence; thisOccurrence <= endOccurrence; ++thisOccurrence) {
                int thisJd = julianDayFromWeekdayInMonth(thisOccurrence, weekday, thisMonth);
                if (thisJd >= jdMonthStart && thisJd <= jdMonthEnd) {
                    setEvent(thisJd + offset, 0, duration);
                }
            }
        }
    }
}

/*
 * Set event by weekday (Monday..Sunday) relative to a date. The expression is
 * "<weekday> <occurrence> <date> plus <offset> days length <duration> days".
 * Occurrence, month and day can be ANY or LAST, year can be ANY, offset and duration are optional.
 */

void HolidayParserDriverPlan::setFromRelativeWeekday(int occurrence, int weekday, int offset, int duration)
{
    // Don't set if only parsing metadata or calendar for event rule is not the current parse calendar
    if (m_parseMetadataOnly || m_eventCalendarType != m_parseCalendarType) {
        return;
    }

    int thisYear;
    if (m_eventYear == ANY) { // Generate the parse year
        thisYear = m_parseYear;
    } else { // Generate a specific event year
        thisYear = m_eventYear;
    }

    int startMonth, endMonth;
    if (m_eventMonth == LAST) { // Generate just the last month
        startMonth = m_parseCalendar.monthsInYear(thisYear);
        endMonth = startMonth;
    } else if (m_eventMonth == ANY) { // Generate every month
        startMonth = 1;
        endMonth = m_parseCalendar.monthsInYear(thisYear);
    } else { // Generate just the specific event month
        startMonth = m_eventMonth;
        endMonth = m_eventMonth;
    }

    // Generate all events in the required month(s)
    int thisMonth;
    for (thisMonth = startMonth; thisMonth <= endMonth; ++thisMonth) {
        int startDay, endDay;
        if (m_eventDay == LAST) { // Generate just the last day in the month
            startDay = m_parseCalendar.daysInMonth(thisYear, thisMonth);
            endDay = startDay;
        } else if (m_eventDay == ANY) { // Generate every day in the month
            startDay = 1;
            endDay = m_parseCalendar.daysInMonth(thisYear, thisMonth);
        } else { // Generate just the specific event day
            startDay = m_eventDay;
            endDay = m_eventDay;
        }

        // Generate all events on the required day(s)
        for (int thisDay = startDay; thisDay <= endDay; ++thisDay) {
            if (m_parseCalendar.isValid(thisYear, thisMonth, thisDay)) {
                int relativeJd = julianDayFromRelativeWeekday(occurrence, weekday, julianDay(thisYear, thisMonth, thisDay));
                setEvent(relativeJd + offset, 0, duration);
            }
        }
    }
}

// TODO Figure out how this works :-)
int HolidayParserDriverPlan::conditionalOffset(int year, int month, int day, int condition)
{
    /** The encoding of the condition is:
          8 lower bits: conditions to shift (bit-register, bit 1=mon, ..., bit 7=sun)
          8 higher bits: weekday to shift to (bit-register, bit 1=mon, ..., bit 7=sun)
    */

    int offset = 0;

    int weekday = m_parseCalendar.dayOfWeek(year, month, day);

    if (condition & (1 << weekday)) {
        /* condition matches -> higher 8 bits contain the possible days to shift to */
        int to = (condition >> 8);
        while (!(to & (1 << ((weekday + offset) % 7))) && (offset < 8)) {
            ++offset;
        }
    }

    if (offset >= 8) {
        offset = 0;
    }

    return offset;
}

/*
 * Set event by date. The expression is
 * "<date> plus <offset> days shift <condition> length <duration> days".
 * Occurrence, month and day can be ANY or LAST, year can be ANY, offset and duration are optional.
 */

void HolidayParserDriverPlan::setFromDate(int offset, int condition, int duration)
{
    // Don't set if only parsing metadata or calendar for event rule is not the current parse calendar
    if (m_parseMetadataOnly || m_eventCalendarType != m_parseCalendarType) {
        return;
    }

    int thisYear;
    if (m_eventYear == ANY) { // Generate the parse year
        thisYear = m_parseYear;
    } else { // Generate a specific event year
        thisYear = m_eventYear;
    }

    int startMonth, endMonth;
    if (m_eventMonth == LAST) { // Generate just the last month
        startMonth = m_parseCalendar.monthsInYear(thisYear);
        endMonth = startMonth;
    } else if (m_eventMonth == ANY) { // Generate every month
        startMonth = 1;
        endMonth = m_parseCalendar.monthsInYear(thisYear);
    } else { // Generate just the specific event month
        startMonth = m_eventMonth;
        endMonth = m_eventMonth;
    }

    // Generate all events in the required month(s)
    for (int thisMonth = startMonth; thisMonth <= endMonth; ++thisMonth) {
        int startDay, endDay;
        if (m_eventDay == LAST) { // Generate just the last day in the month
            startDay = m_parseCalendar.daysInMonth(thisYear, thisMonth);
            endDay = startDay;
        } else if (m_eventDay == ANY) { // Generate every day in the month
            startDay = 1;
            endDay = m_parseCalendar.daysInMonth(thisYear, thisMonth);
        } else { // Generate just the specific event day
            startDay = m_eventDay;
            endDay = m_eventDay;
        }

        // Generate all events on the required day(s)
        for (int thisDay = startDay; thisDay <= endDay; ++thisDay) {
            if (m_parseCalendar.isValid(thisYear, thisMonth, thisDay)) {
                setEvent(julianDay(thisYear, thisMonth, thisDay) + offset, //
                         conditionalOffset(thisYear, thisMonth, thisDay, condition), //
                         duration);
            }
        }
    }
}

/*
 * Set event relative to Easter. The expression is
 * "EASTER plus <offset> days length <duration> days".
 * Offset and duration are optional.
 */

void HolidayParserDriverPlan::setFromEaster(int offset, int duration)
{
    // Don't set if only parsing metadata or calendar for event rule is not the current parse calendar
    if (m_parseMetadataOnly || m_eventCalendarType != m_parseCalendarType) {
        return;
    }

    if (m_eventCalendarType == QLatin1String("gregorian")) {
        setEvent(m_parseYearEaster.toJulianDay() + offset, 0, duration);
    } else {
        error(QStringLiteral("Can only use Easter in Gregorian event rule"));
    }
}

/*
 * Set event relative to Pascha. The expression is
 * "PASCHA plus <offset> days length <duration> days".
 * Offset and duration are optional.
 */

void HolidayParserDriverPlan::setFromPascha(int offset, int duration)
{
    // Don't set if only parsing metadata or calendar for event rule is not the current parse calendar
    if (m_parseMetadataOnly || m_eventCalendarType != m_parseCalendarType) {
        return;
    }

    if (m_eventCalendarType == QLatin1String("gregorian") || m_eventCalendarType == QLatin1String("julian")) {
        setEvent(m_parseYearPascha.toJulianDay(), offset, duration);
    } else {
        error(QStringLiteral("Can only use Pascha in Julian and Gregorian event rule"));
    }
}

// Set the event if it falls inside the requested date range
void HolidayParserDriverPlan::setEvent(int jd, int observeOffset, int duration)
{
    // Don't set if only parsing metadata or calendar for event rule is not the current parse calendar
    if (m_parseMetadataOnly || m_eventCalendarType != m_parseCalendarType) {
        return;
    }

    // Date the holiday will be observed on
    int observeJd = jd + observeOffset;

    addHoliday(QDate::fromJulianDay(observeJd), duration);
}

void HolidayParserDriverPlan::addHoliday(const QDate &observedDate, int duration)
{
    // Only set if event falls in requested date range, i.e. either starts or ends during range
    if (m_parseCalendar.isValid(observedDate) //
        && observedDate <= m_requestEnd //
        && observedDate.addDays(duration - 1) >= m_requestStart) {
        KHolidays::Holiday holiday;
        holiday.d->mObservedDate = observedDate;
        holiday.d->mDuration = duration;
        holiday.d->mName = m_eventName;
        holiday.d->mDescription = m_eventName;
        holiday.d->mCategoryList = m_eventCategories;
        if (m_eventCategories.contains(QStringLiteral("public"))) {
            holiday.d->mDayType = KHolidays::Holiday::NonWorkday;
        } else {
            holiday.d->mDayType = KHolidays::Holiday::Workday;
        }
        m_resultList.append(holiday);
    }
}
