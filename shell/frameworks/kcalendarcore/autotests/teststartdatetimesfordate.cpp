/*
  SPDX-FileCopyrightText: 2021 Glen Ditchfield <GJDitchfield@acm.org>
  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "event.h"

#include <QTest>

using namespace KCalendarCore;

class TestStartDateTimesForDate: public QObject
{
    Q_OBJECT

    const QDate testDate { 2021, 01, 31};
    const QTimeZone tz = QTimeZone::systemTimeZone();
private Q_SLOTS:
    void testNonRecurringEvents_data();
    void testNonRecurringEvents();
};

Event::Ptr mkEvent(bool allDay, QDateTime dtStart, QDateTime dtEnd)
{
    Event::Ptr event = Event::Ptr(new Event());
    event->setDtStart(dtStart);
    event->setDtEnd(dtEnd);
    event->setAllDay(allDay);
    return event;
}

void TestStartDateTimesForDate::testNonRecurringEvents_data()
{
    QTest::addColumn<bool>("allDay");
    QTest::addColumn<QDateTime>("dtStart");
    QTest::addColumn<QDateTime>("dtEnd");
    QTest::addColumn<bool>("expected");

    QTest::newRow("all day, before") << true << QDateTime{testDate.addDays(-2), {}, tz} << QDateTime(testDate.addDays(-1), {}, tz) << false;
    QTest::newRow("all day, up to") << true << QDateTime{testDate.addDays(-1), {}, tz} << QDateTime(testDate, {}, tz) << true;
    QTest::newRow("all day, around") << true << QDateTime{testDate.addDays(-2), {}, tz} << QDateTime(testDate.addDays(+1), {}, tz) << true;
    QTest::newRow("all day, on") << true << QDateTime{testDate, {}, tz} << QDateTime(testDate, {}, tz) << true;
    QTest::newRow("all day, from") << true << QDateTime{testDate, {}, tz} << QDateTime(testDate.addDays(+1), {}, tz) << true;
    QTest::newRow("all day, after") << true << QDateTime{testDate.addDays(+1), {}, tz} << QDateTime(testDate.addDays(+2), {}, tz) << false;

    QTest::newRow("before") << false << QDateTime{testDate.addDays(-2), {}, tz} << QDateTime(testDate, {}, tz).addMSecs(-1) << false;
    QTest::newRow("up to") << false << QDateTime{testDate.addDays(-1), {}, tz} << QDateTime(testDate, {00,00,01}, tz) << true;
    QTest::newRow("around") << false << QDateTime{testDate.addDays(-2), {}, tz} << QDateTime(testDate.addDays(+1), {}, tz) << true;
    QTest::newRow("on") << false << QDateTime{testDate, {}, tz} << QDateTime(testDate.addDays(+1), {}, tz).addMSecs(-1) << true;
    QTest::newRow("during") << false << QDateTime{testDate, {01,00,00}, tz} << QDateTime(testDate, {21,00,00}, tz) << true;
    QTest::newRow("from") << false << QDateTime{testDate, {}, tz} << QDateTime(testDate.addDays(+2), {}, tz) << true;
    QTest::newRow("after") << false << QDateTime{testDate.addDays(+1), {}, tz} << QDateTime(testDate.addDays(+2), {}, tz) << false;
}

void TestStartDateTimesForDate::testNonRecurringEvents()
{
    QFETCH(bool, allDay);
    QFETCH(QDateTime, dtStart);
    QFETCH(QDateTime, dtEnd);
    QFETCH(bool, expected);

    auto event = mkEvent(allDay, dtStart, dtEnd);
    auto result = event->startDateTimesForDate(testDate, tz);
    QVERIFY(expected == (result == QList<QDateTime> {dtStart}));
}

QTEST_MAIN(TestStartDateTimesForDate)
#include "teststartdatetimesfordate.moc"
