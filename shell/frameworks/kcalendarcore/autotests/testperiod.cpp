/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2006 Narayan Newton <narayannewton@gmail.com>
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testperiod.h"
#include "period.h"
#include "utils_p.h"

#include <QTest>
#include <QTimeZone>

QTEST_MAIN(PeriodTest)

using namespace KCalendarCore;

void PeriodTest::testValidity()
{
    const QDateTime p1DateTime(QDate(2006, 8, 30), QTime(7, 0, 0), QTimeZone::UTC);
    Period p1(p1DateTime, Duration(60));
    Period p2;

    QVERIFY(p1.hasDuration());
    QCOMPARE(p1.duration().asSeconds(), 60);
    QVERIFY(p1.start() == QDateTime(QDate(2006, 8, 30), QTime(7, 0, 0), QTimeZone::UTC));

    p2 = p1;

    QVERIFY(p2.hasDuration());
    QVERIFY(p2.duration().asSeconds() == 60);
    QVERIFY(p2.start() == QDateTime(QDate(2006, 8, 30), QTime(7, 0, 0), QTimeZone::UTC));

    const QDateTime p3DateTime(QDate(2006, 8, 30), QTime(7, 0, 0), QTimeZone::UTC);
    Period p3(p3DateTime, Duration(24 * 60 * 60));

    QVERIFY(p3.hasDuration());
    QVERIFY(p3.duration().asSeconds() == 24 * 60 * 60);
    QVERIFY(p3.start() == QDateTime(QDate(2006, 8, 30), QTime(7, 0, 0), QTimeZone::UTC));
}

void PeriodTest::testCompare()
{
    const QDateTime p1DateTime(QDate(2006, 8, 30), {});
    Period p1(p1DateTime, Duration(24 * 60 * 60));
    const QDateTime p2DateTime(QDate(2006, 8, 29), {});
    Period p2(p2DateTime, Duration(23 * 60 * 60));
    const QDateTime p3DateTime(QDate(2006, 8, 30), QTime(7, 0, 0), QTimeZone::UTC);
    Period p3(p3DateTime, Duration(24 * 60 * 60));
    Period p1copy {p1}; // test copy constructor
    Period p1assign;
    p1assign = p1; // test operator=
    Period p3copy {p3};
    Period p3assign = p3;

    QVERIFY(p2 < p1);
    QVERIFY(!(p1 == p2));
    QVERIFY(p1copy == p1);
    QVERIFY(p1assign == p1);
    QVERIFY(p3copy == p3);
    QVERIFY(p3assign == p3);
}

void PeriodTest::testDataStreamOut()
{
    const QDateTime p1DateTime(QDate(2006, 8, 30), QTime(7, 0, 0), QTimeZone::UTC);
    const Duration duration(24 * 60 * 60);
    Period p1(p1DateTime, duration);

    QByteArray byteArray;
    QDataStream out_stream(&byteArray, QIODevice::WriteOnly);

    out_stream << p1;

    QDataStream in_stream(&byteArray, QIODevice::ReadOnly);

    QDateTime begin;
    deserializeKDateTimeAsQDateTime(in_stream, begin);
    // There is no way to serialize KDateTime as of KDE4.5
    // and the to/fromString methods do not perform a perfect reconstruction
    // of a datetime
    QVERIFY(begin == p1.start());

    QDateTime end;
    deserializeKDateTimeAsQDateTime(in_stream, end);
    QVERIFY(end == p1.end());

    bool dailyduration;
    in_stream >> dailyduration;
    QVERIFY(dailyduration == duration.isDaily());

    bool hasduration;
    in_stream >> hasduration;
    QVERIFY(hasduration == p1.hasDuration());
}

void PeriodTest::testDataStreamIn()
{
    const QDateTime p1DateTime(QDate(2006, 8, 30), QTime(7, 0, 0), QTimeZone::UTC);
    const Duration duration(24 * 60 * 60);
    Period p1(p1DateTime, duration);

    QByteArray byteArray;
    QDataStream out_stream(&byteArray, QIODevice::WriteOnly);

    out_stream << p1;

    QDataStream in_stream(&byteArray, QIODevice::ReadOnly);

    Period p2;

    in_stream >> p2;

    QVERIFY(p1 == p2);
}

#include "moc_testperiod.cpp"
