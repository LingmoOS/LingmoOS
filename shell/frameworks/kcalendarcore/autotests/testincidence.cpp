/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2015 Sandro Knauß <knauss@kolabsys.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "testincidence.h"
#include "event.h"
#include "incidence.h"
#include <QTest>
#include <QTimeZone>
#include <cmath>

QTEST_MAIN(IncidenceTest)

Q_DECLARE_METATYPE(KCalendarCore::Incidence::DateTimeRole)

using namespace KCalendarCore;

const auto TEST_TZ = "Europe/Paris";

void IncidenceTest::initTestCase()
{
    qputenv("TZ", TEST_TZ);
}

void IncidenceTest::testDtStartChange()
{
    QDate dt = QDate::currentDate();
    QTime t = QTime::currentTime();
    Event inc;
    inc.setDtStart(QDateTime(dt, {}));
    inc.setAllDay(true);
    inc.recurrence()->setDaily(1);
    inc.resetDirtyFields();

    inc.setDtStart(QDateTime(dt, {}));
    QVERIFY(inc.dirtyFields().empty());

    inc.setDtStart(QDateTime(dt, t));
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldDtStart << IncidenceBase::FieldRecurrence);
    QCOMPARE(inc.recurrence()->startDateTime().time(), t);
    inc.resetDirtyFields();

    inc.setDtStart(QDateTime(dt, {}).addDays(1));
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldDtStart << IncidenceBase::FieldRecurrence);
    QCOMPARE(inc.recurrence()->startDateTime(), QDateTime(dt, {}).addDays(1));
    inc.resetDirtyFields();

    inc.setDtStart(QDateTime());
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldDtStart << IncidenceBase::FieldRecurrence);
    QCOMPARE(inc.recurrence()->startDateTime(), QDateTime());
    inc.resetDirtyFields();

    inc.setDtStart(QDateTime(dt, {}).addDays(1));
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldDtStart << IncidenceBase::FieldRecurrence);
    QCOMPARE(inc.recurrence()->startDateTime(), QDateTime(dt, {}).addDays(1));
}

void IncidenceTest::testDtStartEqual()
{
    QDateTime dt {QDate::currentDate(), QTime::currentTime(), QTimeZone(TEST_TZ)};
    QVERIFY(dt.timeSpec() == Qt::TimeZone);

    Event i1;
    i1.setDtStart(dt);
    auto i2 = i1.clone();
    QVERIFY(i1 == *i2);

    // Create a "floating" datetime, which represents the same instant in real time
    // because we're still running in the test's time zone.
    dt.setTimeZone(QTimeZone::LocalTime);

    i1.setDtStart(dt);
    QVERIFY(i1 != *i2);
    i2->setDtStart(dt);
    QVERIFY(i1 == *i2);
}

void IncidenceTest::testSummaryChange()
{
    Event inc;
    inc.setSummary(QStringLiteral("bla"), false);
    inc.resetDirtyFields();

    inc.setSummary(QStringLiteral("bla"), false);
    QVERIFY(inc.dirtyFields().empty());

    inc.setSummary(QStringLiteral("bla2"), false);
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldSummary);
    inc.resetDirtyFields();

    inc.setSummary(QStringLiteral("bla2"), true);
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldSummary);
}

void IncidenceTest::testLocationChange()
{
    Event inc;
    inc.setLocation(QStringLiteral("here"), false);
    inc.resetDirtyFields();

    inc.setLocation(QStringLiteral("here"), false);
    QVERIFY(inc.dirtyFields().empty());

    inc.setLocation(QStringLiteral("there"), false);
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldLocation);
    inc.resetDirtyFields();

    inc.setLocation(QStringLiteral("there"), true);
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldLocation);
}

void IncidenceTest::testGeo()
{
    Event inc;

    // Default state:  no GEO.
    QCOMPARE(inc.hasGeo(), false);
    QCOMPARE(inc.geoLatitude(), INVALID_LATLON);
    QCOMPARE(inc.geoLongitude(), INVALID_LATLON);
    QVERIFY(inc.dirtyFields().empty());

    // Set GEO, thoroughly.
    inc.setGeoLatitude(90.0);
    inc.setGeoLongitude(180.0);
    QCOMPARE(inc.hasGeo(), true);
    QCOMPARE(inc.geoLatitude(), 90.0);
    QCOMPARE(inc.geoLongitude(), 180.0);
    QCOMPARE(inc.dirtyFields(), (QSet{IncidenceBase::FieldGeoLatitude, IncidenceBase::FieldGeoLongitude}));
    inc.resetDirtyFields();
    inc.setGeoLatitude(-90.0);
    inc.setGeoLongitude(-180.0);
    QCOMPARE(inc.hasGeo(), true);
    QCOMPARE(inc.geoLatitude(), -90.0);
    QCOMPARE(inc.geoLongitude(), -180.0);
    QCOMPARE(inc.dirtyFields(), (QSet{IncidenceBase::FieldGeoLatitude, IncidenceBase::FieldGeoLongitude}));

    // Clear GEO, thoroughly.
    inc.resetDirtyFields();
    inc.setGeoLatitude(INVALID_LATLON);
    inc.setGeoLongitude(INVALID_LATLON);
    QCOMPARE(inc.hasGeo(), false);
    QCOMPARE(inc.geoLatitude(), INVALID_LATLON);
    QCOMPARE(inc.geoLongitude(), INVALID_LATLON);
    QCOMPARE(inc.dirtyFields(), (QSet{IncidenceBase::FieldGeoLatitude, IncidenceBase::FieldGeoLongitude}));

    // Error handling.
    inc.setGeoLatitude(90.0);
    inc.setGeoLongitude(180.0);
    inc.setGeoLatitude(INVALID_LATLON);
    QCOMPARE(inc.hasGeo(), false);
    QCOMPARE(inc.geoLatitude(), INVALID_LATLON);
    QCOMPARE(inc.geoLongitude(), INVALID_LATLON);
    inc.setGeoLatitude(90.0);
    inc.setGeoLongitude(180.0);
    inc.setGeoLongitude(INVALID_LATLON);
    QCOMPARE(inc.hasGeo(), false);
    QCOMPARE(inc.geoLatitude(), INVALID_LATLON);
    QCOMPARE(inc.geoLongitude(), INVALID_LATLON);
    inc.setGeoLatitude(90.0);
    inc.setGeoLongitude(180.0);
    inc.setGeoLatitude(NAN);
    QCOMPARE(inc.hasGeo(), false);
    QCOMPARE(inc.geoLatitude(), INVALID_LATLON);
    QCOMPARE(inc.geoLongitude(), INVALID_LATLON);
    inc.setGeoLatitude(90.0);
    inc.setGeoLongitude(180.0);
    inc.setGeoLongitude(NAN);
    QCOMPARE(inc.hasGeo(), false);
    QCOMPARE(inc.geoLatitude(), INVALID_LATLON);
    QCOMPARE(inc.geoLongitude(), INVALID_LATLON);
}

void IncidenceTest::testRecurrenceTypeChange()
{
    QDate dt = QDate::currentDate();
    Event inc;
    inc.setDtStart(QDateTime(dt, {}));
    KCalendarCore::Recurrence *r = inc.recurrence();
    r->setDaily(1);
    inc.resetDirtyFields();

    r->setDaily(1);
    QVERIFY(inc.dirtyFields().empty());

    r->setDaily(2);
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldRecurrence);
    inc.resetDirtyFields();

    r->setMonthly(2);
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldRecurrence);
}

void IncidenceTest::testRecurrenceEndTimeChange()
{
    QDate dt = QDate::currentDate();
    Event inc;
    inc.setDtStart(QDateTime(dt, {}));
    KCalendarCore::Recurrence *r = inc.recurrence();
    r->setDaily(1);
    r->setEndDateTime(QDateTime(dt, {}).addDays(1));
    inc.resetDirtyFields();

    r->setEndDateTime(QDateTime(dt, {}).addDays(1));
    QVERIFY(inc.dirtyFields().empty());

    r->setEndDateTime(QDateTime(dt, {}).addDays(2));
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldRecurrence);
}

void IncidenceTest::testRecurrenceEndTimeDurationChange()
{
    QDate dt = QDate::currentDate();
    Event inc;
    inc.setDtStart(QDateTime(dt, {}));
    KCalendarCore::Recurrence *r = inc.recurrence();
    r->setDaily(1);
    inc.resetDirtyFields();
    QCOMPARE(r->duration(), -1);
    QVERIFY(!r->endDateTime().isValid());

    r->setDuration(5);
    QVERIFY(r->endDateTime().isValid());
    inc.resetDirtyFields();

    // duration is set and set enddate to inValid
    r->setDuration(5);
    r->setEndDateTime(QDateTime());
    QVERIFY(inc.dirtyFields().empty());

    // now set valid enddate -> set duration to 0 by sideeffect
    r->setEndDateTime(QDateTime(dt, {}).addDays(1));
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldRecurrence);
    QCOMPARE(r->duration(), 0);
    QCOMPARE(r->endDateTime(), QDateTime(dt, {}).addDays(1));

    // with valid endDate, now setDuration and aftward set invalid endDate
    r->setEndDateTime(QDateTime(dt, {}).addDays(1));
    r->setDuration(5);
    inc.resetDirtyFields();

    r->setEndDateTime(QDateTime());
    QVERIFY(inc.dirtyFields().empty());
    QCOMPARE(r->endDate(), dt.addDays(4));
    QCOMPARE(r->duration(), 5);
}

void IncidenceTest::testRecurrenceDurationChange()
{
    QDate dt = QDate::currentDate();
    Event inc;
    inc.setDtStart(QDateTime(dt, {}));
    KCalendarCore::Recurrence *r = inc.recurrence();
    r->setDuration(1);
    inc.resetDirtyFields();

    r->setDuration(1);
    QVERIFY(inc.dirtyFields().empty());

    r->setDuration(2);
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldRecurrence);
}

void IncidenceTest::testRecurrenceExDatesChange()
{
    QDate dt = QDate::currentDate();
    Event inc;
    inc.setDtStart(QDateTime(dt, {}));
    KCalendarCore::Recurrence *r = inc.recurrence();
    r->setExDates(DateList() << dt.addDays(1) << dt.addDays(2));
    inc.resetDirtyFields();

    r->setExDates(DateList() << dt.addDays(2) << dt.addDays(1));
    QVERIFY(inc.dirtyFields().empty());

    r->setExDates(DateList() << dt.addDays(1));
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldRecurrence);
}

void IncidenceTest::testRecurrenceMonthlyDate()
{
    QDate dt = QDate::currentDate();
    Event inc;
    inc.setDtStart(QDateTime(dt, {}));
    KCalendarCore::Recurrence *r = inc.recurrence();
    r->setMonthly(1);
    r->setMonthlyDate(QList<int>() << 1 << 2 << 3);
    inc.resetDirtyFields();

    r->setMonthlyDate(QList<int>() << 3 << 1 << 2);
    QVERIFY(inc.dirtyFields().empty());

    r->setMonthlyDate(QList<int>() << 3 << 1);
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldRecurrence);
}

void IncidenceTest::testRecurrenceMonthlyPos()
{
    QDate dt = QDate::currentDate();
    RecurrenceRule::WDayPos pos1(1, 2);
    RecurrenceRule::WDayPos pos2(3, 4);
    RecurrenceRule::WDayPos pos3(1, 2);
    Event inc;
    inc.setDtStart(QDateTime(dt, {}));
    KCalendarCore::Recurrence *r = inc.recurrence();
    r->setYearly(1);
    r->setMonthlyPos(QList<RecurrenceRule::WDayPos>() << pos1 << pos2);
    inc.resetDirtyFields();

    // TODO: test sorting
    r->setMonthlyPos(QList<RecurrenceRule::WDayPos>() << pos1 << pos2);
    QVERIFY(inc.dirtyFields().empty());

    r->setMonthlyPos(QList<RecurrenceRule::WDayPos>() << pos3);
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldRecurrence);
}

void IncidenceTest::testRecurrenceYearlyDay()
{
    QDate dt = QDate::currentDate();
    Event inc;
    inc.setDtStart(QDateTime(dt, {}));
    KCalendarCore::Recurrence *r = inc.recurrence();
    r->setYearly(1);
    r->setYearlyDay(QList<int>() << 1 << 2 << 3);
    inc.resetDirtyFields();

    r->setYearlyDay(QList<int>() << 3 << 1 << 2);
    QVERIFY(inc.dirtyFields().empty());

    r->setYearlyDay(QList<int>() << 3 << 1);
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldRecurrence);
}

void IncidenceTest::testRecurrenceYearlyMonth()
{
    QDate dt = QDate::currentDate();
    Event inc;
    inc.setDtStart(QDateTime(dt, {}));
    KCalendarCore::Recurrence *r = inc.recurrence();
    r->setYearly(1);
    r->setYearlyMonth(QList<int>() << 1 << 2 << 3);
    inc.resetDirtyFields();

    r->setYearlyMonth(QList<int>() << 3 << 1 << 2);
    QVERIFY(inc.dirtyFields().empty());

    r->setYearlyMonth(QList<int>() << 3 << 1);
    QCOMPARE(inc.dirtyFields(), QSet<IncidenceBase::Field>() << IncidenceBase::FieldRecurrence);
}

#include "moc_testincidence.cpp"
