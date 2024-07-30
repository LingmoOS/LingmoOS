/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-FileContributor: Sergio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "testtimesininterval.h"
#include "event.h"

#include <QDebug>

#include <QTest>
QTEST_MAIN(TimesInIntervalTest)

using namespace KCalendarCore;

void TimesInIntervalTest::test()
{
    const QDateTime currentDate(QDate::currentDate(), {});
    Event event;
    event.setDtStart(currentDate);
    event.setDtEnd(currentDate.addDays(1));
    event.setAllDay(true);
    event.setSummary(QStringLiteral("Event1 Summary"));

    event.recurrence()->setDaily(1);

    //------------------------------------------------------------------------------------------------
    // Just to warm up
    QVERIFY(event.recurs());
    QVERIFY(event.recursAt(currentDate));

    //------------------------------------------------------------------------------------------------
    // Daily recurrence that never stops.
    // Should return numDaysInInterval+1 occurrences
    const int numDaysInInterval = 7;
    QDateTime start(currentDate);
    QDateTime end(start.addDays(numDaysInInterval));

    start.setTime(QTime(0, 0, 0));
    end.setTime(QTime(23, 59, 59));
    auto dateList = event.recurrence()->timesInInterval(start, end);
    QVERIFY(dateList.count() == numDaysInInterval + 1);

    //------------------------------------------------------------------------------------------------
    // start == end == first day of the recurrence, should only return 1 occurrence
    end = start;
    end.setTime(QTime(23, 59, 59));
    dateList = event.recurrence()->timesInInterval(start, end);
    QVERIFY(dateList.count() == 1);

    //------------------------------------------------------------------------------------------------
    // Test daily recurrence that only lasts X days
    const int recurrenceDuration = 3;
    event.recurrence()->setDuration(recurrenceDuration);
    end = start.addDays(100);
    dateList = event.recurrence()->timesInInterval(start, end);
    QVERIFY(dateList.count() == recurrenceDuration);
    //------------------------------------------------------------------------------------------------
    // Test daily recurrence that only lasts X days, and give start == end == last day of
    // recurrence. Previous versions of kcal had a bug and didn't return an occurrence
    start = start.addDays(recurrenceDuration - 1);
    end = start;
    start.setTime(QTime(0, 0, 0));
    end.setTime(QTime(23, 59, 59));

    dateList = event.recurrence()->timesInInterval(start, end);
    QVERIFY(dateList.count() == 1);

    //------------------------------------------------------------------------------------------------
}

// Test that interval start and end are inclusive
void TimesInIntervalTest::testSubDailyRecurrenceIntervalInclusive()
{
    const QDateTime start(QDate(2013, 03, 10), QTime(10, 0, 0), QTimeZone::UTC);
    const QDateTime end(QDate(2013, 03, 10), QTime(11, 0, 0), QTimeZone::UTC);

    KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    event->setUid(QStringLiteral("event"));
    event->setDtStart(start);
    event->recurrence()->setHourly(1);
    event->recurrence()->setDuration(2);

    QList<QDateTime> expectedEventOccurrences;
    expectedEventOccurrences << start << start.addSecs(60 * 60);

    const auto timesInInterval = event->recurrence()->timesInInterval(start, end);
    for (const auto &dt : timesInInterval) {
        QCOMPARE(expectedEventOccurrences.removeAll(dt), 1);
    }
    QCOMPARE(expectedEventOccurrences.size(), 0);
}

// Test that the recurrence dtStart is used for calculation and not the interval start date
void TimesInIntervalTest::testSubDailyRecurrence2()
{
    const QDateTime start(QDate(2013, 03, 10), QTime(10, 2, 3), QTimeZone::UTC);
    const QDateTime end(QDate(2013, 03, 10), QTime(13, 4, 5), QTimeZone::UTC);

    KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    event->setUid(QStringLiteral("event"));
    event->setDtStart(start);
    event->recurrence()->setHourly(1);
    event->recurrence()->setDuration(2);

    QList<QDateTime> expectedEventOccurrences;
    expectedEventOccurrences << start << start.addSecs(60 * 60);

    const auto timesInInterval = event->recurrence()->timesInInterval(start.addSecs(-20), end.addSecs(20));
    //   qDebug() << "timesInInterval " << timesInInterval;
    for (const auto &dt : timesInInterval) {
        //     qDebug() << dt;
        QCOMPARE(expectedEventOccurrences.removeAll(dt), 1);
    }
    QCOMPARE(expectedEventOccurrences.size(), 0);
}

void TimesInIntervalTest::testSubDailyRecurrenceIntervalLimits()
{
    const QDateTime start(QDate(2013, 03, 10), QTime(10, 2, 3), QTimeZone::UTC);
    const QDateTime end(QDate(2013, 03, 10), QTime(12, 2, 3), QTimeZone::UTC);

    KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    event->setUid(QStringLiteral("event"));
    event->setDtStart(start);
    event->recurrence()->setHourly(1);
    event->recurrence()->setDuration(3);

    QList<QDateTime> expectedEventOccurrences;
    expectedEventOccurrences << start.addSecs(60 * 60);

    const auto timesInInterval = event->recurrence()->timesInInterval(start.addSecs(1), end.addSecs(-1));
    for (const auto &dt : timesInInterval) {
        QCOMPARE(expectedEventOccurrences.removeAll(dt), 1);
    }
    QCOMPARE(expectedEventOccurrences.size(), 0);
}

void TimesInIntervalTest::testLocalTimeHandlingNonAllDay()
{
    // Create an event which occurs every weekday of every week,
    // starting from Friday the 11th of October, from 12 pm until 1 pm, clock time,
    // and lasts for two weeks, with three exception datetimes,
    // (only two of which will apply).
    const QDateTime startDt(QDate(2019, 10, 11), QTime(12, 0), QTimeZone::LocalTime);
    QTimeZone anotherZone("America/Toronto");
    if (anotherZone.offsetFromUtc(startDt) == QTimeZone::systemTimeZone().offsetFromUtc(startDt)) {
        anotherZone = QTimeZone(QByteArray("Pacific/Midway"));
    }
    Event event;
    event.setAllDay(false);
    event.setDtStart(startDt);

    RecurrenceRule *const rule = new RecurrenceRule();
    rule->setRecurrenceType(RecurrenceRule::rDaily);
    rule->setStartDt(event.dtStart());
    rule->setFrequency(1);
    rule->setDuration(14);
    rule->setByDays(QList<RecurrenceRule::WDayPos>() << RecurrenceRule::WDayPos(0, 1) // Monday
                                                     << RecurrenceRule::WDayPos(0, 2) // Tuesday
                                                     << RecurrenceRule::WDayPos(0, 3) // Wednesday
                                                     << RecurrenceRule::WDayPos(0, 4) // Thursday
                                                     << RecurrenceRule::WDayPos(0, 5)); // Friday

    Recurrence *recurrence = event.recurrence();
    recurrence->addRRule(rule);
    // 12 o'clock in local time, will apply.
    recurrence->addExDateTime(QDateTime(QDate(2019, 10, 15), QTime(12, 0), QTimeZone::LocalTime));
    // 12 o'clock in another time zone, will not apply.
    recurrence->addExDateTime(QDateTime(QDate(2019, 10, 17), QTime(12, 0), anotherZone));
    // The time in another time zone, corresponding to 12 o'clock in the system time zone, will apply.
    recurrence->addExDateTime(QDateTime(QDate(2019, 10, 24), QTime(12, 00), QTimeZone::systemTimeZone()).toTimeZone(anotherZone));

    // Expand the events and within a wide interval
    const DateTimeList timesInInterval =
        recurrence->timesInInterval(QDateTime(QDate(2019, 10, 05), QTime(0, 0)), QDateTime(QDate(2019, 10, 25), QTime(23, 59)));

    // ensure that the expansion does not include weekend days,
    // nor either of the exception date times.
    const QList<int> expectedDays{11, 14, 16, 17, 18, 21, 22, 23, 25};
    for (int day : expectedDays) {
        QVERIFY(timesInInterval.contains(QDateTime(QDate(2019, 10, day), QTime(12, 0), QTimeZone::LocalTime)));
    }
    QCOMPARE(timesInInterval.size(), expectedDays.size());
}

void TimesInIntervalTest::testLocalTimeHandlingAllDay()
{
    // Create an event which occurs every weekday of every week,
    // starting from Friday the 11th of October, and lasts for two weeks,
    // with four exception datetimes (only three of which will apply).
    const QDateTime startDt(QDate(2019, 10, 11).startOfDay());
    QTimeZone anotherZone("America/Toronto");
    if (anotherZone.offsetFromUtc(startDt) == QTimeZone::systemTimeZone().offsetFromUtc(startDt)) {
        anotherZone = QTimeZone(QByteArray("Pacific/Midway"));
    }
    Event event;
    event.setAllDay(true);
    event.setDtStart(startDt);

    RecurrenceRule *const rule = new RecurrenceRule();
    rule->setRecurrenceType(RecurrenceRule::rDaily);
    rule->setStartDt(event.dtStart());
    rule->setFrequency(1);
    rule->setDuration(14);
    rule->setByDays(QList<RecurrenceRule::WDayPos>() << RecurrenceRule::WDayPos(0, 1) // Monday
                                                     << RecurrenceRule::WDayPos(0, 2) // Tuesday
                                                     << RecurrenceRule::WDayPos(0, 3) // Wednesday
                                                     << RecurrenceRule::WDayPos(0, 4) // Thursday
                                                     << RecurrenceRule::WDayPos(0, 5)); // Friday

    Recurrence *recurrence = event.recurrence();
    recurrence->addRRule(rule);
    // A simple date, will apply.
    recurrence->addExDate(QDate(2019, 10, 14));
    // A date only local time, will apply.
    recurrence->addExDateTime(QDate(2019, 10, 15).startOfDay());
    // A date time starting at 00:00 in another zone, will not apply.
    recurrence->addExDateTime(QDateTime(QDate(2019, 10, 17), QTime(), anotherZone));
    // A date time starting at 00:00 in the system time zone, will apply.
    recurrence->addExDateTime(QDateTime(QDate(2019, 10, 24), QTime(), QTimeZone::systemTimeZone()));

    // Expand the events and within a wide interval
    const DateTimeList timesInInterval =
        recurrence->timesInInterval(QDateTime(QDate(2019, 10, 05), QTime(0, 0)), QDateTime(QDate(2019, 10, 25), QTime(23, 59)));

    // ensure that the expansion does not include weekend days,
    // nor either of the exception date times.
    const QList<int> expectedDays{11, 16, 17, 18, 21, 22, 23, 25};
    for (int day : expectedDays) {
        QVERIFY(timesInInterval.contains(QDate(2019, 10, day).startOfDay()));
    }
    QCOMPARE(timesInInterval.size(), expectedDays.size());
}

// Test that the recurrence dtStart is used for calculation and not the interval start date
void TimesInIntervalTest::testByDayRecurrence()
{
    const int days = 7;
    const QDateTime start(QDate(2020, 11, 6), QTime(2, 0, 0), QTimeZone::UTC);
    const QDateTime intervalEnd = start.addDays(days);
    const QDateTime intervalStart = start.addDays(-days);

    Event::Ptr event(new Event());
    event->setDtStart(start);
    event->setDtEnd(start.addSecs(3600));

    RecurrenceRule *const rule = new RecurrenceRule();
    rule->setRecurrenceType(RecurrenceRule::rWeekly);
    rule->setStartDt(event->dtStart()); // the start day is a Friday
    rule->setFrequency(1);
    rule->setByDays(QList<RecurrenceRule::WDayPos>() << RecurrenceRule::WDayPos(0, 2) // Tuesday
                                                     << RecurrenceRule::WDayPos(0, 3) // Wednesday
                                                     << RecurrenceRule::WDayPos(0, 4) // Thursday
                                                     << RecurrenceRule::WDayPos(0, 5)); // Friday
    event->recurrence()->addRRule(rule);

    QList<QDateTime> expectedEventOccurrences;
    for (int i = 0; i <= days; ++i) {
        const QDateTime dt = start.addDays(i);
        if (dt.date().dayOfWeek() < 6 && dt.date().dayOfWeek() > 1) {
            expectedEventOccurrences << dt;
        }
    }

    QCOMPARE(event->recurrence()->getNextDateTime(intervalStart), start);
    QCOMPARE(event->recurrence()->getNextDateTime(start.addDays(1)), start.addDays(4));

    const QList<QDateTime> timesInInterval = event->recurrence()->timesInInterval(intervalStart, intervalEnd);
    for (const QDateTime &dt : timesInInterval) {
        QCOMPARE(expectedEventOccurrences.removeAll(dt), 1);
    }

    QCOMPARE(expectedEventOccurrences.size(), 0);
}

void TimesInIntervalTest::testRDatePeriod()
{
    const QDateTime start(QDate(2021, 8, 30), QTime(11, 14));
    const QDateTime end(QDate(2021, 8, 30), QTime(11, 42));
    const QDateTime other(QDate(2021, 8, 30), QTime(12, 00));
    const QDateTime later(QDate(2021, 8, 30), QTime(15, 00));
    Recurrence recur;

    QVERIFY(!recur.recurs());
    QVERIFY(!recur.rDateTimePeriod(start).isValid());

    recur.addRDateTimePeriod(Period(start, end));
    QVERIFY(recur.recurs());
    QCOMPARE(recur.rDateTimePeriod(start), Period(start, end));

    const QList<QDateTime> timesInInterval = recur.timesInInterval(start.addDays(-1),
                                                                   start.addDays(+1));
    QCOMPARE(timesInInterval, QList<QDateTime>() << start);

    recur.addRDateTime(other);
    QCOMPARE(recur.rDateTimes(), QList<QDateTime>() << start << other);
    QCOMPARE(recur.rDateTimePeriod(start), Period(start, end));

    recur.setRDateTimes(QList<QDateTime>() << other << later);
    QCOMPARE(recur.rDateTimes(), QList<QDateTime>() << other << later);
    QVERIFY(!recur.rDateTimePeriod(start).isValid());
}

#include "moc_testtimesininterval.cpp"
