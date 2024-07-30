/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testfreebusyperiod.h"
#include "freebusyperiod.h"

#include <QTest>
#include <QTimeZone>
QTEST_MAIN(FreeBusyPeriodTest)

using namespace KCalendarCore;

void FreeBusyPeriodTest::testValidity()
{
    const QDateTime p1DateTime(QDate(2006, 8, 30), QTime(7, 0, 0), QTimeZone::UTC);
    FreeBusyPeriod p1(p1DateTime, Duration(60));

    QString summary = QStringLiteral("I can haz summary?");
    QString location = QStringLiteral("The Moon");
    p1.setSummary(summary);
    p1.setLocation(location);

    QVERIFY(p1.hasDuration());
    QCOMPARE(p1.duration().asSeconds(), 60);
    QVERIFY(p1.start() == QDateTime(QDate(2006, 8, 30), QTime(7, 0, 0), QTimeZone::UTC));

    QCOMPARE(p1.summary(), summary);
    QCOMPARE(p1.location(), location);
}

void FreeBusyPeriodTest::testAssign()
{
    const QDateTime p1DateTime(QDate(2006, 8, 30), QTime(7, 0, 0), QTimeZone::UTC);
    FreeBusyPeriod p1(p1DateTime, Duration(60));
    FreeBusyPeriod p2;

    QString summary = QStringLiteral("I can haz summary?");
    QString location = QStringLiteral("The Moon");
    p1.setSummary(summary);
    p1.setLocation(location);

    p2 = p1;

    QVERIFY(p2.hasDuration());
    QVERIFY(p2.duration().asSeconds() == 60);
    QVERIFY(p2.start() == p1DateTime);
    QCOMPARE(p1.summary(), summary);
    QCOMPARE(p1.location(), location);
}

void FreeBusyPeriodTest::testCopyConstructor()
{
    const QDateTime p1DateTime(QDate(2006, 8, 30), QTime(7, 0, 0), QTimeZone::UTC);
    FreeBusyPeriod p1(p1DateTime, Duration(60));
    QString summary = QStringLiteral("I can haz summary?");
    QString location = QStringLiteral("The Moon");
    p1.setSummary(summary);
    p1.setLocation(location);

    FreeBusyPeriod p2 {p1};

    QVERIFY(p2.hasDuration());
    QVERIFY(p2.duration().asSeconds() == 60);
    QVERIFY(p2.start() == p1DateTime);
    QCOMPARE(p1.summary(), summary);
    QCOMPARE(p1.location(), location);
}


void FreeBusyPeriodTest::testDataStreamOut()
{
    const QDateTime p1DateTime(QDate(2006, 8, 30), QTime(7, 0, 0), QTimeZone::UTC);
    FreeBusyPeriod p1(p1DateTime, Duration(60));

    p1.setSummary(QStringLiteral("I can haz summary?"));
    p1.setLocation(QStringLiteral("The Moon"));

    QByteArray byteArray;
    QDataStream out_stream(&byteArray, QIODevice::WriteOnly);

    out_stream << p1;

    QDataStream in_stream(&byteArray, QIODevice::ReadOnly);

    Period p2;
    Period periodParent = static_cast<Period>(p1);
    in_stream >> p2;
    QVERIFY(periodParent == p2);

    QString summary;
    in_stream >> summary;
    QCOMPARE(summary, p1.summary());

    QString location;
    in_stream >> location;
    QCOMPARE(location, p1.location());
}

void FreeBusyPeriodTest::testDataStreamIn()
{
    const QDateTime p1DateTime = QDate(2006, 8, 30).startOfDay();
    const Duration duration(24 * 60 * 60);
    FreeBusyPeriod p1(p1DateTime, duration);
    p1.setSummary(QStringLiteral("I can haz summary?"));
    p1.setLocation(QStringLiteral("The Moon"));

    QByteArray byteArray;
    QDataStream out_stream(&byteArray, QIODevice::WriteOnly);

    out_stream << p1;

    QDataStream in_stream(&byteArray, QIODevice::ReadOnly);

    FreeBusyPeriod p2;
    in_stream >> p2;

    QCOMPARE(p2, p1);
}

#include "moc_testfreebusyperiod.cpp"
