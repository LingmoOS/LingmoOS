/*
  This file is part of the kcalcore library.
  SPDX-FileCopyrightText: 2007 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testfilestorage.h"
#include "filestorage.h"
#include "memorycalendar.h"

#include <QTest>
#include <QTimeZone>
QTEST_MAIN(FileStorageTest)

using namespace KCalendarCore;

void FileStorageTest::testValidity()
{
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    FileStorage fs(cal, QStringLiteral("fred.ics"));
    QCOMPARE(fs.fileName(), QStringLiteral("fred.ics"));
    QCOMPARE(fs.calendar().data(), cal.data());
}

void FileStorageTest::testSave()
{
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    FileStorage fs(cal, QStringLiteral("fred.ics"));

    QDate dt = QDate::currentDate();

    Event::Ptr event1 = Event::Ptr(new Event());
    event1->setUid(QStringLiteral("1"));
    event1->setDtStart(QDateTime(dt, {}));
    event1->setDtEnd(QDateTime(dt, {}).addDays(1));
    event1->setSummary(QStringLiteral("Event1 Summary"));
    event1->setDescription(QStringLiteral("This is a description of the first event"));
    event1->setLocation(QStringLiteral("the place"));
    cal->addEvent(event1);

    Event::Ptr event2 = Event::Ptr(new Event());
    event2->setUid(QStringLiteral("2"));
    event2->setDtStart(QDateTime(dt, {}).addDays(1));
    event2->setDtEnd(QDateTime(dt, {}).addDays(2));
    event2->setSummary(QStringLiteral("Event2 Summary"));
    event2->setDescription(QStringLiteral("This is a description of the second event"));
    event2->setLocation(QStringLiteral("the other place"));
    cal->addEvent(event2);

    QVERIFY(fs.open());
    QVERIFY(fs.save());
    QVERIFY(fs.close());
    QFile::remove(QStringLiteral("fred.ics"));
}

void FileStorageTest::testSaveLoadSave()
{
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    FileStorage fs(cal, QStringLiteral("fred.ics"));

    QDate dt = QDate::currentDate();

    Event::Ptr event1 = Event::Ptr(new Event());
    event1->setUid(QStringLiteral("1"));
    event1->setDtStart(QDateTime(dt, {}));
    event1->setDtEnd(QDateTime(dt, {}).addDays(1));
    event1->setSummary(QStringLiteral("Event1 Summary"));
    event1->setDescription(QStringLiteral("This is a description of the first event"));
    event1->setLocation(QStringLiteral("the place"));
    cal->addEvent(event1);

    Event::Ptr event2 = Event::Ptr(new Event());
    event2->setUid(QStringLiteral("2"));
    event2->setDtStart(QDateTime(dt, {}).addDays(1));
    event2->setDtEnd(QDateTime(dt, {}).addDays(2));
    event2->setSummary(QStringLiteral("Event2 Summary"));
    event2->setDescription(QStringLiteral("This is a description of the second event"));
    event2->setLocation(QStringLiteral("the other place"));
    cal->addEvent(event2);

    QVERIFY(fs.open());
    QVERIFY(fs.save());
    QVERIFY(fs.close());
    QVERIFY(fs.open());
    QVERIFY(fs.load());
    Event::Ptr e = fs.calendar()->incidence(QStringLiteral("1")).staticCast<Event>();
    QVERIFY(e != nullptr);
    QVERIFY(fs.close());
    QFile::remove(QStringLiteral("fred.ics"));

    QVERIFY(fs.open());
    QVERIFY(fs.save());
    QVERIFY(fs.close());
    QFile::remove(QStringLiteral("fred.ics"));
}

void FileStorageTest::testSpecialChars()
{
    const QDate currentDate = QDate::currentDate();
    const QString uid(QStringLiteral("12345"));

    Event::Ptr event = Event::Ptr(new Event());
    event->setUid(uid);
    event->setDtStart(QDateTime(currentDate, {}));
    event->setDtEnd(QDateTime(currentDate.addDays(1), {}));

    const QChar latin1_umlaut[] = {(QChar)0xFC, QLatin1Char('\0')};

    event->setSummary(QString(latin1_umlaut));

    // Save to file:
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    FileStorage fs(cal, QStringLiteral("bart.ics"));
    cal->addEvent(event);

    QVERIFY(fs.open());
    QVERIFY(fs.save());
    QVERIFY(fs.close());

    // Load again:
    MemoryCalendar::Ptr otherCalendar(new MemoryCalendar(QTimeZone::utc()));
    FileStorage otherFs(otherCalendar, QStringLiteral("bart.ics"));
    QVERIFY(otherFs.open());
    QVERIFY(otherFs.load());

    Event::Ptr otherEvent = otherCalendar->incidence(uid).staticCast<Event>();
    QVERIFY(otherFs.close());

    QVERIFY(otherEvent);

    // Make sure the retrieved incidence is equal to the original one
    QVERIFY(otherEvent->summary() == event->summary());
    QVERIFY(otherEvent->summary().toLatin1().size() == 1
            && strcmp(otherEvent->summary().toLatin1().constData(), QString(latin1_umlaut).toLatin1().constData()) == 0);

    // Make sure bart.ics is in UTF-8
    QFile file(QStringLiteral("bart.ics"));
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    const QByteArray bytesFromFile = file.readAll();
    const QChar utf_umlaut[] = {(QChar)0xC3, (QChar)0XBC, QLatin1Char('\0')};

    QVERIFY(bytesFromFile.contains(QString(utf_umlaut).toLatin1().constData()));
    QVERIFY(!bytesFromFile.contains(QString(latin1_umlaut).toLatin1().constData()));

    file.close();

    file.remove();
}

#include "moc_testfilestorage.cpp"
