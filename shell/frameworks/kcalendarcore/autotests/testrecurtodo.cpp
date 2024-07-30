/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2011 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "testrecurtodo.h"
#include "todo.h"

#include <QDebug>
#include <QTest>

QTEST_MAIN(RecurTodoTest)

using namespace KCalendarCore;

void RecurTodoTest::setTimeZone(const char *zonename)
{
    QVERIFY(QTimeZone(zonename).isValid());
    qputenv("TZ", zonename);
    const QDateTime currentDateTime = QDateTime::currentDateTime();
    QVERIFY(currentDateTime.timeZone().isValid());
    QCOMPARE(currentDateTime.timeZoneAbbreviation(), QString::fromLatin1(zonename));
}

void RecurTodoTest::testAllDay()
{
    setTimeZone("UTC");
    const QDate currentDate = QDate::currentDate();
    const QDateTime currentUtcDateTime = QDateTime::currentDateTimeUtc();

    const QDate dueDate(QDate::currentDate());
    QCOMPARE(currentDate, dueDate);
    QCOMPARE(currentDate, currentUtcDateTime.date());

    Todo todo;
    todo.setDtStart(QDateTime(dueDate.addDays(-1), {}));
    todo.setDtDue(QDateTime(dueDate, {}));
    todo.setSummary(QStringLiteral("All day event"));
    todo.setAllDay(true);

    QCOMPARE(todo.dtStart().daysTo(todo.dtDue()), 1);
    QVERIFY(!todo.recurs());

    Recurrence *recurrence = todo.recurrence();
    recurrence->unsetRecurs();
    recurrence->setDaily(1);
    QCOMPARE(todo.dtDue(), QDateTime(dueDate, {}));
    QCOMPARE(todo.percentComplete(), 0);
    QVERIFY(todo.recurs()); // Previously it did not recur
    todo.setCompleted(currentUtcDateTime);
    QVERIFY(todo.recurs());
    QCOMPARE(todo.percentComplete(), 0); // It is still not done
    const QDate newStartDate = todo.dtStart().date();
    const QDate newDueDate = todo.dtDue().date();
    QCOMPARE(newStartDate, currentDate);
    QCOMPARE(newStartDate.daysTo(newDueDate), 1);

    todo.setCompleted(currentUtcDateTime);

    QCOMPARE(newDueDate, currentDate.addDays(1));
    QCOMPARE(todo.dtDue(true /*first occurrence*/).date(), dueDate);
}

void RecurTodoTest::testRecurrenceStart()
{
    setTimeZone("UTC");
    const QDateTime currentDateTime = QDateTime::currentDateTime();
    const QDate currentDate = currentDateTime.date();
    const QTime currentTimeWithMS = currentDateTime.time();

    const QDate fourDaysAgo(currentDate.addDays(-4));
    const QDate treeDaysAgo(currentDate.addDays(-3));
    const QTime currentTime(currentTimeWithMS.hour(), currentTimeWithMS.minute(), currentTimeWithMS.second());

    Todo todo;
    Recurrence *recurrence = todo.recurrence();
    recurrence->unsetRecurs();
    recurrence->setDaily(1);
    todo.setDtStart(QDateTime(fourDaysAgo, currentTime));
    const QDateTime originalDtDue(treeDaysAgo, currentTime);
    todo.setDtDue(originalDtDue);
    todo.setSummary(QStringLiteral("Not an all day event"));
    QVERIFY(!todo.allDay());
    QVERIFY(recurrence->startDateTime().isValid());
}

void RecurTodoTest::testNonAllDay()
{
    setTimeZone("UTC");
    const QDateTime currentDateTime = QDateTime::currentDateTime();
    const QDate currentDate = currentDateTime.date();
    const QTime currentTimeWithMS = currentDateTime.time();

    const QDate fourDaysAgo(currentDate.addDays(-4));
    const QDate treeDaysAgo(currentDate.addDays(-3));
    const QTime currentTime(currentTimeWithMS.hour(), currentTimeWithMS.minute(), currentTimeWithMS.second());

    Todo todo;
    todo.setDtStart(QDateTime(fourDaysAgo, currentTime));
    const QDateTime originalDtDue(treeDaysAgo, currentTime);
    todo.setDtDue(originalDtDue);
    todo.setSummary(QStringLiteral("Not an all day event"));
    QVERIFY(!todo.allDay());
    Recurrence *recurrence = todo.recurrence();
    recurrence->unsetRecurs();
    recurrence->setDaily(1);
    QVERIFY(recurrence->startDateTime().isValid());
    QCOMPARE(todo.dtDue(), originalDtDue);
    todo.setCompleted(QDateTime::currentDateTimeUtc());
    QVERIFY(todo.recurs());
    QVERIFY(todo.percentComplete() == 0);

    const bool equal = todo.dtStart() == QDateTime(currentDate, currentTime, todo.dtStart().timeZone()).addDays(1);
    if (!equal) {
        qDebug() << "Test Failed. dtDue = " << todo.dtDue().toString() << "OriginalDtDue:" << originalDtDue.toString()
                 << "QDateTime:" << QDateTime(currentDate, currentTime, todo.dtDue().timeZone()).addDays(1).toString();
    }

    QVERIFY(equal);

    todo.setCompleted(QDateTime::currentDateTimeUtc());
    QCOMPARE(todo.dtStart(), QDateTime(currentDate, currentTime, todo.dtStart().timeZone()).addDays(2));
    QCOMPARE(todo.dtDue(true /*first occurrence*/), QDateTime(treeDaysAgo, currentTime));
}

void RecurTodoTest::testIsAllDay()
{
    KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());
    todo->setUid(QStringLiteral("todo"));
    todo->setDtStart(QDateTime(QDate(2013, 03, 10), QTime(10, 0, 0), QTimeZone::UTC));
    todo->setDtDue(QDateTime(QDate(2013, 03, 10), QTime(10, 0, 0), QTimeZone::UTC));
    todo->recurrence()->setDaily(1);
    todo->recurrence()->setDuration(2);
    QCOMPARE(todo->allDay(), false);
    QCOMPARE(todo->recurrence()->allDay(), false);

    KCalendarCore::Todo::Ptr allDay(new KCalendarCore::Todo());
    allDay->setUid(QStringLiteral("todo"));
    allDay->setDtStart(QDateTime(QDate(2013, 03, 10), {}, QTimeZone::UTC));
    allDay->setDtDue(QDateTime(QDate(2013, 03, 10), {}, QTimeZone::UTC));
    allDay->setAllDay(true);
    allDay->recurrence()->setDaily(1);
    allDay->recurrence()->setDuration(2);
    QCOMPARE(allDay->allDay(), true);
    QCOMPARE(allDay->recurrence()->allDay(), true);
}

void RecurTodoTest::testHasDueDate()
{
    KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());
    todo->setUid(QStringLiteral("todo"));
    todo->setDtStart(QDateTime(QDate(2013, 03, 10), QTime(10, 0, 0), QTimeZone::UTC));
    todo->recurrence()->setDaily(1);
    todo->recurrence()->setDuration(2);
    QVERIFY(!todo->hasDueDate());
}

void RecurTodoTest::testRecurTodo_data()
{
    QTest::addColumn<QDateTime>("dtstart");
    QTest::addColumn<QDateTime>("dtdue");

    // Can't use QDateTime::currentDateTimeUtc() due to milliseconds mismatching
    const QDateTime today = QDateTime::fromSecsSinceEpoch(QDateTime::currentSecsSinceEpoch(), QTimeZone::UTC);
    const QDateTime tomorrow = today.addDays(1);
    const QDateTime invalid;

    QTest::newRow("valid dtstart") << today << invalid;
    QTest::newRow("valid dtstart and dtdue") << today << tomorrow;
    QTest::newRow("valid dtdue") << invalid << today;
}

void RecurTodoTest::testRecurTodo()
{
    QFETCH(QDateTime, dtstart);
    QFETCH(QDateTime, dtdue);

    KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());
    todo->setUid(QStringLiteral("todo"));
    todo->setDtStart(dtstart);
    todo->setDtDue(dtdue);
    todo->recurrence()->setDaily(1);

    const bool legacyMode = !dtstart.isValid();
    QCOMPARE(todo->percentComplete(), 0);

    // Recur it
    todo->setCompleted(QDateTime::currentDateTimeUtc());
    QCOMPARE(todo->percentComplete(), 0);

    if (legacyMode) {
        QVERIFY(todo->dtDue().isValid());
        QVERIFY(!todo->dtStart().isValid());
        QCOMPARE(todo->dtDue(), dtdue.addDays(1));

        QCOMPARE(todo->dtDue(/**first=*/true), dtdue);
    } else {
        QVERIFY(todo->dtStart().isValid());
        QVERIFY(!(todo->dtDue().isValid() ^ dtdue.isValid()));
        QCOMPARE(todo->dtStart(), dtstart.addDays(1));

        if (dtdue.isValid()) {
            const int delta = dtstart.daysTo(dtdue);
            QCOMPARE(todo->dtStart().daysTo(todo->dtDue()), delta);
        }

        QCOMPARE(todo->dtStart(/**first=*/true), dtstart);
    }
}

void RecurTodoTest::testDtStart()
{
    QDateTime start(QDate(2013, 03, 10), QTime(10, 0, 0), QTimeZone::UTC);
    KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());
    todo->setUid(QStringLiteral("todo"));
    todo->setDtStart(start);
    todo->recurrence()->setDaily(1);
    todo->recurrence()->setDuration(2);
    QCOMPARE(todo->dtStart(), start);

    KCalendarCore::Todo::Ptr todoWithDue(new KCalendarCore::Todo());
    todoWithDue->setUid(QStringLiteral("todoWithDue"));
    todoWithDue->setDtStart(start);
    todoWithDue->setDtDue(QDateTime(start).addSecs(60));
    todoWithDue->recurrence()->setDaily(1);
    todoWithDue->recurrence()->setDuration(2);
    QCOMPARE(todoWithDue->dtStart(), start);
}

void RecurTodoTest::testRecurrenceBasedOnDtStart()
{
    const QDateTime dtstart(QDate(2013, 03, 10), QTime(10, 0, 0), QTimeZone::UTC);
    const QDateTime dtdue(QDate(2013, 03, 10), QTime(11, 0, 0), QTimeZone::UTC);

    KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());
    todo->setUid(QStringLiteral("todo"));
    todo->setDtStart(dtstart);
    todo->setDtDue(dtdue);
    todo->recurrence()->setDaily(1);
    todo->recurrence()->setDuration(3);

    QCOMPARE(todo->recurrence()->getNextDateTime(dtstart.addMSecs(-1)), dtstart);
    QCOMPARE(todo->recurrence()->getNextDateTime(dtstart), dtstart.addDays(1));
    QCOMPARE(todo->recurrence()->getNextDateTime(dtstart.addDays(1)), dtstart.addDays(2));
    QCOMPARE(todo->recurrence()->getNextDateTime(dtstart.addDays(2)), QDateTime());
}

// For backwards compatibility only
void RecurTodoTest::testRecurrenceBasedOnDue()
{
    const QDateTime dtdue(QDate(2013, 03, 10), QTime(11, 0, 0), QTimeZone::UTC);

    KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());
    todo->setUid(QStringLiteral("todo"));
    todo->setDtDue(dtdue);
    todo->recurrence()->setDaily(1);
    todo->recurrence()->setDuration(3);

    QCOMPARE(todo->recurrence()->getNextDateTime(dtdue.addMSecs(-1)), dtdue);
    QCOMPARE(todo->recurrence()->getNextDateTime(dtdue), dtdue.addDays(1));
    QCOMPARE(todo->recurrence()->getNextDateTime(dtdue.addDays(1)), dtdue.addDays(2));
    QCOMPARE(todo->recurrence()->getNextDateTime(dtdue.addDays(2)), QDateTime());
}

/** Test that occurrences specified by a recurrence rule are eliminated by
 * exception dates.
 */
void RecurTodoTest::testRecurrenceExdates()
{
    const QDateTime dtstart(QDate(2013, 03, 10), QTime(10, 0, 0), QTimeZone::UTC);
    const QDateTime dtdue(QDate(2013, 03, 10), QTime(11, 0, 0), QTimeZone::UTC);

    KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());
    todo->setUid(QStringLiteral("todo"));
    todo->setDtStart(dtstart);
    todo->setDtDue(dtdue);
    todo->recurrence()->setDaily(1);
    todo->recurrence()->setDuration(3);

    // Test for boundary errors.
    todo->recurrence()->addExDateTime(dtstart);
    todo->recurrence()->addExDateTime(dtstart.addDays(2));

    QCOMPARE(todo->recurrence()->getNextDateTime(dtstart.addMSecs(-1)), dtstart.addDays(1));
    QCOMPARE(todo->recurrence()->getNextDateTime(dtstart.addDays(1)), QDateTime());
}

#include "moc_testrecurtodo.cpp"
