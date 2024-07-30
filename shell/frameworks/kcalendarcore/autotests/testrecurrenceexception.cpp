/*
 * SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "testrecurrenceexception.h"
#include "memorycalendar.h"

#include <QTest>
QTEST_MAIN(TestRecurrenceException)

void TestRecurrenceException::testCreateTodoException()
{
    const QDateTime dtstart(QDate(2013, 03, 10), QTime(10, 0, 0), QTimeZone::UTC);
    const QDateTime dtdue(QDate(2013, 03, 10), QTime(11, 0, 0), QTimeZone::UTC);
    const QDateTime recurrenceId(QDateTime(dtstart).addDays(1));

    KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());
    todo->setUid(QStringLiteral("todo"));
    todo->setDtStart(dtstart);
    todo->setDtDue(dtdue);
    todo->recurrence()->setDaily(1);
    todo->recurrence()->setDuration(3);
    todo->setCreated(dtstart);
    todo->setLastModified(dtstart);

    const KCalendarCore::Todo::Ptr exception = KCalendarCore::MemoryCalendar::createException(todo, recurrenceId, false).staticCast<KCalendarCore::Todo>();
    QCOMPARE(exception->dtStart(), recurrenceId);
    QCOMPARE(exception->dtDue(), QDateTime(dtdue).addDays(1));
    QVERIFY(exception->created() >= todo->created());
    QVERIFY(exception->lastModified() >= exception->created());
    // FIXME should be done on clearing the recurrence, but we can't due to BC.
    // Probably not that important as long as dtRecurrence is ignored if the todo is not recurring
    // QCOMPARE(exception->dtRecurrence(), QDateTime());
    // TODO dtCompleted
}

#include "moc_testrecurrenceexception.cpp"
