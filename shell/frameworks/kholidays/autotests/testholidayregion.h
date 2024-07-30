/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2010 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTHOLIDAYREGION_H
#define TESTHOLIDAYREGION_H

#include <QObject>

#include "holidayregion.h"

class QString;
class QDate;

class HolidayRegionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLoadFileCalendarSystems();
    void testLoadFile();
    void testGb();
    void testIran();
    void testIsrael();
    void testRegions();
    void testDefaultRegions();
    void testSolistaleInHolidays();
    void testLoadFileCalendarSystemsForPlasma();
    void testDominicanRepublicDiadelaConstitucion();
    void testUsAmericanJuneteenth();

private:
    void printMetadata(const KHolidays::HolidayRegion &region);
    void printHolidays(const KHolidays::Holiday::List &holidays);
    void parseRegionCalendarYear(const KHolidays::HolidayRegion &region, int year, const QString &calendarType = "gregorian");
    void parseRegionDateRange(const KHolidays::HolidayRegion &region, const QDate &startDate, const QDate &endDate);
    void parseRawRegionDateRange(const KHolidays::HolidayRegion &region, const QDate &startDate, const QDate &endDate);
    void parseRegionDate(const KHolidays::HolidayRegion &region, const QDate &date);
    void parseRegionDateRangeCategory(const KHolidays::HolidayRegion &region, const QDate &startDate, const QDate &endDate, const QString &category);
};

#endif // TESTHOLIDAYREGION_H
