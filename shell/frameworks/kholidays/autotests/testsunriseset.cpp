/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2012 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testsunriseset.h"
#include "sunriseset.h"
using namespace KHolidays;
using namespace SunRiseSet;

#include <QTest>

QTEST_MAIN(SunriseTest)

void SunriseTest::TestSunrise()
{
    // test bogus latitudes and longitudes
    QCOMPARE(utcSunrise(QDate(2012, 1, 1), 1000.0, 1000.0), QTime());
    QCOMPARE(utcSunrise(QDate(2012, 7, 1), -1000.0, -1000.0), QTime());
    QCOMPARE(utcSunrise(QDate(2012, 12, 31), 1000.0, -1000.0), QTime());

    // NYC
    QCOMPARE(utcSunrise(QDate(2012, 1, 1), 40.72, -74.02), QTime(12, 20));
    QCOMPARE(utcSunrise(QDate(2012, 7, 1), 40.72, -74.02), QTime(9, 29));
    QCOMPARE(utcSunrise(QDate(2012, 12, 31), 40.72, -74.02), QTime(12, 20));

    // LA
    QCOMPARE(utcSunrise(QDate(2012, 1, 1), 34.05, -118.23), QTime(14, 58));
    QCOMPARE(utcSunrise(QDate(2012, 7, 1), 34.05, -118.23), QTime(12, 45));
    QCOMPARE(utcSunrise(QDate(2012, 12, 31), 34.05, -118.23), QTime(14, 58));

    // Berlin
    QCOMPARE(utcSunrise(QDate(2012, 1, 1), 51.40, 7.38), QTime(7, 36));
    QCOMPARE(utcSunrise(QDate(2012, 7, 1), 51.40, 7.38), QTime(3, 19));
    QCOMPARE(utcSunrise(QDate(2012, 12, 31), 51.40, 7.38), QTime(7, 36));

    QCOMPARE(utcSunrise(QDate(2020, 12, 3), 52.5, 13.0), QTime(7, 0));

    // Tasmania
    QCOMPARE(utcSunrise(QDate(2012, 1, 1), -14.60, 133.77), QTime(20, 39));
    QCOMPARE(utcSunrise(QDate(2012, 7, 1), -14.60, 133.77), QTime(21, 31));
    QCOMPARE(utcSunrise(QDate(2012, 12, 31), -14.60, 133.77), QTime(20, 39));

    // Spitsbergen (affected by polar day/night)
    QCOMPARE(utcSunrise(QDate(2022, 6, 21), 78.2231, 15.6322), QTime());
    QCOMPARE(utcSunrise(QDate(2022, 12, 21), 78.2231, 15.6322), QTime());
    QCOMPARE(utcSunrise(QDate(2022, 3, 21), 78.2231, 15.6322), QTime(4, 42));
    QCOMPARE(utcSunrise(QDate(2022, 9, 21), 78.2231, 15.6322), QTime(4, 23));
}

void SunriseTest::TestSunset()
{
    // NYC
    QCOMPARE(utcSunset(QDate(2012, 1, 1), 40.72, -74.02), QTime(21, 39));
    QCOMPARE(utcSunset(QDate(2012, 7, 1), 40.72, -74.02), QTime(00, 31));
    QCOMPARE(utcSunset(QDate(2012, 12, 31), 40.72, -74.02), QTime(21, 38));

    // LA
    QCOMPARE(utcSunset(QDate(2012, 1, 1), 34.05, -118.23), QTime(00, 54));
    QCOMPARE(utcSunset(QDate(2012, 7, 1), 34.05, -118.23), QTime(3, 8));
    QCOMPARE(utcSunset(QDate(2012, 12, 31), 34.05, -118.23), QTime(00, 54));

    // Berlin
    QCOMPARE(utcSunset(QDate(2012, 1, 1), 51.40, 7.38), QTime(15, 32));
    QCOMPARE(utcSunset(QDate(2012, 7, 1), 51.40, 7.38), QTime(19, 50));
    QCOMPARE(utcSunset(QDate(2012, 12, 31), 51.40, 7.38), QTime(15, 32));

    // Tasmania
    QCOMPARE(utcSunset(QDate(2012, 1, 1), -14.60, 133.77), QTime(9, 37));
    QCOMPARE(utcSunset(QDate(2012, 7, 1), -14.60, 133.77), QTime(8, 47));
    QCOMPARE(utcSunset(QDate(2012, 12, 31), -14.60, 133.77), QTime(9, 37));

    // Spitsbergen (affected by polar day/night)
    QCOMPARE(utcSunset(QDate(2022, 6, 21), 78.2231, 15.6322), QTime());
    QCOMPARE(utcSunset(QDate(2022, 12, 21), 78.2231, 15.6322), QTime());
    QCOMPARE(utcSunset(QDate(2022, 3, 21), 78.2231, 15.6322), QTime(17, 27));
    QCOMPARE(utcSunset(QDate(2022, 9, 21), 78.2231, 15.6322), QTime(17, 18));
}

void SunriseTest::TestDawn()
{
    // NYC
    QCOMPARE(utcDawn(QDate(2012, 1, 1), 40.72, -74.02), QTime(11, 49));
    QCOMPARE(utcDawn(QDate(2012, 7, 1), 40.72, -74.02), QTime(8, 56));
    QCOMPARE(utcDawn(QDate(2012, 12, 31), 40.72, -74.02), QTime(11, 49));

    // LA
    QCOMPARE(utcDawn(QDate(2012, 1, 1), 34.05, -118.23), QTime(14, 31));
    QCOMPARE(utcDawn(QDate(2012, 7, 1), 34.05, -118.23), QTime(12, 16));
    QCOMPARE(utcDawn(QDate(2012, 12, 31), 34.05, -118.23), QTime(14, 31));

    // Berlin
    QCOMPARE(utcDawn(QDate(2012, 1, 1), 51.40, 7.38), QTime(6, 56));
    QCOMPARE(utcDawn(QDate(2012, 7, 1), 51.40, 7.38), QTime(2, 32));
    QCOMPARE(utcDawn(QDate(2012, 12, 31), 51.40, 7.38), QTime(6, 56));

    // Tasmania
    QCOMPARE(utcDawn(QDate(2012, 1, 1), -14.60, 133.77), QTime(20, 16));
    QCOMPARE(utcDawn(QDate(2012, 7, 1), -14.60, 133.77), QTime(21, 7));
    QCOMPARE(utcDawn(QDate(2012, 12, 31), -14.60, 133.77), QTime(20, 15));
}

void SunriseTest::TestDusk()
{
    QCOMPARE(utcDusk(QDate(2012, 1, 1), 40.72, -74.02), QTime(22, 10));
    QCOMPARE(utcDusk(QDate(2012, 7, 1), 40.72, -74.02), QTime(1, 4));
    QCOMPARE(utcDusk(QDate(2012, 12, 31), 40.72, -74.02), QTime(22, 9));

    // LA
    QCOMPARE(utcDusk(QDate(2012, 1, 1), 34.05, -118.23), QTime(1, 22));
    QCOMPARE(utcDusk(QDate(2012, 7, 1), 34.05, -118.23), QTime(3, 37));
    QCOMPARE(utcDusk(QDate(2012, 12, 31), 34.05, -118.23), QTime(1, 22));

    // Berlin
    QCOMPARE(utcDusk(QDate(2012, 1, 1), 51.40, 7.38), QTime(16, 12));
    QCOMPARE(utcDusk(QDate(2012, 7, 1), 51.40, 7.38), QTime(20, 37));
    QCOMPARE(utcDusk(QDate(2012, 12, 31), 51.40, 7.38), QTime(16, 12));

    // Tasmania
    QCOMPARE(utcDusk(QDate(2012, 1, 1), -14.60, 133.77), QTime(10, 1));
    QCOMPARE(utcDusk(QDate(2012, 7, 1), -14.60, 133.77), QTime(9, 10));
    QCOMPARE(utcDusk(QDate(2012, 12, 31), -14.60, 133.77), QTime(10, 1));
}

static bool verifyPolarDayRange(QDate begin, QDate end, double latitude, bool expected)
{
    for (QDate date = begin; date <= end; date = date.addDays(1)) {
        if (isPolarDay(date, latitude) != expected) {
            qDebug() << date << latitude << isPolarDay(date, latitude) << expected;
            return false;
        }
        if (expected && utcSunrise(date, latitude, 0.0).isValid() && utcSunset(date, latitude, 0.0).isValid()) {
            qDebug() << date << latitude << utcSunrise(date, latitude, 0.0) << utcSunset(date, latitude, 0.0);
            return false;
        }
    }
    return true;
}

static bool verifyPolarTwilightRange(QDate begin, QDate end, double latitude, bool expected)
{
    for (QDate date = begin; date <= end; date = date.addDays(1)) {
        if (isPolarTwilight(date, latitude) != expected) {
            qDebug() << date << latitude << isPolarTwilight(date, latitude) << expected;
            return false;
        }
    }
    return true;
}

static bool verifyPolarNightRange(QDate begin, QDate end, double latitude, bool expected)
{
    for (QDate date = begin; date <= end; date = date.addDays(1)) {
        if (isPolarNight(date, latitude) != expected) {
            qDebug() << date << latitude << isPolarNight(date, latitude) << expected;
            return false;
        }
        if (expected && utcSunrise(date, latitude, 0.0).isValid() && utcSunset(date, latitude, 0.0).isValid()) {
            qDebug() << date << latitude << utcSunrise(date, latitude, 0.0) << utcSunset(date, latitude, 0.0);
            return false;
        }
    }
    return true;
}

void SunriseTest::TestPolarDayNight()
{
    // polar circle (north): polar day from 6th June to 6th July, no polar night
    QVERIFY(verifyPolarDayRange(QDate(2022, 1, 1), QDate(2022, 6, 5), 66.5, false));
    QVERIFY(verifyPolarDayRange(QDate(2022, 6, 6), QDate(2022, 7, 5), 66.5, true));
    QVERIFY(verifyPolarDayRange(QDate(2022, 7, 6), QDate(2022, 12, 31), 66.5, false));
    QVERIFY(verifyPolarNightRange(QDate(2022, 1, 1), QDate(2022, 12, 31), 66.5, false));

    // polar circle (south): same in reverse
    QVERIFY(verifyPolarDayRange(QDate(2021, 1, 6), QDate(2021, 12, 7), -66.5, false));
    QVERIFY(verifyPolarDayRange(QDate(2021, 12, 8), QDate(2022, 1, 4), -66.5, true));
    QVERIFY(verifyPolarDayRange(QDate(2022, 1, 5), QDate(2022, 12, 7), -66.5, false));
    QVERIFY(verifyPolarNightRange(QDate(2022, 1, 1), QDate(2022, 12, 31), -66.5, false));

    // 68°: polar twilight from Dec 9 to Jan 2, no polar night
    QVERIFY(verifyPolarTwilightRange(QDate(2021, 1, 3), QDate(2021, 12, 8), 68.0, false));
    QVERIFY(verifyPolarTwilightRange(QDate(2021, 12, 9), QDate(2022, 1, 2), 68.0, true));
    QVERIFY(verifyPolarTwilightRange(QDate(2022, 1, 3), QDate(2022, 12, 8), 68.0, false));
    QVERIFY(verifyPolarNightRange(QDate(2022, 1, 1), QDate(2022, 12, 31), 68.0, false));

    // -68°: polar twilight from June 8 to July 3, no polar night
    QVERIFY(verifyPolarTwilightRange(QDate(2021, 1, 1), QDate(2021, 6, 7), -68.0, false));
    QVERIFY(verifyPolarTwilightRange(QDate(2021, 6, 8), QDate(2021, 7, 3), -68.0, true));
    QVERIFY(verifyPolarTwilightRange(QDate(2021, 7, 4), QDate(2021, 12, 31), -68.0, false));
    QVERIFY(verifyPolarNightRange(QDate(2022, 1, 1), QDate(2022, 12, 31), -68.0, false));

    // 78.5°: polar night from Nov 11 to Jan 30
    QVERIFY(verifyPolarNightRange(QDate(2021, 1, 31), QDate(2021, 11, 10), 78.5, false));
    QVERIFY(verifyPolarNightRange(QDate(2021, 11, 11), QDate(2022, 1, 30), 78.5, true));
    QVERIFY(verifyPolarNightRange(QDate(2022, 1, 31), QDate(2022, 11, 10), 78.5, false));

    // -78.5°: polar night from May 10 to Aug 2
    QVERIFY(verifyPolarNightRange(QDate(2022, 1, 1), QDate(2022, 5, 9), -78.5, false));
    QVERIFY(verifyPolarNightRange(QDate(2022, 5, 10), QDate(2022, 8, 2), -78.5, true));
    QVERIFY(verifyPolarNightRange(QDate(2022, 8, 3), QDate(2022, 12, 31), -78.5, false));
}

#include "moc_testsunriseset.cpp"
