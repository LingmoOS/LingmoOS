/*
 * SPDX-FileCopyrightText: 2020 Glen Ditchfield <GJDitchfield@acm.org>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "testdateserialization.h"
#include "icalformat.h"
#include "memorycalendar.h"

#include <QTest>

QTEST_MAIN(TestDateSerialization)

using namespace KCalendarCore;

// Check that serialization and deserialization of a minimal recurring todo
// preserves the start and due dates of the todo and its first occurrence.
// See bug 345498.
void TestDateSerialization::testNewRecurringTodo()
{
    QDateTime startDate = QDate(2015, 3, 24).startOfDay();
    QDateTime dueDate{startDate.addDays(1)};

    Todo::Ptr todo(new Todo);
    todo->setDtStart(startDate);
    todo->setDtDue(dueDate, true);
    todo->setAllDay(true);
    todo->recurrence()->setMonthly(1);

    MemoryCalendar::Ptr cal{new MemoryCalendar(QTimeZone::utc())};
    cal->addIncidence(todo);

    ICalFormat format;
    const QString result = format.toString(cal);

    Incidence::Ptr i = format.fromString(result);
    QVERIFY(i);
    QVERIFY(i->type() == IncidenceBase::IncidenceType::TypeTodo);
    Todo::Ptr newTodo = i.staticCast<Todo>();
    QCOMPARE(newTodo->dtStart(true), startDate);
    QCOMPARE(newTodo->dtStart(false), startDate);
    QCOMPARE(newTodo->dtDue(true), dueDate);
    QCOMPARE(newTodo->dtDue(false), dueDate);
}

// Check that serialization and deserialization of a minimal recurring todo
// that has been completed once preserves the start and due dates of the todo
// and correctly calculates the start and due dates of the next occurrence.
// See bug 345565.
void TestDateSerialization::testTodoCompletedOnce()
{
    QDateTime startDate = QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1).startOfDay();
    QDateTime dueDate{startDate.addDays(1)};

    Todo::Ptr todo(new Todo);
    todo->setDtStart(startDate);
    todo->setDtDue(dueDate, true);
    todo->setAllDay(true);
    todo->recurrence()->setMonthly(1);

    MemoryCalendar::Ptr cal{new MemoryCalendar(QTimeZone::utc())};
    cal->addIncidence(todo);

    ICalFormat format;
    QString result = format.toString(cal);

    Incidence::Ptr i = format.fromString(result);
    QVERIFY(i);
    QVERIFY(i->type() == IncidenceBase::IncidenceType::TypeTodo);
    todo = i.staticCast<Todo>();
    todo->setCompleted(dueDate);

    cal = MemoryCalendar::Ptr{new MemoryCalendar(QTimeZone::utc())};
    cal->addIncidence(todo);
    result = format.toString(cal);

    QCOMPARE(todo->dtStart(true), startDate);
    QCOMPARE(todo->dtStart(false), startDate.addMonths(1));
    QCOMPARE(todo->dtDue(true), dueDate);
    QCOMPARE(todo->dtDue(false), dueDate.addMonths(1));
}

// Check that QDateTime values with UTC offsets are handled correctly
void TestDateSerialization::testUTCOffset()
{
    QDateTime startDate({2022, 3, 6}, {10, 25}, QTimeZone::fromSecondsAheadOfUtc(3600));

    Event::Ptr event(new Event);
    event->setDtStart(startDate);

    MemoryCalendar::Ptr cal{new MemoryCalendar(QTimeZone::utc())};
    cal->addIncidence(event);

    ICalFormat format;
    QString result = format.toString(cal);

    Incidence::Ptr i = format.fromString(result);
    QVERIFY(i);
    QCOMPARE(i->dtStart(), startDate);
    // Qt::OffsetFromUTC is turned into Qt::Timezone with a "UTC+/-X" timezone right now
    // but Qt::OffsetFromUTC would be equally correct here should this ever change in the future
    QCOMPARE(i->dtStart().timeSpec(), Qt::TimeZone);
    QCOMPARE(i->dtStart().timeZone().offsetFromUtc(i->dtStart()), 3600);
}

#include "moc_testdateserialization.cpp"
