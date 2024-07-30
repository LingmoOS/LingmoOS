/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2006 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testduration.h"
#include "duration.h"

#include <QDateTime>
#include <QTimeZone>

#include <QTest>
QTEST_MAIN(DurationTest)

using namespace KCalendarCore;

void DurationTest::testValidity()
{
    const QDateTime firstDateTime(QDate(2006, 8, 3), QTime(7, 0, 0), QTimeZone::UTC);

    Duration d(firstDateTime, QDateTime(QDate(2006, 8, 3), QTime(8, 0, 0), QTimeZone::UTC));

    QCOMPARE(d.asSeconds(), 1 * 60 * 60);
}

void DurationTest::testCompare()
{
    const QDateTime firstDateTime(QDate(2006, 8, 3), QTime(7, 0, 0), QTimeZone::UTC);

    Duration d1(firstDateTime, QDateTime(QDate(2006, 8, 3), QTime(8, 0, 0), QTimeZone::UTC));
    // d1 has 1hr duration

    Duration d2(2 * 60 * 60); // 2hr duration

    Duration d1copy {d1}; // test copy constructor
    Duration d1assign;
    d1assign = d1; // test operator=

    QVERIFY(d1 < d2);
    QVERIFY(d1 != d2);
    QVERIFY(d1copy == d1);
    QVERIFY(d1assign == d1);

    Duration d3(7, Duration::Days);
    Duration d4(7 * 24 * 60 * 60, Duration::Seconds);
    QVERIFY(d3 != d4); // cannot compare days durations with seconds durations

    QVERIFY(d3 > d2);
    QVERIFY(-d3 < d2);

    Duration d5 = d1;
    d5 += d2; // should be 3hrs
    QVERIFY(d5 > d2);
    QVERIFY(d2 < d5);
    Duration d6(3 * 60 * 60);
    QVERIFY(d6 == d5);
    QVERIFY((d6 -= (2 * 60 * 60)) == d1);
}

void DurationTest::testSerializer_data()
{
    QTest::addColumn<KCalendarCore::Duration>("duration");

    Duration duration1;
    Duration duration2(7, Duration::Days);
    Duration duration3(7 * 24 * 60 * 60, Duration::Seconds);

    const QDateTime firstDateTime(QDate(2006, 8, 3), QTime(7, 0, 0), QTimeZone::UTC);
    Duration duration4(firstDateTime, QDateTime(QDate(2006, 8, 3), QTime(8, 0, 0), QTimeZone::UTC));

    QTest::newRow("duration1") << duration1;
    QTest::newRow("duration2") << duration2;
    QTest::newRow("duration3") << duration3;
    QTest::newRow("duration4") << duration4;
}

void DurationTest::testSerializer()
{
    QFETCH(KCalendarCore::Duration, duration);

    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << duration; // Serialize

    Duration duration2;
    QDataStream stream2(&array, QIODevice::ReadOnly);
    stream2 >> duration2; // deserialize
    QVERIFY(duration == duration2);
}

void DurationTest::testIsNull()
{
    Duration d;
    QVERIFY(d.isNull());

    Duration d2(7, Duration::Days);
    QVERIFY(!d2.isNull());
}

#include "moc_testduration.cpp"
