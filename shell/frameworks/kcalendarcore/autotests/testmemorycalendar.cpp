/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2006-2007 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testmemorycalendar.h"
#include "filestorage.h"
#include "memorycalendar.h"

#include <QDebug>

#include <QTest>
#include <QTimeZone>
QTEST_MAIN(MemoryCalendarTest)

using namespace KCalendarCore;

void MemoryCalendarTest::testValidity()
{
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    cal->setProductId(QStringLiteral("fredware calendar"));
    QVERIFY(cal->productId() == QLatin1String("fredware calendar"));
    QVERIFY(cal->timeZoneId() == QByteArrayLiteral("UTC"));
    QVERIFY(cal->timeZone() == QTimeZone::utc());
}

void MemoryCalendarTest::testInvalidTimeZone()
{
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone()));
    // On invalid time zone, fallback to system time zone.
    QVERIFY(cal->timeZone() == QTimeZone::systemTimeZone());
}

void MemoryCalendarTest::testEvents()
{
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    cal->setProductId(QStringLiteral("fredware calendar"));
    QDate dt = QDate::currentDate();

    Event::Ptr event1 = Event::Ptr(new Event());
    event1->setUid(QStringLiteral("1"));
    event1->setDtStart(QDateTime(dt, {}));
    event1->setDtEnd(QDateTime(dt, {}).addDays(1));
    event1->setAllDay(true);
    event1->setSummary(QStringLiteral("Event1 Summary"));
    event1->setDescription(QStringLiteral("This is a description of the first event"));
    event1->setLocation(QStringLiteral("the place"));

    Event::Ptr event2 = Event::Ptr(new Event());
    event2->setUid(QStringLiteral("2"));
    event2->setDtStart(QDateTime(dt, {}).addDays(1));
    event2->setDtEnd(QDateTime(dt, {}).addDays(2));
    event2->setAllDay(true);
    event2->setSummary(QStringLiteral("Event2 Summary"));
    event2->setDescription(QStringLiteral("This is a description of the second event"));
    event2->setLocation(QStringLiteral("the other place"));

    QVERIFY(cal->addEvent(event1));
    QVERIFY(cal->addEvent(event2));

    FileStorage store(cal, QStringLiteral("foo.ics"));
    QVERIFY(store.save());
    QFile::remove(QStringLiteral("foo.ics"));
}

void MemoryCalendarTest::testIncidences()
{
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    cal->setProductId(QStringLiteral("fredware calendar"));
    QDate dt = QDate::currentDate();

    Event::Ptr event1 = Event::Ptr(new Event());
    event1->setUid(QStringLiteral("1"));
    event1->setDtStart(QDateTime(dt, {}));
    event1->setDtEnd(QDateTime(dt, {}).addDays(1));
    event1->setAllDay(true);
    event1->setSummary(QStringLiteral("Event1 Summary"));
    event1->setDescription(QStringLiteral("This is a description of the first event"));
    event1->setLocation(QStringLiteral("the place"));

    Event::Ptr event2 = Event::Ptr(new Event());
    event2->setUid(QStringLiteral("2"));
    event2->setDtStart(QDateTime(dt, {}).addDays(1));
    event2->setDtEnd(QDateTime(dt, {}).addDays(2));
    event2->setAllDay(true);
    event2->setSummary(QStringLiteral("Event2 Summary"));
    event2->setDescription(QStringLiteral("This is a description of the second event"));
    event2->setLocation(QStringLiteral("the other place"));

    QVERIFY(cal->addEvent(event1));
    QVERIFY(cal->addEvent(event2));

    Todo::Ptr todo1 = Todo::Ptr(new Todo());
    todo1->setUid(QStringLiteral("3"));
    todo1->setDtStart(QDateTime(dt, {}).addDays(1));
    todo1->setDtDue(QDateTime(dt, {}).addDays(2));
    todo1->setAllDay(true);
    todo1->setSummary(QStringLiteral("Todo1 Summary"));
    todo1->setDescription(QStringLiteral("This is a description of a todo"));
    todo1->setLocation(QStringLiteral("this place"));

    Todo::Ptr todo2 = Todo::Ptr(new Todo());
    todo2->setUid(QStringLiteral("4"));
    todo2->setDtStart(QDateTime(dt, {}).addDays(1));
    todo2->setAllDay(true);
    todo2->setSummary(QStringLiteral("<qt><h1>Todo2 Summary</h1></qt>"), true);
    todo2->setDescription(QStringLiteral("This is a description of a todo"));
    todo2->setLocation(QStringLiteral("<html><a href=\"http://www.fred.com\">this place</a></html>"), true);

    QVERIFY(cal->addTodo(todo1));
    QVERIFY(cal->addTodo(todo2));

    FileStorage store(cal, QStringLiteral("foo.ics"));
    QVERIFY(store.save());

    QVERIFY(store.load());
    Todo::Ptr todo = cal->incidence(QStringLiteral("4")).staticCast<Todo>();
    QVERIFY(todo->uid() == QLatin1Char('4'));
    QVERIFY(todo->summaryIsRich());
    QVERIFY(todo->locationIsRich());
    QFile::remove(QStringLiteral("foo.ics"));
}

void MemoryCalendarTest::testRelationsCrash()
{
    // Before, there was a crash that occurred only when reloading a calendar in which
    // the incidences had special relations.
    // This test tests that scenario, and will crash if it fails.
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    FileStorage store1(cal, QLatin1String(ICALTESTDATADIR) + QLatin1String("test_relations.ics"));
    QVERIFY(store1.load());
    const Todo::List oldTodos = cal->todos();
    qDebug() << "Loaded " << oldTodos.count() << " todos into oldTodos.";

    FileStorage store2(cal, QLatin1String(ICALTESTDATADIR) + QLatin1String("test_relations.ics"));
    QVERIFY(store2.load());
    const Todo::List newTodos = cal->todos();
    qDebug() << "Loaded " << newTodos.count() << " into newTodos.";

    // We can saftely access the old deleted todos here, since they are not really deleted
    // and are still kept in a map of deleted items somewhere.
    //
    // Here we make sure that non of the old items have connections to the new items, and
    // the other way around.

    // This doesn't makes sense so i commented it. when you load a calendar the second time
    // it reuses what it can, so oldTodo == newTodo

    /*  foreach (const Todo::Ptr &oldTodo, oldTodos ) {
        foreach (const Todo::Ptr &newTodo, newTodos ) {
          QVERIFY( oldTodo != newTodo );

          // Make sure that none of the new todos point to an old, deleted todo
          QVERIFY( newTodo->relatedTo() != oldTodo );
          QVERIFY( !newTodo->relations().contains( oldTodo ) );

          // Make sure that none of the old todos point to a new todo
          QVERIFY( oldTodo->relatedTo() != newTodo );
          QVERIFY( !oldTodo->relations().contains( newTodo ) );
        }
      }
    */
}

void MemoryCalendarTest::testRecurrenceExceptions()
{
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    cal->setProductId(QStringLiteral("fredware calendar"));
    QDate dt = QDate::currentDate();
    QDateTime start(dt, {});

    Event::Ptr event1 = Event::Ptr(new Event());
    event1->setUid(QStringLiteral("1"));
    event1->setDtStart(start);
    event1->setDtEnd(start.addDays(1));
    event1->setSummary(QStringLiteral("Event1 Summary"));
    event1->recurrence()->setDaily(1);
    event1->recurrence()->setDuration(3);
    QVERIFY(cal->addEvent(event1));

    const QDateTime recurrenceId = event1->dtStart().addDays(1);
    Event::Ptr exception1 = cal->createException(event1, recurrenceId).staticCast<Event>();
    QCOMPARE(exception1->recurrenceId(), recurrenceId);
    QCOMPARE(exception1->uid(), event1->uid());
    exception1->setSummary(QStringLiteral("exception"));

    QVERIFY(exception1);
    QVERIFY(cal->addEvent(exception1));

    QCOMPARE(cal->event(event1->uid()), event1);
    QCOMPARE(cal->event(event1->uid(), recurrenceId), exception1);

    const Event::List incidences = cal->rawEvents(start.date(), start.addDays(3).date(), start.timeZone());
    // Contains incidence and exception
    QCOMPARE(incidences.size(), 2);

    // Returns only exceptions for an event
    const Event::List exceptions = cal->eventInstances(event1);
    QCOMPARE(exceptions.size(), 1);
    QCOMPARE(exceptions.first()->uid(), event1->uid());
    QCOMPARE(exceptions.first()->summary(), exception1->summary());
}

void MemoryCalendarTest::testChangeRecurId()
{
    // When we change the recurring id, internal hashtables should be updated.

    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    QDateTime start(QDate::currentDate(), {});

    // Add main event
    Event::Ptr event1 = Event::Ptr(new Event());
    const QString uid = QStringLiteral("1");
    event1->setUid(uid);
    event1->setDtStart(start);
    event1->setDtEnd(start.addDays(1));
    event1->setAllDay(true);
    event1->setSummary(QStringLiteral("Event1 Summary"));
    event1->recurrence()->setDaily(1);
    event1->recurrence()->setDuration(3);
    QVERIFY(cal->addEvent(event1));

    // Add exception event:
    const QDateTime recurrenceId = event1->dtStart().addDays(1);
    Event::Ptr exception1 = cal->createException(event1, recurrenceId).staticCast<Event>();
    QCOMPARE(exception1->recurrenceId(), recurrenceId);
    QCOMPARE(exception1->uid(), event1->uid());
    exception1->setSummary(QStringLiteral("exception"));
    QVERIFY(exception1);
    QVERIFY(cal->addEvent(exception1));

    const QString oldIdentifier = exception1->instanceIdentifier();
    Incidence::Ptr foo = cal->instance(oldIdentifier);
    QVERIFY(foo && foo->hasRecurrenceId());
    // Now change the recurring id!
    exception1->setRecurrenceId(start.addDays(2));
    const QString newIdentifier = exception1->instanceIdentifier();
    QVERIFY(oldIdentifier != newIdentifier);

    foo = cal->instance(oldIdentifier);
    QVERIFY(!foo);

    foo = cal->instance(newIdentifier);
    QVERIFY(foo);

    // Test hashing
    Incidence::List incidences = cal->incidences();
    QVERIFY(incidences.count() == 2);

    QDateTime newRecId = start.addDays(2);
    Incidence::Ptr main = cal->incidence(uid);
    Incidence::Ptr exception = cal->incidence(uid, newRecId);
    Incidence::Ptr noException = cal->incidence(uid, recurrenceId);
    QVERIFY(!noException);
    QVERIFY(main);
    QVERIFY(exception);
    QVERIFY(exception->recurrenceId() == newRecId);
    QVERIFY(exception->summary() == QLatin1String("exception"));
    QVERIFY(main->summary() == event1->summary());
}

void MemoryCalendarTest::testRawEventsForDate()
{
    // We're checking that events at a date in a given time zone
    // are properly returned for the day after / before if
    // the calendar is for another time zone.
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));

    Event::Ptr event = Event::Ptr(new Event());
    event->setDtStart(QDateTime(QDate(2019, 10, 29), QTime(1, 30), QTimeZone("Asia/Ho_Chi_Minh")));

    QVERIFY(cal->addEvent(event));

    QCOMPARE(cal->rawEventsForDate(QDate(2019, 10, 28)).count(), 1);
    QCOMPARE(cal->rawEventsForDate(QDate(2019, 10, 29), QTimeZone("Asia/Ho_Chi_Minh")).count(), 1);

    cal->setTimeZoneId("Asia/Ho_Chi_Minh");
    QCOMPARE(cal->rawEventsForDate(QDate(2019, 10, 29)).count(), 1);
    QCOMPARE(cal->rawEventsForDate(QDate(2019, 10, 28), QTimeZone::utc()).count(), 1);

    event->setDtStart(QDateTime(QDate(2019, 10, 30), QTime(23, 00), QTimeZone::utc()));
    QCOMPARE(cal->rawEventsForDate(QDate(2019, 10, 31)).count(), 1);
    QCOMPARE(cal->rawEventsForDate(QDate(2019, 10, 30), QTimeZone::utc()).count(), 1);

    QVERIFY(cal->deleteIncidence(event));
    QCOMPARE(cal->rawEventsForDate(QDate(2019, 10, 31)).count(), 0);

    // Multi-days events are treated differently.
    event->setDtEnd(QDateTime(QDate(2019, 10, 31), QTime(23, 00), QTimeZone::utc()));
    QVERIFY(cal->addEvent(event));
    QCOMPARE(cal->rawEventsForDate(QDate(2019, 10, 31)).count(), 1);
    QCOMPARE(cal->rawEventsForDate(QDate(2019, 11, 1)).count(), 1);
}

class TestCalendarObserver: public Calendar::CalendarObserver
{
public:
    TestCalendarObserver(const Calendar::Ptr &cal)
        : mCalendar(cal)
    {
        cal->registerObserver(this);
    }
    ~TestCalendarObserver()
    {
        mCalendar->unregisterObserver(this);
    }
    void calendarIncidenceChanged(const Incidence::Ptr &incidence) override
    {
        mUpdated.append(incidence);
    }
    bool hasIncidenceChanged(const Incidence::Ptr &incidence) const
    {
        return std::find_if(mUpdated.constBegin(), mUpdated.constEnd(),
                            [incidence] (const Incidence::Ptr &it) {
                                return (it->uid() == incidence->uid()
                                        && it->recurrenceId() == incidence->recurrenceId());
                            }) != mUpdated.constEnd();
    }
    Incidence::List mUpdated;
private:
    Calendar::Ptr mCalendar;
};

void MemoryCalendarTest::testRawEvents()
{
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));

    Event::Ptr event = Event::Ptr(new Event());
    // This event span in 20201011T2330Z - 20201012T2330Z
    event->setDtStart(QDateTime(QDate(2020, 10, 12), QTime(1, 30), QTimeZone("Europe/Paris")));
    event->setDtEnd(QDateTime(QDate(2020, 10, 13), QTime(1, 30), QTimeZone("Europe/Paris")));

    QVERIFY(cal->addEvent(event));

    // Not full-event inclusive by default, UTC timezone.
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 1), QDate(2020, 10, 10)).count(), 0);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 11), QDate(2020, 10, 11)).count(), 1);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 12), QDate(2020, 10, 12)).count(), 1);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 13), QDate(2020, 10, 31)).count(), 0);
    QCOMPARE(cal->rawEvents(QDate(), QDate(2020, 10, 10)).count(), 0);
    QCOMPARE(cal->rawEvents(QDate(), QDate(2020, 10, 11)).count(), 1);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 13), QDate()).count(), 0);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 12), QDate()).count(), 1);

    // Changing the time zone we are considering the dates in.
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 1), QDate(2020, 10, 11), QTimeZone("Europe/Paris")).count(), 0);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 12), QDate(2020, 10, 12), QTimeZone("Europe/Paris")).count(), 1);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 13), QDate(2020, 10, 13), QTimeZone("Europe/Paris")).count(), 1);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 14), QDate(2020, 10, 31), QTimeZone("Europe/Paris")).count(), 0);
    QCOMPARE(cal->rawEvents(QDate(), QDate(2020, 10, 11), QTimeZone("Europe/Paris")).count(), 0);
    QCOMPARE(cal->rawEvents(QDate(), QDate(2020, 10, 12), QTimeZone("Europe/Paris")).count(), 1);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 14), QDate(), QTimeZone("Europe/Paris")).count(), 0);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 13), QDate(), QTimeZone("Europe/Paris")).count(), 1);

    // Full event must be in the span.
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 1), QDate(2020, 10, 10), QTimeZone(), true).count(), 0);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 11), QDate(2020, 10, 11), QTimeZone(), true).count(), 0);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 12), QDate(2020, 10, 12), QTimeZone(), true).count(), 0);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 11), QDate(2020, 10, 12), QTimeZone(), true).count(), 1);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 13), QDate(2020, 10, 31), QTimeZone(), true).count(), 0);
    QCOMPARE(cal->rawEvents(QDate(), QDate(2020, 10, 10), QTimeZone(), true).count(), 0);
    QCOMPARE(cal->rawEvents(QDate(), QDate(2020, 10, 11), QTimeZone(), true).count(), 0);
    QCOMPARE(cal->rawEvents(QDate(), QDate(2020, 10, 12), QTimeZone(), true).count(), 1);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 13), QDate(), QTimeZone(), true).count(), 0);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 12), QDate(), QTimeZone(), true).count(), 0);
    QCOMPARE(cal->rawEvents(QDate(2020, 10, 11), QDate(), QTimeZone(), true).count(), 1);
}

void MemoryCalendarTest::testDeleteIncidence()
{
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));

    Event::Ptr event = Event::Ptr(new Event());
    event->setDtStart(QDateTime(QDate(2021, 1, 4), QTime(10, 13), QTimeZone("Europe/Paris")));

    QVERIFY(cal->addEvent(event));
    QVERIFY(cal->instance(event->instanceIdentifier()));

    QVERIFY(cal->deleteIncidence(event));
    QVERIFY(cal->instance(event->instanceIdentifier()).isNull());

    event->recurrence()->setDaily(1);
    event->recurrence()->setDuration(3);
    QVERIFY(cal->addEvent(event));
    QVERIFY(cal->instance(event->instanceIdentifier()));

    Event::Ptr exception = Event::Ptr(event->clone());
    exception->recurrence()->clear();
    exception->setRecurrenceId(event->dtStart().addDays(1));
    exception->setDtStart(event->dtStart().addDays(1).addSecs(3600));
    QVERIFY(cal->addEvent(exception));
    QVERIFY(cal->instance(exception->instanceIdentifier()));

    Event::Ptr exception2 = Event::Ptr(event->clone());
    exception2->recurrence()->clear();
    exception2->setRecurrenceId(event->dtStart().addDays(2));
    exception2->setDtStart(event->dtStart().addDays(2).addSecs(-3600));
    QVERIFY(cal->addEvent(exception2));
    QVERIFY(cal->instance(exception2->instanceIdentifier()));

    QVERIFY(cal->deleteIncidence(exception));
    QVERIFY(cal->incidence(event->uid(), exception->recurrenceId()).isNull());
    QVERIFY(!cal->deleteIncidence(exception));
    QVERIFY(cal->incidence(event->uid(), exception2->recurrenceId()));
    QVERIFY(cal->incidence(event->uid()));

    QVERIFY(cal->deleteIncidence(event));
    QVERIFY(cal->incidence(event->uid(), exception2->recurrenceId()).isNull());
    QVERIFY(cal->incidence(event->uid()).isNull());
}

void MemoryCalendarTest::testUpdateIncidence()
{
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));

    const QDateTime dt(QDate(2021, 02, 25), QTime(14, 0), QTimeZone::UTC);
    Event::Ptr event(new Event());
    event->setCreated(dt);
    event->setLastModified(dt);
    event->setDtStart(dt);
    event->setDtEnd(dt.addSecs(3600));

    // Adding event to cal, makes cal an observer of event.
    QVERIFY(cal->addIncidence(event));
    QCOMPARE(cal->rawEventsForDate(dt.date(), dt.timeZone()).count(), 1);

    QVERIFY(cal->updateLastModifiedOnChange());

    const QDateTime now = QDateTime::currentDateTimeUtc();

    // Any single modification is updating the lastModified field.
    event->setSummary(QString::fromLatin1("test"));
    QVERIFY(event->lastModified().secsTo(now) < 5);

    // Reset lastModified field.
    event->setLastModified(dt);
    QCOMPARE(event->lastModified(), dt);

    // Any modification within a startUpdates()/endUpdates() should not touch
    // lastModified field, before the changes are completed.
    event->startUpdates();
    QVERIFY(cal->rawEventsForDate(dt.date(), dt.timeZone()).isEmpty());
    event->setSummary(QString::fromLatin1("test again"));
    QCOMPARE(event->lastModified(), dt);
    event->endUpdates();
    QVERIFY(event->lastModified().secsTo(now) < 5);
    QCOMPARE(cal->rawEventsForDate(dt.date(), dt.timeZone()).count(), 1);

    // Reset lastModified field.
    event->setLastModified(dt);
    QCOMPARE(event->lastModified(), dt);

    // Don't update lastModified on change.
    cal->setUpdateLastModifiedOnChange(false);
    event->setSummary(QString::fromLatin1("last test"));
    QCOMPARE(event->lastModified(), dt);
}

#include "moc_testmemorycalendar.cpp"
