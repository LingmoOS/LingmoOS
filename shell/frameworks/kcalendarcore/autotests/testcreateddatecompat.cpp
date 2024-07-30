/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "testcreateddatecompat.h"
#include "icalformat.h"
#include "memorycalendar.h"

#include <iostream>

#include <QTest>
#include <QTimeZone>

//"X-KDE-ICAL-IMPLEMENTATION-VERSION:1.0\n"

const char *const icalFile32 =
    "BEGIN:VCALENDAR\n"
    "PRODID:-//K Desktop Environment//NONSGML libkcal 3.2//EN\n"
    "VERSION:2.0\n"
    "BEGIN:VEVENT\n"
    "DTSTAMP:20031213T204753Z\n"
    "ORGANIZER:MAILTO:nobody@nowhere\n"
    "CREATED:20031213T204152Z\n"
    "UID:uid\n"
    "SEQUENCE:0\n"
    "LAST-MODIFIED:20031213T204152Z\n"
    "SUMMARY:Holladiho\n"
    "DTSTART:20031213T071500Z\n"
    "END:VEVENT\n"
    "END:VCALENDAR\n";

const char *const icalFile33 =
    "BEGIN:VCALENDAR\n"
    "PRODID:-//K Desktop Environment//NONSGML libkcal 3.2//EN\n"
    "VERSION:2.0\n"
    "X-KDE-ICAL-IMPLEMENTATION-VERSION:1.0\n"
    "BEGIN:VEVENT\n"
    "DTSTAMP:20031213T204753Z\n"
    "ORGANIZER:MAILTO:nobody@nowhere\n"
    "CREATED:20031213T204152Z\n"
    "UID:uid\n"
    "SEQUENCE:0\n"
    "LAST-MODIFIED:20031213T204152Z\n"
    "SUMMARY:Holladiho\n"
    "DTSTART:20031213T071500Z\n"
    "END:VEVENT\n"
    "END:VCALENDAR\n";

void CreatedDateCompatTest::testCompat32()
{
    KCalendarCore::MemoryCalendar::Ptr cal(new KCalendarCore::MemoryCalendar(QTimeZone::utc()));
    KCalendarCore::ICalFormat format;
    QVERIFY(format.fromRawString(cal, QByteArray(icalFile32)));
    KCalendarCore::Event::Ptr event = cal->event(QStringLiteral("uid"));
    QVERIFY(event);
    QCOMPARE(event->created(), QDateTime(QDate(2003, 12, 13), QTime(20, 47, 53), QTimeZone::UTC));
}

void CreatedDateCompatTest::testCompat33()
{
    KCalendarCore::MemoryCalendar::Ptr cal(new KCalendarCore::MemoryCalendar(QTimeZone::utc()));
    KCalendarCore::ICalFormat format;
    QVERIFY(format.fromRawString(cal, QByteArray(icalFile33)));
    KCalendarCore::Event::Ptr event = cal->event(QStringLiteral("uid"));
    QVERIFY(event);
    QCOMPARE(event->created(), QDateTime(QDate(2003, 12, 13), QTime(20, 41, 52), QTimeZone::UTC));
    QVERIFY(!event->customProperties().contains("X-KDE-ICAL-IMPLEMENTATION-VERSION"));
}

QTEST_MAIN(CreatedDateCompatTest)

#include "moc_testcreateddatecompat.cpp"
