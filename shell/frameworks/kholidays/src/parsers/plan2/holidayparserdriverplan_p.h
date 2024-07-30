/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2010 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KHOLIDAYS_HOLIDAYPARSERDRIVERPLAN_P_H
#define KHOLIDAYS_HOLIDAYPARSERDRIVERPLAN_P_H

#include "../holidayparserdriver_p.h"

#include <QByteArray>
#include <QStringList>

#include <string>

namespace KHolidays
{
class HolidayScannerPlan;
class HolidayParserPlan;
class location;

/**
 * HolidayParserDriverPlan implementation class
 *
 * Implements the standard driver interface for parsing Plan holiday files as
 * implemented in KDE SC 4.5 onwards.  This includes extensions to the file
 * format to support non-Gregorian calendar systems, metadata, and other new
 * features.
 *
 * Implemented using Bison/Flex and the Bison C++ skeleton v2.1a, see README.
 * While almost all code is new, the structure of the parser is based on the
 * original Plan parser.
 *
 * @internal Private, for internal use only
 */
class HolidayParserDriverPlan : public HolidayParserDriver
{
public:
    /**
     * Constructor of Plan file parser driver
     *
     * @param filePath full path to holiday file
     */
    explicit HolidayParserDriverPlan(const QString &planFilename);

    /**
     * Destructor.
     */
    ~HolidayParserDriverPlan() override;

    /**
     * Bison C++ skeleton error message handling
     *
     * @param errorMessage error message to log
     */
    void error(const KHolidays::location &errorLocation, const QString &errorMessage);

    /**
     * Standard error message handling
     *
     * @param errorMessage error message to log
     */
    void error(const QString &errorMessage) override;

protected:
    /**
     * Actually parse the file, new plan format implementation
     */
    void parse() override;

    /**
     * Parse the file for metadata only and populate the metadata variables
     */
    void parseMetadata() override;

    /**
     * Set the calendar system to use
     *
     * @param calendar The QCalendarSystem calendar system to use
     */
    void setParseCalendar(QCalendarSystem::CalendarSystem calendar) override;

    // Bison C++ skeleton required friend for Bison parser class implementation
    friend class HolidayParserPlan;

    // Accessor methods for parser to talk to driver
    QString filePath();
    std::string *fileToParse() const;

    // Calendar and date calculation utilities
    int adjustedMonthNumber(int month);
    bool isLeapYear(int year);
    int parseYear();

    // Utilities for parser to calculate interim Julian Day Number during parsing
    int julianDayFromEventName(const QString &eventName);
    int julianDayFromEaster();
    int julianDayFromPascha();
    int julianDayFromMonthDay(int month, int day);
    int julianDayFromRelativeWeekday(int occurrence, int weekday, int jd);
    int julianDayFromWeekdayInMonth(int occurrence, int weekday, int month);

    // Utilities for parser to set variables during parsing
    void setParseCalendar(const QString &calendarType);
    void setFileCountryCode(const QString &countryCode);
    void setFileLanguageCode(const QString &languageCode);
    void setFileName(const QString &ame);
    void setFileDescription(const QString &description);
    void setEventName(const QString &eventName);
    void setEventCategory(const QString &category);
    void setEventCalendarType(const QString &calendarType = QStringLiteral("gregorian"));
    void setEventDate(int eventYear, int eventMonth, int eventDay);
    void setEventDate(int jd);

    // Terminal functions for parser to create holidays from parsed variables
    void setFromEaster(int offset, int duration);
    void setFromPascha(int offset, int duration);
    void setFromDate(int offset, int condition, int duration);
    void setFromWeekdayInMonth(int occurrence, int weekday, int month, int offset, int duration);
    void setFromRelativeWeekday(int occurrence, int weekday, int offset, int duration);
    void setEvent(int event_jd, int observe_offset, int duration);

private:
    // Calendar and date calculation utilities
    int julianDay(int year, int month, int day);
    void julianDayToDate(int jd, int *year, int *month, int *day);
    QDate easter(int year);
    QDate pascha(int year);
    QCalendarSystem::CalendarSystem typeToSystem(const QString &calendarType);
    QString systemToType(QCalendarSystem::CalendarSystem calendar);

    int conditionalOffset(int year, int month, int day, int condition);

    void addHoliday(const QDate &date, int duration);

    QByteArray m_scanData; // Holiday file stored as a string

    QStringList m_fileCalendarTypes; // List of all Calendar Types used in file
    QString m_parseCalendarType; // Calendar Type being parsed

    bool m_traceParsing; // Bison C++ skeleton enable tracing in Bison parser class
    HolidayParserPlan *m_parser; // Bison C++ skeleton Bison parser class implementation

    bool m_traceScanning; // Flex C++ enable tracing in Flex scanner class
    HolidayScannerPlan *m_scanner; // Flex C++ scanner class implementation

    bool m_parseMetadataOnly; // Only parse file for metadata
    QDate m_parseYearStart; // First day of year being parsed
    QDate m_parseYearEaster; // Easter in the parse year, Gregorian only
    QDate m_parseYearPascha; // Orthodox Easter in the parse year, Gregorian only

    QStringList m_eventCategories; // Event categories
    QString m_eventCalendarType; // Calendar Type for event rule
    QString m_eventName; // Event name text
    int m_eventYear; // Event date fields
    int m_eventMonth; // Event date fields
    int m_eventDay; // Event date fields

    std::string *m_fileToParse = nullptr;
};

}

#endif // KHOLIDAYS_HOLIDAYPARSERDRIVERPLAN_P_H
