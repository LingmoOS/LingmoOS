/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2010 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testholidayregion.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTest>

QTEST_MAIN(HolidayRegionTest)

void HolidayRegionTest::printMetadata(const KHolidays::HolidayRegion &region)
{
    if (region.isValid()) {
        qDebug() << "This regionCode = " << region.regionCode();
        qDebug() << "Is valid? = " << region.isValid();
        qDebug() << "Country code = " << region.countryCode();
        qDebug() << "Language code = " << region.languageCode();
        qDebug() << "Name = " << region.name();
        qDebug() << "Description = " << region.description();
    } else {
        qDebug() << "Not Valid!";
    }
    qDebug() << "";
}

void HolidayRegionTest::printHolidays(const KHolidays::Holiday::List &holidays)
{
    if (!holidays.isEmpty()) {
        for (const KHolidays::Holiday &holiday : holidays) {
            qDebug().nospace() << "Date = " << holiday.observedStartDate().toString(Qt::ISODate) << " Duration = " << holiday.duration()
                               << " Name = " << holiday.name().leftJustified(20, /*QChar fill=*/QLatin1Char(' '), /*bool truncate=*/false)
                               << " Category = " << holiday.categoryList();
        }
    } else {
        qDebug() << "No holidays";
    }
}

void HolidayRegionTest::parseRegionCalendarYear(const KHolidays::HolidayRegion &region, int year, const QString &calendarType)
{
    qDebug() << "Parsing region = " << region.regionCode() << " year = " << year << " calendar = " << calendarType;
    printHolidays(region.rawHolidaysWithAstroSeasons(year));
    qDebug() << "";
}

void HolidayRegionTest::parseRegionDateRange(const KHolidays::HolidayRegion &region, const QDate &startDate, const QDate &endDate)
{
    qDebug() << "Parsing regionCode = " << region.regionCode() << " start date = " << startDate.toString(Qt::ISODate)
             << " end date = " << endDate.toString(Qt::ISODate);
    printHolidays(region.rawHolidaysWithAstroSeasons(startDate, endDate));
    qDebug() << "";
}

void HolidayRegionTest::parseRawRegionDateRange(const KHolidays::HolidayRegion &region, const QDate &startDate, const QDate &endDate)
{
    qDebug() << "Parsing regionCode = " << region.regionCode() << " start date = " << startDate.toString(Qt::ISODate)
             << " end date = " << endDate.toString(Qt::ISODate);
    printHolidays(region.rawHolidaysWithAstroSeasons(startDate, endDate));
    qDebug() << "";
}

void HolidayRegionTest::parseRegionDate(const KHolidays::HolidayRegion &region, const QDate &date)
{
    qDebug() << "Parsing regionCode = " << region.regionCode() << " date = " << date.toString(Qt::ISODate);
    printHolidays(region.rawHolidaysWithAstroSeasons(date));
    qDebug() << "";
}

void HolidayRegionTest::parseRegionDateRangeCategory(const KHolidays::HolidayRegion &region,
                                                     const QDate &startDate,
                                                     const QDate &endDate,
                                                     const QString &category)
{
    qDebug() << "Parsing regionCode = " << region.regionCode() << " start date = " << startDate.toString(Qt::ISODate)
             << " end date = " << endDate.toString(Qt::ISODate) << " category = " << category;
    printHolidays(region.rawHolidays(startDate, endDate, category));
    qDebug() << "";
}

void HolidayRegionTest::testLoadFileCalendarSystems()
{
    KHolidays::HolidayRegion region(QFileInfo(KDESRCDIR "/holiday_calendar_systems"));
    printMetadata(region);
    parseRegionDateRangeCategory(region, QDate(2020, 7, 1), QDate(2021, 6, 30), "seasonal");
    parseRegionDateRangeCategory(region, QDate(2020, 1, 1), QDate(2022, 12, 31), "seasonal");
    parseRegionDateRangeCategory(region, QDate(2020, 1, 1), QDate(2020, 12, 31), "public");
    parseRegionCalendarYear(region, 2022);
    parseRegionCalendarYear(region, 2023);
    parseRegionCalendarYear(region, 2024);
    parseRegionCalendarYear(region, 2025);
    qDebug() << "";
}

void HolidayRegionTest::testLoadFile()
{
    KHolidays::HolidayRegion region(QFileInfo(KDESRCDIR "/holiday_gb-eaw_en-gb_Test"));
    printMetadata(region);
    parseRegionCalendarYear(region, 2020, QStringLiteral("gregorian"));
    parseRegionCalendarYear(region, 2020);
    parseRegionCalendarYear(region, 2021);
    parseRegionCalendarYear(region, 2022);
    parseRegionCalendarYear(region, 2023);
    parseRegionCalendarYear(region, 2024);
    parseRegionCalendarYear(region, 2025);
    qDebug() << "";
}

void HolidayRegionTest::testGb()
{
    KHolidays::HolidayRegion region(QStringLiteral("gb-eaw_en-gb"));
    printMetadata(region);
    parseRegionDateRange(region, QDate(2020, 7, 1), QDate(2021, 6, 30));
    parseRegionDateRange(region, QDate(2020, 1, 1), QDate(2022, 12, 31));
    parseRegionDateRange(region, QDate(2020, 1, 1), QDate(2020, 12, 31));
    parseRegionDate(region, QDate(2020, 1, 1));
    parseRegionCalendarYear(region, 2020);
    parseRegionCalendarYear(region, 2021);
    parseRegionCalendarYear(region, 2022);
    parseRegionCalendarYear(region, 2023);
    parseRegionCalendarYear(region, 2024);
    parseRegionCalendarYear(region, 2025);
}

void HolidayRegionTest::testIran()
{
    KHolidays::HolidayRegion region(QStringLiteral("ir_en-us"));
    printMetadata(region);
    parseRegionCalendarYear(region, 2020);
    parseRegionCalendarYear(region, 2021);
    parseRegionCalendarYear(region, 2022);
    parseRegionCalendarYear(region, 2023);
    parseRegionCalendarYear(region, 2024);
    parseRegionCalendarYear(region, 2025);
}

void HolidayRegionTest::testIsrael()
{
    KHolidays::HolidayRegion region(QStringLiteral("il_en-us"));
    printMetadata(region);
    parseRegionCalendarYear(region, 2020);
    parseRegionCalendarYear(region, 2021);
    parseRegionCalendarYear(region, 2022);
    parseRegionCalendarYear(region, 2023);
    parseRegionCalendarYear(region, 2024);
    parseRegionCalendarYear(region, 2025);
}

void HolidayRegionTest::testRegions()
{
    qDebug() << "Available region codes:";
    const QStringList regionCodes = KHolidays::HolidayRegion::regionCodes();
    for (const QString &regionCode : regionCodes) {
        KHolidays::HolidayRegion testRegion(regionCode);
        qDebug() << regionCode << " = " << testRegion.name();
    }
    qDebug() << "";

    qDebug() << "This years holidays:";
    for (const QString &regionCode : regionCodes) {
        KHolidays::HolidayRegion testRegion(regionCode);
        printMetadata(testRegion);
        parseRegionCalendarYear(testRegion, QDate::currentDate().year());
        qDebug() << "";
    }
    qDebug() << "";
}

void HolidayRegionTest::testDefaultRegions()
{
    QCOMPARE(KHolidays::HolidayRegion::defaultRegionCode("be", "nl"), QString("be_nl"));
    QCOMPARE(KHolidays::HolidayRegion::defaultRegionCode("be", "fr"), QString("be_fr"));
    QCOMPARE(KHolidays::HolidayRegion::defaultRegionCode("be", "xx"), QString("be_fr"));
    QCOMPARE(KHolidays::HolidayRegion::defaultRegionCode("xx", "fr_BE"), QString("be_fr"));
    QCOMPARE(KHolidays::HolidayRegion::defaultRegionCode("xx", "xx"), QString());
    QCOMPARE(KHolidays::HolidayRegion::defaultRegionCode("au", "en_GB"), QString("au-act_en-gb"));
    QCOMPARE(KHolidays::HolidayRegion::defaultRegionCode("au-VIC", "en_GB"), QString("au-vic_en-gb"));
    QCOMPARE(KHolidays::HolidayRegion::defaultRegionCode("DE-BE"), QLatin1String("de-be_de"));
    QCOMPARE(KHolidays::HolidayRegion::defaultRegionCode("DE"), QLatin1String("de_de"));
    QCOMPARE(KHolidays::HolidayRegion::defaultRegionCode("AT-9"), QLatin1String("at_de"));
    QCOMPARE(KHolidays::HolidayRegion::defaultRegionCode("US-CA"), QLatin1String("us_en-us"));
}

void HolidayRegionTest::testSolistaleInHolidays()
{
    KHolidays::HolidayRegion region(QFileInfo(KDESRCDIR "/holiday_solstice_in_holidays"));

    auto holidays = region.rawHolidaysWithAstroSeasons(QDate(2020, 7, 1), QDate(2021, 6, 30));
    QCOMPARE(holidays.size(), 5);
    QCOMPARE(holidays.first().observedStartDate(), QDate(2020, 9, 22));
    QCOMPARE(holidays.first().name(), QLatin1String("September Equinox"));

    holidays = region.rawHolidays(QDate(2020, 7, 1), QDate(2021, 6, 30));
    QCOMPARE(holidays.size(), 1);
    QCOMPARE(holidays.first().observedStartDate(), QDate(2021, 1, 1));
    QCOMPARE(holidays.first().name(), QLatin1String("New Years"));

    holidays = region.rawHolidays(QDate(2020, 7, 1), QDate(2021, 6, 30), QLatin1String("public"));
    QCOMPARE(holidays.size(), 1);
    QCOMPARE(holidays.first().observedStartDate(), QDate(2021, 1, 1));
    QCOMPARE(holidays.first().name(), QLatin1String("New Years"));

    holidays = region.rawHolidays(QDate(2020, 7, 1), QDate(2021, 6, 30), QLatin1String("seasonal"));
    QCOMPARE(holidays.size(), 4);
    QCOMPARE(holidays.first().observedStartDate(), QDate(2020, 9, 22));
    QCOMPARE(holidays.first().name(), QLatin1String("September Equinox"));

    holidays = region.rawHolidaysWithAstroSeasons(2020);
    QCOMPARE(holidays.size(), 5);
    QCOMPARE(holidays.first().observedStartDate(), QDate(2020, 1, 1));
    QCOMPARE(holidays.first().name(), QLatin1String("New Years"));
    QCOMPARE(holidays.last().observedStartDate(), QDate(2020, 12, 21));
    QCOMPARE(holidays.last().name(), QLatin1String("December Solstice"));

    holidays = region.rawHolidaysWithAstroSeasons(QDate(2021, 3, 20));
    QCOMPARE(holidays.size(), 1);
    QCOMPARE(holidays.first().observedStartDate(), QDate(2021, 3, 20));
    QCOMPARE(holidays.first().name(), QLatin1String("March Equinox"));
}

void HolidayRegionTest::testLoadFileCalendarSystemsForPlasma()
{
    KHolidays::HolidayRegion region(QFileInfo(KDESRCDIR "/holiday_calendar_systems"));
    printMetadata(region);
    parseRawRegionDateRange(region, QDate(2020, 7, 1), QDate(2021, 6, 30));
    parseRawRegionDateRange(region, QDate(2020, 1, 1), QDate(2020, 12, 31));
    parseRawRegionDateRange(region, QDate(2020, 11, 1), QDate(2021, 1, 2));
    parseRawRegionDateRange(region, QDate(2020, 12, 25), QDate(2021, 7, 2));
    parseRawRegionDateRange(region, QDate(2020, 12, 25), QDate(2022, 1, 2));
}

void HolidayRegionTest::testDominicanRepublicDiadelaConstitucion()
{
    KHolidays::HolidayRegion region(QStringLiteral("do_es"));
    printMetadata(region);
    auto holidays = region.rawHolidays(QDate(2020, 10, 20), QDate(2020, 12, 15));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2020, 11, 9));
    holidays = region.rawHolidays(QDate(2021, 10, 20), QDate(2021, 12, 15), QLatin1String("public"));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2021, 11, 6));
    holidays = region.rawHolidays(QDate(2022, 10, 20), QDate(2022, 12, 15), QLatin1String("public"));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2022, 11, 6));
    holidays = region.rawHolidays(QDate(2023, 10, 20), QDate(2023, 12, 15), QLatin1String("public"));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2023, 11, 6));
    holidays = region.rawHolidays(QDate(2024, 10, 20), QDate(2024, 12, 15), QLatin1String("public"));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2024, 11, 4));
    holidays = region.rawHolidays(QDate(2025, 10, 20), QDate(2025, 12, 15), QLatin1String("public"));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2025, 11, 10));
    holidays = region.rawHolidays(QDate(2026, 10, 20), QDate(2026, 12, 15), QLatin1String("public"));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2026, 11, 9));
    holidays = region.rawHolidays(QDate(2027, 10, 20), QDate(2027, 12, 15), QLatin1String("public"));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2027, 11, 6));
    holidays = region.rawHolidays(QDate(2028, 10, 20), QDate(2028, 12, 15), QLatin1String("public"));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2028, 11, 6));
    holidays = region.rawHolidays(QDate(2029, 10, 20), QDate(2029, 12, 15), QLatin1String("public"));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2029, 11, 5));
    holidays = region.rawHolidays(QDate(2030, 10, 20), QDate(2030, 12, 15), QLatin1String("public"));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2030, 11, 4));
    holidays = region.rawHolidays(QDate(2031, 10, 20), QDate(2031, 12, 15), QLatin1String("public"));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2031, 11, 10));
    holidays = region.rawHolidays(QDate(2035, 10, 20), QDate(2035, 12, 15), QLatin1String("public"));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2035, 11, 5));
}

void HolidayRegionTest::testUsAmericanJuneteenth()
{
    KHolidays::HolidayRegion region(QStringLiteral("us_en-us"));
    printMetadata(region);
    auto holidays = region.rawHolidays(QDate(2023, 6, 19), QDate(2023, 6, 19));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2023, 6, 19));
    holidays = region.rawHolidays(QDate(2024, 6, 1), QDate(2024, 6, 25), QLatin1String("public"));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2024, 6, 19));
    holidays = region.rawHolidays(QDate(2027, 6, 1), QDate(2027, 6, 25), QLatin1String("public"));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2027, 6, 18));
    holidays = region.rawHolidays(QDate(2033, 6, 20), QDate(2033, 6, 25), QLatin1String("public"));
    QCOMPARE(holidays.first().observedStartDate(), QDate(2033, 6, 20));
}

#include "moc_testholidayregion.cpp"
