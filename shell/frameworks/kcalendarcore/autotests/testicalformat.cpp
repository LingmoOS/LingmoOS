/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-FileContributor: Sergio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testicalformat.h"
#include "event.h"
#include "icalformat.h"
#include "memorycalendar.h"
#include "occurrenceiterator.h"

#include <QTest>
#include <QTimeZone>

QTEST_MAIN(ICalFormatTest)

using namespace KCalendarCore;

void ICalFormatTest::testDeserializeSerialize()
{
    ICalFormat format;

    const QString serializedCalendar = QLatin1String(
        "BEGIN:VCALENDAR\n"
        "PRODID:-//IDN nextcloud.com//Calendar app 2.0.4//EN\n"
        "VERSION:2.0\n"
        "BEGIN:VEVENT\n"
        "CREATED:20201103T161248Z\n"
        "DTSTAMP:20201103T161340Z\n"
        "LAST-MODIFIED:20201103T161340Z\n"
        "SEQUENCE:2\n"
        "UID:bd1d299d-3b03-4514-be69-e680ad2ff884\n"
        "DTSTART;TZID=Europe/Paris:20201103T100000\n"
        "DTEND;TZID=Europe/Paris:20201103T110000\n"
        "SUMMARY:test recur\n"
        "RRULE:FREQ=DAILY;COUNT=4\n"
        "END:VEVENT\n"
        "BEGIN:VEVENT\n"
        "CREATED:20201103T161823Z\n"
        "DTSTAMP:20201103T161823Z\n"
        "LAST-MODIFIED:20201103T161823Z\n"
        "SEQUENCE:1\n"
        "UID:bd1d299d-3b03-4514-be69-e680ad2ff884\n"
        "DTSTART;TZID=Europe/Paris:20201104T111500\n"
        "DTEND;TZID=Europe/Paris:20201104T121500\n"
        "SUMMARY:test recur\n"
        "COLOR:khaki\n"
        "RECURRENCE-ID;TZID=Europe/Paris:20201104T100000\n"
        "END:VEVENT\n"
        "END:VCALENDAR");
    MemoryCalendar::Ptr calendar = MemoryCalendar::Ptr(new MemoryCalendar(QTimeZone::utc()));
    QVERIFY(format.fromString(calendar, serializedCalendar));
    const QString uid = QString::fromLatin1("bd1d299d-3b03-4514-be69-e680ad2ff884");
    Incidence::Ptr parent = calendar->incidence(uid);
    QVERIFY(parent);
    const QDateTime start(QDate(2020, 11, 3), QTime(9, 0), QTimeZone::utc());
    QCOMPARE(parent->dtStart(), start);
    QCOMPARE(parent.staticCast<Event>()->dtEnd(), start.addSecs(3600));
    QCOMPARE(parent->summary(), QString::fromLatin1("test recur"));
    QCOMPARE(parent->revision(), 2);
    Recurrence *recur = parent->recurrence();
    QVERIFY(recur->recurs());
    QCOMPARE(recur->duration(), 4);
    QCOMPARE(recur->recurrenceType(), static_cast<ushort>(Recurrence::rDaily));

    Incidence::Ptr occurrence = calendar->incidence(uid, start.addDays(1));
    QVERIFY(occurrence);
    const QDateTime startOcc(QDate(2020, 11, 4), QTime(10, 15), QTimeZone::utc());
    QCOMPARE(occurrence->dtStart(), startOcc);
    QCOMPARE(occurrence.staticCast<Event>()->dtEnd(), startOcc.addSecs(3600));
    QCOMPARE(occurrence->color(), QString::fromLatin1("khaki"));
    QCOMPARE(occurrence->summary(), QString::fromLatin1("test recur"));
    QCOMPARE(occurrence->revision(), 1);
    QVERIFY(occurrence->hasRecurrenceId());
    QCOMPARE(occurrence->recurrenceId(), start.addDays(1));

    const QString serialization = format.toString(calendar);
    QVERIFY(!serialization.isEmpty());
    MemoryCalendar::Ptr check = MemoryCalendar::Ptr(new MemoryCalendar(QTimeZone::utc()));
    QVERIFY(format.fromString(check, serialization));
    Incidence::Ptr reparent = check->incidence(uid);
    QVERIFY(reparent);
    QCOMPARE(*parent, *reparent);
    Incidence::Ptr reoccurence = check->incidence(uid, start.addDays(1));
    QVERIFY(reoccurence);
    QCOMPARE(*occurrence, *reoccurence);
}

void ICalFormatTest::testCharsets()
{
    ICalFormat format;
    const QDate currentDate = QDate::currentDate();
    Event::Ptr event = Event::Ptr(new Event());
    event->setUid(QStringLiteral("12345"));
    event->setDtStart(QDateTime(currentDate, {}));
    event->setDtEnd(QDateTime(currentDate.addDays(1), {}));
    event->setAllDay(true);

    // ü
    const QChar latin1_umlaut[] = {(QChar)0xFC, QLatin1Char('\0')};
    event->setSummary(QString(latin1_umlaut));

    // Test if toString( Incidence ) didn't mess charsets
    const QString serialized = format.toString(event.staticCast<Incidence>());
    const QChar utf_umlaut[] = {(QChar)0xC3, (QChar)0XBC, QLatin1Char('\0')};
    QVERIFY(serialized.toUtf8().contains(QString(utf_umlaut).toLatin1().constData()));
    QVERIFY(!serialized.toUtf8().contains(QString(latin1_umlaut).toLatin1().constData()));
    QVERIFY(serialized.toLatin1().contains(QString(latin1_umlaut).toLatin1().constData()));
    QVERIFY(!serialized.toLatin1().contains(QString(utf_umlaut).toLatin1().constData()));

    // test fromString( QString )
    const QString serializedCalendar = QLatin1String("BEGIN:VCALENDAR\nPRODID:-//K Desktop Environment//NONSGML libkcal 3.2//EN\nVERSION:2.0\n") + serialized
        + QLatin1String("\nEND:VCALENDAR");

    Incidence::Ptr event2 = format.fromString(serializedCalendar);
    QVERIFY(event->summary() == event2->summary());
    QVERIFY(event2->summary().toUtf8() == QByteArray(QString(utf_umlaut).toLatin1().constData()));

    // test save()
    MemoryCalendar::Ptr calendar(new MemoryCalendar(QTimeZone::utc()));
    calendar->addIncidence(event);
    QVERIFY(format.save(calendar, QLatin1String("hommer.ics")));

    // Make sure hommer.ics is in UTF-8
    QFile file(QStringLiteral("hommer.ics"));
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    const QByteArray bytesFromFile = file.readAll();
    QVERIFY(bytesFromFile.contains(QString(utf_umlaut).toLatin1().constData()));
    QVERIFY(!bytesFromFile.contains(QString(latin1_umlaut).toLatin1().constData()));
    file.close();

    // Test load:
    MemoryCalendar::Ptr calendar2(new MemoryCalendar(QTimeZone::utc()));
    QVERIFY(format.load(calendar2, QLatin1String("hommer.ics")));
    QVERIFY(calendar2->incidences().count() == 1);

    Event::Ptr loadedEvent = calendar2->incidences().at(0).staticCast<Event>();
    QVERIFY(loadedEvent->summary().toUtf8() == QByteArray(QString(utf_umlaut).toLatin1().constData()));
    QVERIFY(*loadedEvent == *event);

    // Test fromRawString()
    MemoryCalendar::Ptr calendar3(new MemoryCalendar(QTimeZone::utc()));
    QVERIFY(format.fromRawString(calendar3, bytesFromFile));
    QVERIFY(calendar3->incidences().count() == 1);
    QVERIFY(*calendar3->incidences().at(0) == *event);

    QFile::remove(QStringLiteral("hommer.ics"));
}

void ICalFormatTest::testVolatileProperties()
{
    // Volatile properties are not written to the serialized data
    ICalFormat format;
    const QDate currentDate = QDate::currentDate();
    Event::Ptr event = Event::Ptr(new Event());
    event->setUid(QStringLiteral("12345"));
    event->setDtStart(QDateTime(currentDate, {}));
    event->setDtEnd(QDateTime(currentDate.addDays(1), {}));
    event->setAllDay(true);
    event->setCustomProperty("VOLATILE", "FOO", QStringLiteral("BAR"));
    QString string = format.toICalString(event);
    Incidence::Ptr incidence = format.fromString(string);

    QCOMPARE(incidence->uid(), QStringLiteral("12345"));
    QVERIFY(incidence->customProperties().isEmpty());
}

void ICalFormatTest::testCuType()
{
    ICalFormat format;
    const QDate currentDate = QDate::currentDate();
    Event::Ptr event(new Event());
    event->setUid(QStringLiteral("12345"));
    event->setDtStart(QDateTime(currentDate, {}));
    event->setDtEnd(QDateTime(currentDate.addDays(1), {}));
    event->setAllDay(true);

    Attendee attendee(QStringLiteral("fred"), QStringLiteral("fred@flintstone.com"));
    attendee.setCuType(Attendee::Resource);

    event->addAttendee(attendee);

    const QString serialized = format.toString(event.staticCast<Incidence>());

    // test fromString(QString)
    const QString serializedCalendar = QLatin1String("BEGIN:VCALENDAR\nPRODID:-//K Desktop Environment//NONSGML libkcal 3.2//EN\nVERSION:2.0\n") + serialized
        + QLatin1String("\nEND:VCALENDAR");

    Incidence::Ptr event2 = format.fromString(serializedCalendar);
    QVERIFY(event2->attendeeCount() == 1);
    Attendee attendee2 = event2->attendees()[0];
    QVERIFY(attendee2.cuType() == attendee.cuType());
    QVERIFY(attendee2.name() == attendee.name());
    QVERIFY(attendee2.email() == attendee.email());
}

void ICalFormatTest::testAlarm()
{
    ICalFormat format;

    Event::Ptr event(new Event);
    event->setDtStart(QDate(2017, 03, 24).startOfDay());
    Alarm::Ptr alarm = event->newAlarm();
    alarm->setType(Alarm::Display);
    alarm->setStartOffset(Duration(0));

    const QString serialized = QLatin1String("BEGIN:VCALENDAR\nPRODID:-//K Desktop Environment//NONSGML libkcal 3.2//EN\nVERSION:2.0\n")
        + format.toString(event.staticCast<Incidence>()) + QLatin1String("\nEND:VCALENDAR");

    Incidence::Ptr event2 = format.fromString(serialized);
    Alarm::Ptr alarm2 = event2->alarms()[0];
    QCOMPARE(*alarm, *alarm2);
}

void ICalFormatTest::testDateTimeSerialization_data()
{
    QTest::addColumn<QDateTime>("dtStart");
    QTest::addColumn<QByteArray>("dtStartData");

    QTest::newRow("UTC time spec") << QDateTime(QDate(2021, 4, 9), QTime(12, 00), QTimeZone::UTC) << QByteArray("DTSTART:20210409T120000Z");
    QTest::newRow("UTC time zone")
        << QDateTime(QDate(2021, 4, 9), QTime(12, 00), QTimeZone::utc())
        << QByteArray("DTSTART:20210409T120000Z");
    QTest::newRow("named time zone")
        << QDateTime(QDate(2021, 4, 9), QTime(14, 00), QTimeZone("Europe/Paris"))
        << QByteArray("DTSTART;TZID=Europe/Paris:20210409T140000");
}

void ICalFormatTest::testDateTimeSerialization()
{
    QFETCH(QDateTime, dtStart);
    QFETCH(QByteArray, dtStartData);

    Incidence::Ptr event(new Event);
    QVERIFY(event);
    event->setDtStart(dtStart);
    QCOMPARE(event->dtStart(), dtStart);

    ICalFormat format;
    const QByteArray output = format.toRawString(event);
    const QList<QByteArray> lines = output.split('\n');
    for (const QByteArray &line: lines) {
        if (line.startsWith(QByteArray("DTSTART"))) {
            QCOMPARE(line.chopped(1), dtStartData);
            break;
        }
    }
}

void ICalFormatTest::testRDate()
{
    ICalFormat format;

    const QString serializedCalendar = QLatin1String(
        "BEGIN:VCALENDAR\n"
        "VERSION:2.0\n"
        "PRODID:-//Lotus Development Corporation//NONSGML Notes 9.0.1//EN_C\n"
        "METHOD:PUBLISH\n"
        "BEGIN:VEVENT\n"
        "DTSTART:20210630T100000Z\n"
        "DTEND:20210630T110000Z\n"
        "TRANSP:OPAQUE\n"
        "RDATE;VALUE=PERIOD:20210630T100000Z/20210630T110000Z\n"
        " ,20210825T100000Z/20210825T110000Z,20211027T100000Z/20211027T110000Z\n"
        " ,20211215T110000Z/PT2H\n"
        "LAST-MODIFIED:20210601T094627Z\n"
        "DTSTAMP:20210601T092939Z\n"
        "UID:5FC21473F5CC80CCC12586E70033ED9C-Lotus_Notes_Generated\n"
        "END:VEVENT\n"
        "END:VCALENDAR\n");
    MemoryCalendar::Ptr calendar(new MemoryCalendar(QTimeZone::utc()));
    QVERIFY(format.fromString(calendar, serializedCalendar));
    const QString uid = QString::fromLatin1("5FC21473F5CC80CCC12586E70033ED9C-Lotus_Notes_Generated");
    Incidence::Ptr event = calendar->incidence(uid);
    QVERIFY(event);
    QVERIFY(event->recurs());

    const QDateTime ev1(QDate(2021, 6, 30), QTime(10, 0), QTimeZone::UTC);
    const QDateTime ev2(QDate(2021, 8, 25), QTime(10, 0), QTimeZone::UTC);
    const QDateTime ev3(QDate(2021, 10, 27), QTime(10, 0), QTimeZone::UTC);
    const QDateTime ev4(QDate(2021, 12, 15), QTime(11, 0), QTimeZone::UTC);
    QCOMPARE(event->recurrence()->rDateTimes(),
             QList<QDateTime>() << ev1 << ev2 << ev3 << ev4);

    OccurrenceIterator it(*calendar, QDateTime(QDate(2021, 6, 1), QTime(0, 0)),
                          QDateTime(QDate(2021, 12, 31), QTime(0, 0)));
    QVERIFY(it.hasNext());
    it.next();
    QCOMPARE(it.occurrenceStartDate(), ev1);
    QCOMPARE(it.occurrenceEndDate(), ev1.addSecs(3600));
    QVERIFY(it.hasNext());
    it.next();
    QCOMPARE(it.occurrenceStartDate(), ev2);
    QCOMPARE(it.occurrenceEndDate(), ev2.addSecs(3600));
    QVERIFY(it.hasNext());
    it.next();
    QCOMPARE(it.occurrenceStartDate(), ev3);
    QCOMPARE(it.occurrenceEndDate(), ev3.addSecs(3600));
    QVERIFY(it.hasNext());
    it.next();
    QCOMPARE(it.occurrenceStartDate(), ev4);
    QCOMPARE(it.occurrenceEndDate(), ev4.addSecs(7200));

    const QStringList output = format.toString(calendar).split(QString::fromLatin1("\r\n"));
    QVERIFY(output.contains(QString::fromLatin1("RDATE;VALUE=PERIOD:20210630T100000Z/20210630T110000Z")));
    QVERIFY(output.contains(QString::fromLatin1("RDATE;VALUE=PERIOD:20210825T100000Z/20210825T110000Z")));
    QVERIFY(output.contains(QString::fromLatin1("RDATE;VALUE=PERIOD:20211027T100000Z/20211027T110000Z")));
    QVERIFY(output.contains(QString::fromLatin1("RDATE;VALUE=PERIOD:20211215T110000Z/PT2H")));
}

void ICalFormatTest::testDateTime_data()
{
    QTest::addColumn<QByteArray>("dtStartData");
    QTest::addColumn<QDateTime>("dtStart");

    QTest::newRow("clock time") << QByteArray("DTSTART:20191113T130000") << QDateTime(QDate(2019, 11, 13), QTime(13, 00), QTimeZone::LocalTime);
    QTest::newRow("date")
        << QByteArray("DTSTART;VALUE=DATE:20191113")
        << QDate(2019, 11, 13).startOfDay();
    QTest::newRow("UTC time") << QByteArray("DTSTART:20191113T130000Z") << QDateTime(QDate(2019, 11, 13), QTime(13, 00), QTimeZone::UTC);
    QTest::newRow("time zone time")
        << QByteArray("DTSTART;TZID=Europe/Paris:20191113T130000")
        << QDateTime(QDate(2019, 11, 13), QTime(13, 00), QTimeZone("Europe/Paris"));
}

void ICalFormatTest::testDateTime()
{
    QFETCH(QByteArray, dtStartData);
    QFETCH(QDateTime, dtStart);

    // test fromString(QString)
    const QByteArray serializedCalendar
        = "BEGIN:VCALENDAR\nPRODID:-//K Desktop Environment//NONSGML libkcal 3.2//EN\nVERSION:2.0\nBEGIN:VEVENT\nUID:12345\n"
          + dtStartData
          + "\nEND:VEVENT\nEND:VCALENDAR";

    ICalFormat format;
    Incidence::Ptr event = format.fromString(QString::fromUtf8(serializedCalendar));
    QVERIFY(event);
    QCOMPARE(dtStart, event->dtStart());
}

/**
 * If an instance does not have a UID, one will be created for it.
 */
void ICalFormatTest::testUidGeneration()
{
    const QString serializedCalendar = QLatin1String(
        "BEGIN:VCALENDAR\n"
        "VERSION:2.0\n"
        "BEGIN:VEVENT\n"
        "DTSTAMP:20201103T161340Z\n"
        "SUMMARY:test\n"
        "END:VEVENT\n"
        "END:VCALENDAR");
    auto calendar = MemoryCalendar::Ptr(new MemoryCalendar(QTimeZone::utc()));
    ICalFormat format;
    QVERIFY(format.fromString(calendar, serializedCalendar));
    const auto events = calendar->events();
    QCOMPARE(events.count(), 1);
    const auto event = events[0];
    QVERIFY( ! event->uid().isEmpty());
}

/**
 * Generated UIDs do not depend on the order of properties.
 */
void ICalFormatTest::testUidGenerationStability()
{
    ICalFormat format;

    const QString serializedCalendar1 = QLatin1String(
        "BEGIN:VCALENDAR\n"
        "VERSION:2.0\n"
        "BEGIN:VEVENT\n"
        "DTSTAMP:20201103T161340Z\n"
        "SUMMARY:test\n"
        "END:VEVENT\n"
        "END:VCALENDAR");
    auto calendar1 = MemoryCalendar::Ptr(new MemoryCalendar(QTimeZone::utc()));
    QVERIFY(format.fromString(calendar1, serializedCalendar1));
    const auto events1 = calendar1->events();
    QCOMPARE(events1.count(), 1);

    const QString serializedCalendar2 = QLatin1String(
        "BEGIN:VCALENDAR\n"
        "VERSION:2.0\n"
        "BEGIN:VEVENT\n"
        "SUMMARY:test\n"
        "DTSTAMP:20201103T161340Z\n"    // Reordered.
        "END:VEVENT\n"
        "END:VCALENDAR");
    auto calendar2 = MemoryCalendar::Ptr(new MemoryCalendar(QTimeZone::utc()));
    QVERIFY(format.fromString(calendar2, serializedCalendar2));
    const auto events2 = calendar2->events();
    QCOMPARE(events2.count(), 1);

    const auto event1 = events1[0];
    const auto event2 = events2[0];
    QCOMPARE(event1->uid(), event2->uid());
}

/**
 * Generated UIDs depend on property names and values.
 */
void ICalFormatTest::testUidGenerationUniqueness()
{
    ICalFormat format;

    const QString serializedCalendar1 = QLatin1String(
        "BEGIN:VCALENDAR\n"
        "VERSION:2.0\n"
        "BEGIN:VEVENT\n"
        "DTSTAMP:20201103T161340Z\n"
        "SUMMARY:test\n"
        "END:VEVENT\n"
        "END:VCALENDAR");
    auto calendar1 = MemoryCalendar::Ptr(new MemoryCalendar(QTimeZone::utc()));
    QVERIFY(format.fromString(calendar1, serializedCalendar1));
    const auto events1 = calendar1->events();
    QCOMPARE(events1.count(), 1);

    const QString serializedCalendar2 = QLatin1String(
        "BEGIN:VCALENDAR\n"
        "VERSION:2.0\n"
        "BEGIN:VEVENT\n"
        "DTSTART:20201103T161340Z\n"    // Property name change.
        "SUMMARY:test\n"
        "END:VEVENT\n"
        "END:VCALENDAR");
    auto calendar2 = MemoryCalendar::Ptr(new MemoryCalendar(QTimeZone::utc()));
    QVERIFY(format.fromString(calendar2, serializedCalendar2));
    const auto events2 = calendar2->events();
    QCOMPARE(events2.count(), 1);

    const QString serializedCalendar3 = QLatin1String(
        "BEGIN:VCALENDAR\n"
        "VERSION:2.0\n"
        "BEGIN:VEVENT\n"
        "DTSTAMP:20201103T161341Z\n"    // Property value changed.
        "SUMMARY:test\n"
        "END:VEVENT\n"
        "END:VCALENDAR");
    auto calendar3 = MemoryCalendar::Ptr(new MemoryCalendar(QTimeZone::utc()));
    QVERIFY(format.fromString(calendar3, serializedCalendar3));
    const auto events3 = calendar3->events();
    QCOMPARE(events3.count(), 1);

    const auto event1 = events1[0];
    const auto event2 = events2[0];
    const auto event3 = events3[0];
    QVERIFY(event1->uid() != event2->uid());
    QVERIFY(event1->uid() != event3->uid());
    QVERIFY(event2->uid() != event3->uid());
}

void ICalFormatTest::testIcalFormat()
{
    ICalFormat format;
    auto duration = format.durationFromString(QStringLiteral("PT2H"));
    QVERIFY(!duration.isNull());
    QCOMPARE(duration.asSeconds(), 7200);
    QCOMPARE(format.toString(duration), QLatin1String("PT2H"));
}

void ICalFormatTest::testNonTextCustomProperties()
{
    const auto input = QLatin1String(
        "BEGIN:VCALENDAR\n"
        "VERSION:2.0\n"
        "BEGIN:VEVENT\n"
        "X-APPLE-TRAVEL-START;ROUTING=CAR;VALUE=URI;X-ADDRESS=Bingerdenallee 1\\n\n"
        " 6921 JN Duiven\\nNederland;X-TITLE=Home:\n"
        "X-APPLE-TRAVEL-DURATION;VALUE=DURATION:PT45M\n"
        "X-APPLE-STRUCTURED-LOCATION;VALUE=URI;X-ADDRESS=Olympus 1\\n3524 WB Utre\n"
        " cht\\nThe Netherlands;X-APPLE-RADIUS=49.91307222863458;X-TITLE=Olympus 1\n"
        " :geo:52.063921,5.128511\n"
        "BEGIN:VALARM\n"
        "TRIGGER;X-APPLE-RELATED-TRAVEL=-PT30M:-PT1H15M\n"
        "END:VALARM\n"
        "END:VEVENT\n"
        "END:VCALENDAR\n");
    ICalFormat format;
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    QVERIFY(format.fromString(cal, input));
    const auto events = cal->events();
    QCOMPARE(events.size(), 1);

    const auto event = events[0];
    QCOMPARE(event->nonKDECustomProperty("X-APPLE-TRAVEL-DURATION"), QLatin1String("PT45M"));
    QCOMPARE(event->nonKDECustomProperty("X-APPLE-TRAVEL-START"), QString());
    QCOMPARE(event->nonKDECustomProperty("X-APPLE-STRUCTURED-LOCATION"), QLatin1String("geo:52.063921,5.128511"));
}

void ICalFormatTest::testAllDaySchedulingMessage()
{
    auto event = KCalendarCore::Event::Ptr::create();
    event->setSummary(QStringLiteral("All Day Event"));
    event->setDtStart(QDateTime(QDate(2023, 10, 13), QTime(0, 0, 0), QTimeZone("Europe/Prague")));
    event->setDtEnd(QDateTime(QDate(2023, 10, 15), QTime(0, 0, 0), QTimeZone("Europe/Prague")));
    event->setOrganizer(Person(QStringLiteral("Dan"), QStringLiteral("dvratil@example.com")));
    event->addAttendee(Attendee(QStringLiteral("Konqi"), QStringLiteral("konqi@example.com")));
    event->setAllDay(true);

    ICalFormat format;
    auto calendar = MemoryCalendar::Ptr::create(QTimeZone::utc());
    const auto itipString = format.createScheduleMessage(event, KCalendarCore::iTIPRequest);
    QVERIFY(!itipString.isEmpty());

    auto scheduleMsg = format.parseScheduleMessage(calendar, itipString);
    QVERIFY(scheduleMsg->error().isEmpty());

    auto parsedEvent = scheduleMsg->event().staticCast<KCalendarCore::Event>();
    QVERIFY(parsedEvent);
    QCOMPARE(parsedEvent->dtStart().date(), event->dtStart().date());
    QCOMPARE(parsedEvent->dtEnd().date(), event->dtEnd().date());
}

void ICalFormatTest::testAllDayRecurringUntil()
{
    const auto input = QLatin1String(
        "BEGIN:VCALENDAR\n"
        "VERSION:2.0\n"
        "BEGIN:VEVENT\n"
        "DTSTAMP:20240327T220619Z\n"
        "CREATED:20240327T191625Z\n"
        "UID:c89d88b9-810f-4275-9155-8eea2345386b\n"
        "SEQUENCE:2\n"
        "LAST-MODIFIED:20240327T220619Z\n"
        "SUMMARY:Test\n"
        "RRULE:FREQ=WEEKLY;UNTIL=20240401;BYDAY=WE\n"
        "EXDATE;VALUE=DATE:20240327\n"
        "DTSTART;VALUE=DATE:20240327\n"
        "DTEND;VALUE=DATE:20240328\n"
        "TRANSP:OPAQUE\n"
        "END:VEVENT\n"
        "END:VCALENDAR\n");
    ICalFormat format;
    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    QVERIFY(format.fromString(cal, input));
    const auto events = cal->events();
    QCOMPARE(events.size(), 1);

    const auto event = events[0];
    auto recurrence = event->recurrence();
    QVERIFY(recurrence);
    QCOMPARE(recurrence->endDate(), QDate(2024, 4, 1));
}

#include "moc_testicalformat.cpp"
