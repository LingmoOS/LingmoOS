/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2015 Sandro Knauß <sknauss@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testcalendarobserver.h"
#include "calendar.h"
#include "filestorage.h"
#include "memorycalendar.h"

#include <QSignalSpy>
#include <QTest>
#include <QTimeZone>

QTEST_MAIN(CalendarObserverTest)

using namespace KCalendarCore;
Q_DECLARE_METATYPE(KCalendarCore::Incidence::Ptr)
Q_DECLARE_METATYPE(const Calendar *)

class SimpleObserver : public QObject, public Calendar::CalendarObserver
{
    Q_OBJECT
public:
    SimpleObserver(Calendar *cal, QObject *parent = nullptr)
        : QObject(parent)
        , mCal(cal)
    {
    }

    Calendar *mCal = nullptr;
Q_SIGNALS:
    void incidenceAdded(const KCalendarCore::Incidence::Ptr &incidence);
    void incidenceChanged(const KCalendarCore::Incidence::Ptr &incidence);
    void incidenceAboutToBeDeleted(const KCalendarCore::Incidence::Ptr &incidence);
    void incidenceDeleted(const KCalendarCore::Incidence::Ptr &incidence, const Calendar *calendar);

protected:
    void calendarIncidenceAdded(const KCalendarCore::Incidence::Ptr &incidence) override
    {
        Q_EMIT incidenceAdded(incidence);
    }

    void calendarIncidenceChanged(const KCalendarCore::Incidence::Ptr &incidence) override
    {
        Q_EMIT incidenceChanged(incidence);
    }

    void calendarIncidenceAboutToBeDeleted(const KCalendarCore::Incidence::Ptr &incidence) override
    {
        QVERIFY(mCal->incidences().contains(incidence));
        Q_EMIT incidenceAboutToBeDeleted(incidence);
    }

    void calendarIncidenceDeleted(const KCalendarCore::Incidence::Ptr &incidence, const Calendar *calendar) override
    {
        QCOMPARE(calendar, mCal);
        QVERIFY(!calendar->incidences().contains(incidence));
        Q_EMIT incidenceDeleted(incidence, calendar);
    }
};

void CalendarObserverTest::testAdd()
{
    qRegisterMetaType<KCalendarCore::Incidence::Ptr>();
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    SimpleObserver ob(cal.data());
    QSignalSpy spy(&ob, &SimpleObserver::incidenceAdded);
    cal->registerObserver(&ob);
    Event::Ptr event1 = Event::Ptr(new Event());
    event1->setUid(QStringLiteral("1"));

    cal->addEvent(event1);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<KCalendarCore::Incidence::Ptr>(), static_cast<KCalendarCore::Incidence::Ptr>(event1));
}

void CalendarObserverTest::testChange()
{
    qRegisterMetaType<KCalendarCore::Incidence::Ptr>();
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    SimpleObserver ob(cal.data());
    QSignalSpy spy(&ob, &SimpleObserver::incidenceChanged);
    cal->registerObserver(&ob);
    Event::Ptr event1 = Event::Ptr(new Event());
    event1->setUid(QStringLiteral("1"));
    cal->addEvent(event1);
    QCOMPARE(spy.count(), 0);

    event1->setDescription(QStringLiteral("desc"));
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<KCalendarCore::Incidence::Ptr>(), static_cast<KCalendarCore::Incidence::Ptr>(event1));
}

void CalendarObserverTest::testDelete()
{
    qRegisterMetaType<KCalendarCore::Incidence::Ptr>();
    qRegisterMetaType<const Calendar *>();
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    SimpleObserver ob(cal.data());
    QSignalSpy spy1(&ob, &SimpleObserver::incidenceAboutToBeDeleted);
    QSignalSpy spy2(&ob, &SimpleObserver::incidenceDeleted);
    cal->registerObserver(&ob);
    Event::Ptr event1 = Event::Ptr(new Event());
    event1->setUid(QStringLiteral("1"));
    cal->addEvent(event1);
    QCOMPARE(spy1.count(), 0);
    QCOMPARE(spy2.count(), 0);

    cal->deleteEvent(event1);
    QCOMPARE(spy1.count(), 1);
    QCOMPARE(spy2.count(), 1);
    QList<QVariant> arguments = spy1.takeFirst();
    QCOMPARE(arguments.at(0).value<KCalendarCore::Incidence::Ptr>(), static_cast<KCalendarCore::Incidence::Ptr>(event1));
    arguments = spy2.takeFirst();
    QCOMPARE(arguments.at(0).value<KCalendarCore::Incidence::Ptr>(), static_cast<KCalendarCore::Incidence::Ptr>(event1));
    QCOMPARE(arguments.at(1).value<const Calendar *>(), cal.data());
}

#include "moc_testcalendarobserver.cpp"
#include "testcalendarobserver.moc"
