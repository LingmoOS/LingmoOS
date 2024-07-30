/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2006, 2008 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "testtodo.h"
#include "attachment.h"
#include "event.h"
#include "sorting.h"
#include "todo.h"

#include <QTest>
#include <QTimeZone>
QTEST_MAIN(TodoTest)

using namespace KCalendarCore;

const auto TEST_TZ = "UTC";

void TodoTest::initTestCase()
{
    qputenv("TZ", TEST_TZ);
}

void TodoTest::testValidity()
{
    QDate dt = QDate::currentDate();
    Todo todo;
    todo.setDtStart(QDateTime(dt, {}));
    todo.setDtDue(QDateTime(dt, {}).addDays(1));
    todo.setSummary(QStringLiteral("To-do1 Summary"));
    todo.setDescription(QStringLiteral("This is a description of the first to-do"));
    todo.setLocation(QStringLiteral("the place"));
    todo.setPercentComplete(5);

    QCOMPARE(todo.summary(), QStringLiteral("To-do1 Summary"));
    QCOMPARE(todo.location(), QStringLiteral("the place"));
    QCOMPARE(todo.percentComplete(), 5);
}

void TodoTest::testCompare()
{
    QDate dt = QDate::currentDate();
    Todo todo1;
    todo1.setDtStart(QDateTime(dt, {}));
    todo1.setDtDue(QDateTime(dt, {}).addDays(1));
    todo1.setSummary(QStringLiteral("To-do1 Summary"));
    todo1.setDescription(QStringLiteral("This is a description of the first to-do"));
    todo1.setLocation(QStringLiteral("the place"));
    todo1.setCompleted(true);

    Todo todo2;
    todo2.setDtStart(QDateTime(dt, {}).addDays(1));
    todo2.setDtDue(QDateTime(dt, {}).addDays(2));
    todo2.setSummary(QStringLiteral("To-do2 Summary"));
    todo2.setDescription(QStringLiteral("This is a description of the second to-do"));
    todo2.setLocation(QStringLiteral("the other place"));
    todo2.setCompleted(false);

    QVERIFY(!(todo1 == todo2));
    QCOMPARE(todo1.dtDue(), todo2.dtStart());
    QCOMPARE(todo2.summary(), QStringLiteral("To-do2 Summary"));
    QVERIFY(!(todo1.isCompleted() == todo2.isCompleted()));
}

void TodoTest::testDtDueEqual()
{
    QDateTime dt {QDate::currentDate(), QTime::currentTime(), QTimeZone(TEST_TZ)};
    QVERIFY(dt.timeSpec() == Qt::TimeZone);

    Todo t1;
    t1.setDtDue(dt);
    auto t2 = t1.clone();
    QVERIFY(t1 == *t2);

    // Create a "floating" datetime, which represents the same instant in real time
    // because we're still running in the test's time zone.
    dt.setTimeZone(QTimeZone::LocalTime);

    t1.setDtDue(dt);
    QVERIFY(t1 != *t2);
    t2->setDtDue(dt);
    QVERIFY(t1 == *t2);
}

void TodoTest::testClone()
{
    QDate dt = QDate::currentDate();
    Todo todo1;
    todo1.setDtStart(QDateTime(dt, {}));
    todo1.setDtDue(QDateTime(dt, {}).addDays(1));
    todo1.setSummary(QStringLiteral("Todo1 Summary"));
    todo1.setDescription(QStringLiteral("This is a description of the first todo"));
    todo1.setLocation(QStringLiteral("the place"));

    Todo *todo2 = todo1.clone();
    QCOMPARE(todo1.summary(), todo2->summary());
    QCOMPARE(todo1.dtStart(), todo2->dtStart());
    QCOMPARE(todo1.dtDue(), todo2->dtDue());
    QCOMPARE(todo1.description(), todo2->description());
    QCOMPARE(todo1.location(), todo2->location());
    QCOMPARE(todo1.isCompleted(), todo2->isCompleted());
    delete todo2;
}

void TodoTest::testCopyIncidence()
{
    QDate dt = QDate::currentDate();
    Event event;
    event.setDtStart(QDateTime(dt, {}));
    event.setSummary(QStringLiteral("Event1 Summary"));
    event.setDescription(QStringLiteral("This is a description of the first event"));
    event.setLocation(QStringLiteral("the place"));

    Todo todo(event);
    QCOMPARE(todo.uid(), event.uid());
    QCOMPARE(todo.dtStart(), event.dtStart());
    QCOMPARE(todo.summary(), event.summary());
    QCOMPARE(todo.description(), event.description());
    QCOMPARE(todo.location(), event.location());
}

void TodoTest::testCopyConstructor()
{
    QDate dt = QDate::currentDate();
    Todo todo1;
    todo1.setDtStart(QDateTime(dt, {}));
    todo1.setDtDue(QDateTime(dt, {}).addDays(1));
    todo1.setSummary(QStringLiteral("Todo1 Summary"));
    todo1.setDescription(QStringLiteral("This is a description of the first todo"));
    todo1.setLocation(QStringLiteral("the place"));

    Todo todo2 {todo1};
    QVERIFY(todo1 == todo2);
}

void TodoTest::testAssign()
{
    QDate dt = QDate::currentDate();
    Todo todo1;
    todo1.setDtStart(QDateTime(dt, {}));
    todo1.setDtDue(QDateTime(dt, {}).addDays(1));
    todo1.setSummary(QStringLiteral("Todo1 Summary"));
    todo1.setDescription(QStringLiteral("This is a description of the first todo"));
    todo1.setLocation(QStringLiteral("the place"));

    IncidenceBase *todo2 = new Todo;
    *todo2 = todo1;     // Use IncidenceBase's virtual assignment.
    QVERIFY(todo1 == *todo2);

    // release todo2
    delete todo2;
}

void TodoTest::testSetCompletedWithDate()
{
    Todo t;
    t.setStatus(Incidence::StatusNone);
    QVERIFY(!t.isCompleted());
    QVERIFY(!t.hasCompletedDate());
    QVERIFY(t.status() != Incidence::StatusCompleted);
    QVERIFY(t.percentComplete() != 100);

    const auto now = QDateTime::currentDateTimeUtc();
    t.setCompleted(now);
    QVERIFY(t.isCompleted());
    QVERIFY(t.completed() == now);
    QVERIFY(t.hasCompletedDate());
    QVERIFY(t.status() == Incidence::StatusNone);
    QVERIFY(t.percentComplete() == 100);
}

void TodoTest::testSetCompletedWithoutDate()
{
    Todo t;
    t.setStatus(Incidence::StatusNeedsAction);
    QVERIFY(!t.isCompleted());
    QVERIFY(!t.hasCompletedDate());
    QVERIFY(t.status() != Incidence::StatusCompleted);
    QVERIFY(t.percentComplete() != 100);

    t.setCompleted(QDateTime());
    QVERIFY(t.isCompleted());
    QVERIFY(!t.hasCompletedDate());
    QVERIFY(t.status() == Incidence::StatusCompleted);
    QVERIFY(t.percentComplete() == 100);
}

void TodoTest::testSetCompleted()
{
    Todo todo1;
    todo1.setSummary(QStringLiteral("Todo Summary"));
    QDateTime today = QDateTime::currentDateTimeUtc();

    // due yesterday
    QDateTime originalDueDate = today.addDays(-1);

    todo1.setDtStart(originalDueDate);
    todo1.setDtDue(originalDueDate);
    todo1.recurrence()->setDaily(1);
    todo1.setCompleted(today);
    QVERIFY(originalDueDate != todo1.dtDue());
    QVERIFY(!todo1.isCompleted());
}

void TodoTest::testSetCompletedBool()
{
    Todo t;
    QVERIFY(!t.isCompleted());
    QVERIFY(!t.hasCompletedDate());
    QVERIFY(t.status() != Incidence::StatusCompleted);
    QVERIFY(t.percentComplete() != 100);

    t.setCompleted(true);
    QVERIFY(t.isCompleted());
    QVERIFY(!t.hasCompletedDate());
    QVERIFY(t.status() == Incidence::StatusCompleted);
    QVERIFY(t.percentComplete() == 100);

    const auto yesterday = QDateTime::currentDateTimeUtc().addDays(-1);
    t.setCompleted(yesterday);
    t.setCompleted(true);
    QVERIFY(t.isCompleted());
    QVERIFY(t.completed() == yesterday);
    QVERIFY(t.status() == Incidence::StatusCompleted);
    QVERIFY(t.percentComplete() == 100);

    t.setCompleted(false);
    QVERIFY(!t.isCompleted());
    QVERIFY(!t.hasCompletedDate());
    QVERIFY(t.status() == Incidence::StatusNone);
    QVERIFY(t.percentComplete() == 0);
}

void TodoTest::testSetPercent()
{
    Todo t;
    t.setStatus(Incidence::StatusCompleted);
    t.setCompleted(QDateTime::currentDateTimeUtc());
    t.setPercentComplete(100);
    QVERIFY(t.percentComplete() == 100);
    QVERIFY(t.isCompleted());

    // Completion reset if necessary.
    QVERIFY(t.hasCompletedDate());
    t.setPercentComplete(99);
    QVERIFY(t.percentComplete() == 99);
    QVERIFY(!t.isCompleted());
    QVERIFY(!t.hasCompletedDate());
    QVERIFY(t.status() != Incidence::StatusCompleted);

    t.setPercentComplete(0);
    QVERIFY(t.percentComplete() == 0);
    QVERIFY(!t.isCompleted());
    QVERIFY(!t.hasCompletedDate());
    QVERIFY(t.status() != Incidence::StatusCompleted);
}

void TodoTest::testStatus()
{
    QDateTime today = QDateTime::currentDateTimeUtc();
    QDateTime yesterday = today.addDays(-1);

    Todo todo1;
    todo1.setAllDay(true);
    todo1.setDtStart(yesterday);
    todo1.setDtDue(today);
    todo1.setPercentComplete(50);
    QVERIFY(todo1.isInProgress(false));
    QVERIFY(!todo1.isNotStarted(false));
    QVERIFY(!todo1.isOverdue());
    todo1.setPercentComplete(100);
    QVERIFY(todo1.isCompleted());

    Todo todo2 = todo1;
    todo2.setPercentComplete(33);
    todo2.setDtDue(QDateTime());
    QVERIFY(todo2.isOpenEnded());
}

void TodoTest::testSerializer_data()
{
    QTest::addColumn<KCalendarCore::Todo::Ptr>("todo");

    QDateTime today = QDateTime::currentDateTimeUtc();
    QDateTime yesterday = today.addDays(-1);

    Todo::Ptr todo1 = Todo::Ptr(new Todo());
    Todo::Ptr todo2 = Todo::Ptr(new Todo());
    Todo::Ptr todo3 = Todo::Ptr(new Todo());
    Todo::Ptr todo4 = Todo::Ptr(new Todo());
    Todo::Ptr todo5 = Todo::Ptr(new Todo());
    Todo::Ptr todo6 = Todo::Ptr(new Todo());

    todo1->setSummary(QStringLiteral("Summary"), false);
    todo1->setDescription(QStringLiteral("description"), false);
    todo1->setCreated(yesterday);
    todo1->setRevision(50);
    todo1->setDtDue(yesterday);
    todo1->setDtStart(today);
    todo1->setPercentComplete(50);
    todo1->setLocation(QStringLiteral("<b>location</b>"), false);

    todo2->setDescription(QStringLiteral("<b>description</b>"), true);
    todo2->setSummary(QStringLiteral("<b>Summary2</b>"), true);
    todo2->setLocation(QStringLiteral("<b>location</b>"), true);
    todo2->setDtDue(yesterday);
    todo2->setPercentComplete(100);

    todo3->setDtStart(today);
    todo3->setPercentComplete(100);
    todo3->setCategories(QStringList() << QStringLiteral("a") << QStringLiteral("b") << QStringLiteral("c") << QStringLiteral("d"));
    todo3->setResources(QStringList() << QStringLiteral("a") << QStringLiteral("b") << QStringLiteral("c") << QStringLiteral("d"));
    todo3->setPriority(5);

    QVERIFY(!todo4->dirtyFields().contains(IncidenceBase::FieldRecurrence));
    todo4->recurrence()->setDaily(1);
    QVERIFY(todo4->dirtyFields().contains(IncidenceBase::FieldRecurrence));

    Attachment attachment(QStringLiteral("http://www.kde.org"));
    todo4->addAttachment(attachment);

    todo5->recurrence()->setDaily(1);
    todo5->setCompleted(today);
    todo5->setStatus(Incidence::StatusDraft);
    todo5->setSecrecy(Incidence::SecrecyPrivate);
    todo5->setRelatedTo(QStringLiteral("uid1"), Incidence::RelTypeParent);
    todo5->setGeoLatitude(40);
    todo5->setGeoLongitude(40);
    todo5->setOrganizer(QStringLiteral("organizer@mail.com"));

    todo6->recurrence()->setDaily(1);
    todo6->setCompleted(today);
    todo6->setRecurrenceId(yesterday);
    todo6->setStatus(Incidence::StatusDraft);
    todo6->setSecrecy(Incidence::SecrecyPrivate);
    todo6->setRelatedTo(QStringLiteral("uid1"), Incidence::RelTypeParent);
    todo6->setGeoLatitude(40);
    todo6->setGeoLongitude(40);
    todo6->setUid(QStringLiteral("uid22"));
    todo6->setLastModified(today);
    todo6->addContact(QStringLiteral("addContact"));

    // Remaining properties tested in testevent.cpp

    QTest::newRow("todo1") << todo1;
    QTest::newRow("todo2") << todo2;
    QTest::newRow("todo3") << todo3;
    QTest::newRow("todo4") << todo4;
    QTest::newRow("todo5") << todo5;
    QTest::newRow("todo6") << todo6;
}

void TodoTest::testSerializer()
{
    QFETCH(KCalendarCore::Todo::Ptr, todo);
    IncidenceBase::Ptr incidenceBase = todo.staticCast<KCalendarCore::IncidenceBase>();

    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << incidenceBase;

    Todo::Ptr todo2 = Todo::Ptr(new Todo());
    IncidenceBase::Ptr incidenceBase2 = todo2.staticCast<KCalendarCore::IncidenceBase>();
    QVERIFY(*todo != *todo2);
    QDataStream stream2(&array, QIODevice::ReadOnly);
    stream2 >> incidenceBase2;
    QVERIFY(*todo == *todo2);
}

void TodoTest::testRoles()
{
    const QDateTime today = QDateTime::currentDateTimeUtc();
    const QDateTime yesterday = today.addDays(-1);
    Todo todo;
    todo.setDtStart(today.addDays(-1));
    todo.setDtDue(today);
    QCOMPARE(todo.dateTime(Incidence::RoleDisplayStart), today);
    QCOMPARE(todo.dateTime(Incidence::RoleDisplayEnd), today);
    todo.setDtDue(QDateTime());
    QCOMPARE(todo.dateTime(Incidence::RoleDisplayStart), yesterday);
    QCOMPARE(todo.dateTime(Incidence::RoleDisplayEnd), yesterday);
}

void TodoTest::testIconNameOneoff()
{
    const QDateTime now = QDateTime::currentDateTime();
    Todo todo;
    todo.setDtStart(now);

    QCOMPARE(todo.iconName(), QLatin1String("view-calendar-tasks"));
    todo.setCompleted(now);
    QCOMPARE(todo.iconName(), QLatin1String("task-complete"));
}

void TodoTest::testIconNameRecurringNeverDue()
{
    const QDateTime now = QDateTime::currentDateTime();
    Todo todo;
    todo.setDtStart(now);
    todo.recurrence()->setDaily(1);

    QCOMPARE(todo.iconName(now), QLatin1String("view-calendar-tasks"));

    todo.setCompleted(now);
    QCOMPARE(todo.iconName(now), QLatin1String("task-complete"));
    QCOMPARE(todo.iconName(now.addDays(1)), QLatin1String("view-calendar-tasks"));
}

void TodoTest::testIconNameRecurringDue()
{
    const QDateTime now = QDateTime::currentDateTime();
    const QDateTime later = now.addSecs(3600);
    Todo todo;
    todo.setDtStart(now);
    todo.setDtDue(later, /*first=*/true);
    todo.recurrence()->setDaily(1);

    QCOMPARE(todo.iconName(now), QLatin1String("view-calendar-tasks"));
    QCOMPARE(todo.iconName(later), QLatin1String("view-calendar-tasks")); // Legacy case

    todo.setCompleted(now);
    QCOMPARE(todo.iconName(now), QLatin1String("task-complete"));
    QCOMPARE(todo.iconName(later), QLatin1String("task-complete")); // Legacy case
    QCOMPARE(todo.iconName(now.addDays(1)), QLatin1String("view-calendar-tasks"));
}

void TodoTest::testCategoriesComparison()
{
    QSharedPointer<Todo> small(new Todo);
    small->setCategories({QStringLiteral("alpha")});
    small->setSummary(QStringLiteral("alpha"));

    QSharedPointer<Todo> medium(new Todo);
    medium->setCategories({QStringLiteral("beta")});
    medium->setSummary(QStringLiteral("beta 1"));

    QSharedPointer<Todo> large(new Todo);
    large->setCategories({QStringLiteral("beta")});
    large->setSummary(QStringLiteral("beta 2"));

    QVERIFY(Incidences::categoriesLessThan(small, medium));
    QVERIFY(!Incidences::categoriesLessThan(medium, small));
    QVERIFY(Incidences::categoriesLessThan(medium, large));
    QVERIFY(!Incidences::categoriesLessThan(large, medium));
    QVERIFY(!Incidences::categoriesLessThan(small, small));

    QVERIFY(!Incidences::categoriesMoreThan(small, medium));
    QVERIFY(Incidences::categoriesMoreThan(medium, small));
    QVERIFY(!Incidences::categoriesMoreThan(medium, large));
    QVERIFY(Incidences::categoriesMoreThan(large, medium));
    QVERIFY(!Incidences::categoriesMoreThan(small, small));
}

void TodoTest::testDtDueComparison()
{
    auto now = QDateTime::currentDateTime();
    auto later = now.addSecs(1);

    QSharedPointer<Todo> small(new Todo);
    small->setDtDue(now);
    small->setAllDay(false);
    small->setSummary(QStringLiteral("now"));

    QSharedPointer<Todo> medium(new Todo);
    medium->setDtDue(later);
    medium->setAllDay(false);
    medium->setSummary(QStringLiteral("later 1"));

    QSharedPointer<Todo> large(new Todo);
    large->setDtDue(later);
    large->setAllDay(false);
    large->setSummary(QStringLiteral("later 2"));

    QSharedPointer<Todo> never(new Todo);
    never->setDtDue(QDateTime());
    never->setAllDay(false);
    never->setSummary(QStringLiteral("never"));

    QVERIFY(Todos::dueDateLessThan(small, medium));
    QVERIFY(!Todos::dueDateLessThan(medium, small));
    QVERIFY(Todos::dueDateLessThan(medium, large));
    QVERIFY(!Todos::dueDateLessThan(large, medium));
    QVERIFY(Todos::dueDateLessThan(large, never));
    QVERIFY(!Todos::dueDateLessThan(never, large));
    QVERIFY(!Todos::dueDateLessThan(small, small));
    QVERIFY(!Todos::dueDateLessThan(never, never));

    QVERIFY(!Todos::dueDateMoreThan(small, medium));
    QVERIFY(Todos::dueDateMoreThan(medium, small));
    QVERIFY(!Todos::dueDateMoreThan(medium, large));
    QVERIFY(Todos::dueDateMoreThan(large, medium));
    QVERIFY(!Todos::dueDateMoreThan(large, never));
    QVERIFY(Todos::dueDateMoreThan(never, large));
    QVERIFY(!Todos::dueDateMoreThan(small, small));
    QVERIFY(!Todos::dueDateLessThan(never, never));
}

void TodoTest::testDtDueChange()
{
    const QDate date {QDate::currentDate()};
    const QTime time {1, 0, 0};
    Todo todo;
    // Note:  QDateTime's default timespec QTimeZone::LocalTime => "floating" time.
    todo.setDtDue(QDateTime(date, time));

    todo.resetDirtyFields();
    todo.setDtDue(QDateTime(date, time));   // No change.
    QVERIFY(todo.dirtyFields().empty());

    todo.setDtDue(QDateTime(date, time).addDays(1));    // Normal change.
    QCOMPARE(todo.dirtyFields(), QSet<IncidenceBase::Field>{IncidenceBase::FieldDtDue});

    todo.resetDirtyFields();
    todo.setDtDue(QDateTime());     // "Unset" dtDue.
    QCOMPARE(todo.dirtyFields(), QSet<IncidenceBase::Field>{IncidenceBase::FieldDtDue});

    todo.resetDirtyFields();
    todo.setDtDue(QDateTime(date, time));   // Setting an unset due datetime.
    QCOMPARE(todo.dirtyFields(), QSet<IncidenceBase::Field>{IncidenceBase::FieldDtDue});

    // Change from a floating time to the same time in a fixed time zone.
    todo.resetDirtyFields();
    todo.setDtDue(QDateTime(date, time, QTimeZone(TEST_TZ)));
    QCOMPARE(todo.dirtyFields(), QSet<IncidenceBase::Field>{IncidenceBase::FieldDtDue});

    // Change from a time in a fixed time zone to a floating time.
    todo.resetDirtyFields();
    todo.setDtDue(QDateTime(date, time, QTimeZone::LocalTime));
    QCOMPARE(todo.dirtyFields(), QSet<IncidenceBase::Field>{IncidenceBase::FieldDtDue});
}

#include "moc_testtodo.cpp"
