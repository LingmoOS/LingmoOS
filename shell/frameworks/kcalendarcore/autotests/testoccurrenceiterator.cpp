/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "testoccurrenceiterator.h"
#include "calfilter.h"
#include "memorycalendar.h"
#include "occurrenceiterator.h"

#include <QDebug>
#include <QTest>
#include <QTimeZone>

QTEST_MAIN(TestOccurrenceIterator)

void TestOccurrenceIterator::testIterationWithExceptions()
{
    KCalendarCore::MemoryCalendar calendar(QTimeZone::utc());

    QDateTime start(QDate(2013, 03, 10), QTime(10, 0, 0), QTimeZone::UTC);
    QDateTime end(QDate(2013, 03, 10), QTime(11, 0, 0), QTimeZone::UTC);

    QDateTime recurrenceId(QDate(2013, 03, 11), QTime(10, 0, 0), QTimeZone::UTC);
    QDateTime exceptionStart(QDate(2013, 03, 11), QTime(12, 0, 0), QTimeZone::UTC);
    QDateTime exceptionEnd(QDate(2013, 03, 11), QTime(13, 0, 0), QTimeZone::UTC);

    QDateTime actualEnd(QDate(2013, 03, 12), QTime(11, 0, 0), QTimeZone::UTC);

    KCalendarCore::Event::Ptr event1(new KCalendarCore::Event());
    event1->setUid(QStringLiteral("event1"));
    event1->setSummary(QStringLiteral("event1"));
    event1->setDtStart(start);
    event1->setDtEnd(end);
    event1->recurrence()->setDaily(1);
    calendar.addEvent(event1);

    KCalendarCore::Event::Ptr exception(new KCalendarCore::Event());
    exception->setUid(event1->uid());
    exception->setSummary(QStringLiteral("exception"));
    exception->setRecurrenceId(recurrenceId);
    exception->setDtStart(exceptionStart);
    exception->setDtEnd(exceptionEnd);
    calendar.addEvent(exception);

    int occurrence = 0;
    KCalendarCore::OccurrenceIterator rIt(calendar, start, actualEnd);
    while (rIt.hasNext()) {
        rIt.next();
        occurrence++;
        if (occurrence == 1) {
            QCOMPARE(rIt.occurrenceStartDate(), start);
            QCOMPARE(rIt.incidence()->summary(), event1->summary());
        }
        if (occurrence == 2) {
            QCOMPARE(rIt.occurrenceStartDate(), exceptionStart);
            QCOMPARE(rIt.incidence()->summary(), exception->summary());
        }
        if (occurrence == 3) {
            QCOMPARE(rIt.occurrenceStartDate(), start.addDays(2));
            QCOMPARE(rIt.incidence()->summary(), event1->summary());
        }
    }
    QCOMPARE(occurrence, 3);
}

void TestOccurrenceIterator::testEventsAndTodos()
{
    KCalendarCore::MemoryCalendar calendar(QTimeZone::utc());

    QDateTime start(QDate(2013, 03, 10), QTime(10, 0, 0), QTimeZone::UTC);
    QDateTime actualEnd(QDate(2013, 03, 13), QTime(11, 0, 0), QTimeZone::UTC);

    KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    event->setUid(QStringLiteral("event"));
    event->setDtStart(start);
    event->recurrence()->setDaily(1);
    event->recurrence()->setDuration(2);
    calendar.addEvent(event);

    KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());
    todo->setUid(QStringLiteral("todo"));
    todo->setDtStart(start);
    todo->recurrence()->setDaily(1);
    todo->recurrence()->setDuration(2);
    calendar.addTodo(todo);

    KCalendarCore::OccurrenceIterator rIt(calendar, start, actualEnd);
    QList<QDateTime> expectedTodoOccurrences;
    expectedTodoOccurrences << start << start.addDays(1);
    QList<QDateTime> expectedEventOccurrences;
    expectedEventOccurrences << start << start.addDays(1);
    while (rIt.hasNext()) {
        rIt.next();
        qDebug() << rIt.occurrenceStartDate().toString();
        if (rIt.incidence()->type() == KCalendarCore::Incidence::TypeTodo) {
            QCOMPARE(expectedTodoOccurrences.removeAll(rIt.occurrenceStartDate()), 1);
        } else {
            QCOMPARE(expectedEventOccurrences.removeAll(rIt.occurrenceStartDate()), 1);
        }
    }
    QCOMPARE(expectedTodoOccurrences.size(), 0);
    QCOMPARE(expectedEventOccurrences.size(), 0);
}

void TestOccurrenceIterator::testFilterCompletedTodos()
{
    KCalendarCore::MemoryCalendar calendar(QTimeZone::utc());
    calendar.filter()->setCriteria(KCalendarCore::CalFilter::HideCompletedTodos);

    QDateTime start(QDate(2013, 03, 10), QTime(10, 0, 0), QTimeZone::UTC);
    QDateTime actualEnd(QDate(2013, 03, 13), QTime(11, 0, 0), QTimeZone::UTC);

    KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());
    todo->setUid(QStringLiteral("todo"));
    todo->setDtDue(start);
    todo->setDtStart(start);
    todo->recurrence()->setDaily(1);
    todo->recurrence()->setDuration(2);
    // Yes, recurring todos are weird... setting this says that all occurrences
    // until this one have been completed, and thus should be skipped.
    // that's what kontact did, so it's what we test now.
    todo->setDtRecurrence(start.addDays(2));
    calendar.addTodo(todo);

    KCalendarCore::OccurrenceIterator rIt(calendar, start, actualEnd);
    QVERIFY(!rIt.hasNext());
}

void TestOccurrenceIterator::testAllDayEvents()
{
    KCalendarCore::MemoryCalendar calendar(QTimeZone::utc());

    QDateTime start(QDate(2013, 03, 10), QTime(), QTimeZone::UTC);
    QDateTime actualEnd(QDate(2013, 03, 13), QTime(11, 0, 0), QTimeZone::UTC);

    KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    event->setUid(QStringLiteral("event"));
    event->setDtStart(start);
    event->setAllDay(true);
    event->recurrence()->setDaily(1);
    event->recurrence()->setDuration(2);
    calendar.addEvent(event);

    KCalendarCore::OccurrenceIterator rIt(calendar, start, actualEnd);
    QList<QDateTime> expectedEventOccurrences;
    expectedEventOccurrences << start << start.addDays(1);
    while (rIt.hasNext()) {
        rIt.next();
        qDebug() << rIt.occurrenceStartDate().toString();
        QCOMPARE(expectedEventOccurrences.removeAll(rIt.occurrenceStartDate()), 1);
    }
    QCOMPARE(expectedEventOccurrences.size(), 0);
}

void TestOccurrenceIterator::testWithExceptionThisAndFuture()
{
    KCalendarCore::MemoryCalendar calendar(QTimeZone::utc());

    QDateTime start(QDate(2013, 03, 10), QTime(10, 0, 0), QTimeZone::UTC);
    QDateTime end(QDate(2013, 03, 10), QTime(11, 0, 0), QTimeZone::UTC);

    QDateTime recurrenceId1(QDate(2013, 03, 11), QTime(10, 0, 0), QTimeZone::UTC);
    QDateTime exceptionStart1(QDate(2013, 03, 11), QTime(12, 0, 0), QTimeZone::UTC);
    QDateTime exceptionEnd1(QDate(2013, 03, 11), QTime(13, 0, 0), QTimeZone::UTC);

    QDateTime recurrenceId2(QDate(2013, 03, 13), QTime(10, 0, 0), QTimeZone::UTC);
    QDateTime exceptionStart2(QDate(2013, 03, 13), QTime(14, 0, 0), QTimeZone::UTC);
    QDateTime exceptionEnd2(QDate(2013, 03, 13), QTime(15, 0, 0), QTimeZone::UTC);

    QDateTime actualEnd(QDate(2013, 03, 14), QTime(11, 0, 0), QTimeZone::UTC);

    KCalendarCore::Event::Ptr event1(new KCalendarCore::Event());
    event1->setUid(QStringLiteral("event1"));
    event1->setSummary(QStringLiteral("event1"));
    event1->setDtStart(start);
    event1->setDtEnd(end);
    event1->recurrence()->setDaily(1);
    calendar.addEvent(event1);

    KCalendarCore::Event::Ptr exception1(new KCalendarCore::Event());
    exception1->setUid(event1->uid());
    exception1->setSummary(QStringLiteral("exception1"));
    exception1->setRecurrenceId(recurrenceId1);
    exception1->setThisAndFuture(true);
    exception1->setDtStart(exceptionStart1);
    exception1->setDtEnd(exceptionEnd1);
    calendar.addEvent(exception1);

    KCalendarCore::Event::Ptr exception2(new KCalendarCore::Event());
    exception2->setUid(event1->uid());
    exception2->setSummary(QStringLiteral("exception2"));
    exception2->setRecurrenceId(recurrenceId2);
    exception2->setDtStart(exceptionStart2);
    exception2->setDtEnd(exceptionEnd2);
    calendar.addEvent(exception2);

    int occurrence = 0;
    KCalendarCore::OccurrenceIterator rIt(calendar, start, actualEnd);
    while (rIt.hasNext()) {
        rIt.next();
        occurrence++;
        QCOMPARE(rIt.recurrenceId(), start.addDays(occurrence - 1));
        if (occurrence == 1) {
            QCOMPARE(rIt.occurrenceStartDate(), start);
            QCOMPARE(rIt.incidence()->summary(), event1->summary());
        }
        if (occurrence == 2) {
            QCOMPARE(rIt.occurrenceStartDate(), exceptionStart1);
            QCOMPARE(rIt.incidence()->summary(), exception1->summary());
        }
        if (occurrence == 3) {
            QCOMPARE(rIt.occurrenceStartDate(), exceptionStart1.addDays(1));
            QCOMPARE(rIt.incidence()->summary(), exception1->summary());
        }
        if (occurrence == 4) {
            QCOMPARE(rIt.occurrenceStartDate(), exceptionStart2);
            QCOMPARE(rIt.incidence()->summary(), exception2->summary());
        }
        if (occurrence == 5) {
            QCOMPARE(rIt.occurrenceStartDate(), exceptionStart1.addDays(3));
            QCOMPARE(rIt.incidence()->summary(), exception1->summary());
        }
    }
    QCOMPARE(occurrence, 5);
}

void TestOccurrenceIterator::testSubDailyRecurrences()
{
    KCalendarCore::MemoryCalendar calendar(QTimeZone::utc());

    QDateTime start(QDate(2013, 03, 10), QTime(10, 0, 0), QTimeZone::UTC);
    QDateTime actualEnd(QDate(2013, 03, 10), QTime(13, 0, 0), QTimeZone::UTC);

    KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    event->setUid(QStringLiteral("event"));
    event->setDtStart(start);
    event->recurrence()->setHourly(1);
    event->recurrence()->setDuration(2);
    calendar.addEvent(event);

    KCalendarCore::OccurrenceIterator rIt(calendar, start, actualEnd);
    QList<QDateTime> expectedEventOccurrences;
    expectedEventOccurrences << start << start.addSecs(60 * 60);
    while (rIt.hasNext()) {
        rIt.next();
        qDebug() << rIt.occurrenceStartDate().toString();
        QCOMPARE(expectedEventOccurrences.removeAll(rIt.occurrenceStartDate()), 1);
    }
    QCOMPARE(expectedEventOccurrences.size(), 0);
}

void TestOccurrenceIterator::testJournals()
{
    KCalendarCore::MemoryCalendar calendar(QTimeZone::utc());

    const QDateTime today = QDateTime::currentDateTimeUtc();
    const QDateTime yesterday = today.addDays(-1);
    const QDateTime tomorrow = today.addDays(1);

    KCalendarCore::Journal::Ptr journal(new KCalendarCore::Journal());
    journal->setUid(QStringLiteral("journal"));
    journal->setDtStart(today);
    calendar.addJournal(journal);

    KCalendarCore::OccurrenceIterator rIt(calendar, yesterday, tomorrow);
    QVERIFY(rIt.hasNext());
    rIt.next();
    QCOMPARE(rIt.occurrenceStartDate(), today);
    QVERIFY(!rIt.hasNext());

    KCalendarCore::OccurrenceIterator rIt2(calendar, tomorrow, tomorrow.addDays(1));
    QVERIFY(!rIt2.hasNext());
}

void TestOccurrenceIterator::testEndDate()
{
    const QDateTime start(QDate(2021, 8, 30), QTime(10, 0, 0));
    const QDateTime end(QDate(2021, 8, 30), QTime(11, 0, 0));

    const QDateTime start2(QDate(2021, 8, 30), QTime(15, 0, 0));
    const QDateTime end2(QDate(2021, 8, 30), QTime(18, 0, 0));

    KCalendarCore::MemoryCalendar calendar(QTimeZone::utc());

    KCalendarCore::Event::Ptr event(new KCalendarCore::Event);
    event->setUid(QStringLiteral("event"));
    event->setSummary(QStringLiteral("event"));
    event->setDtStart(start);
    event->setDtEnd(end);
    event->recurrence()->setDaily(1);
    event->recurrence()->addRDateTimePeriod(KCalendarCore::Period(start2, end2));
    calendar.addEvent(event);

    KCalendarCore::Journal::Ptr journal(new KCalendarCore::Journal);
    journal->setUid(QStringLiteral("journal"));
    journal->setDtStart(start);
    calendar.addJournal(journal);

    KCalendarCore::OccurrenceIterator rIt(calendar, start, start.addDays(1));
    // The base event.
    QVERIFY(rIt.hasNext());
    rIt.next();
    QCOMPARE(rIt.occurrenceStartDate(), start);
    QCOMPARE(rIt.occurrenceEndDate(), end);
    // The additional occurrence with a longer duration.
    QVERIFY(rIt.hasNext());
    rIt.next();
    QCOMPARE(rIt.occurrenceStartDate(), start2);
    QCOMPARE(rIt.occurrenceEndDate(), end2);
    // The recurring occurrence with the base duration.
    QVERIFY(rIt.hasNext());
    rIt.next();
    QCOMPARE(rIt.occurrenceStartDate(), start.addDays(1));
    QCOMPARE(rIt.occurrenceEndDate(), end.addDays(1));
    // The journal.
    QVERIFY(rIt.hasNext());
    rIt.next();
    QCOMPARE(rIt.occurrenceStartDate(), start);
    QVERIFY(!rIt.occurrenceEndDate().isValid());
    QVERIFY(!rIt.hasNext());
}

#include "moc_testoccurrenceiterator.cpp"
