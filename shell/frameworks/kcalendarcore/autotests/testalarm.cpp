/*
  This file is part of the kcalcore library.
  SPDX-FileCopyrightText: 2006 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testalarm.h"
#include "alarm.h"
#include "event.h"

#include <QTest>
QTEST_MAIN(AlarmTest)

using namespace KCalendarCore;

void AlarmTest::testValidity()
{
    Event inc;
    Alarm alarm(&inc);
    alarm.setType(Alarm::Email);
    QVERIFY(alarm.type() == Alarm::Email);
}

void AlarmTest::testCompare()
{
    Event inc1;
    Event inc2;
    Alarm alarm1(&inc1);
    Alarm alarm2(&inc2);
    alarm1.setType(Alarm::Email);
    alarm2.setType(Alarm::Email);

    alarm1.setMailAddress(Person(QStringLiteral("name"), QStringLiteral("email@foo.com")));
    alarm2.setMailAddress(Person(QStringLiteral("name"), QStringLiteral("email@foo.com")));

    QVERIFY(alarm1 == alarm2);

    alarm2.setMailAddress(Person(QStringLiteral("name"), QStringLiteral("email@foo.pt")));
    QVERIFY(alarm1 != alarm2);

    alarm2.setType(Alarm::Display);
    QVERIFY(alarm1 != alarm2);
}

void AlarmTest::testAssignment()
{
    Alarm alarm1(nullptr);
    alarm1.setType(Alarm::Display);
    Alarm alarm2(nullptr);
    QCOMPARE(alarm2.type(), Alarm::Invalid);
    alarm2 = alarm1;
    QVERIFY(alarm1 == alarm2);
}

void AlarmTest::testCopyConstructor()
{
    Alarm alarm1(nullptr);
    alarm1.setType(Alarm::Display);
    Alarm alarm3 {alarm1};
    QVERIFY(alarm1 == alarm3);
}

void AlarmTest::testSerializer_data()
{
    QTest::addColumn<KCalendarCore::Alarm::Ptr>("alarm");
    Alarm::Ptr a1 = Alarm::Ptr(new Alarm(nullptr));
    Alarm::Ptr a2 = Alarm::Ptr(new Alarm(nullptr));
    Alarm::Ptr a3 = Alarm::Ptr(new Alarm(nullptr));
    Alarm::Ptr a4 = Alarm::Ptr(new Alarm(nullptr));

    a1->setType(Alarm::Email);
    a2->setType(Alarm::Procedure);
    a3->setType(Alarm::Display);
    a4->setType(Alarm::Audio);

    a3->setDisplayAlarm(QStringLiteral("foo"));
    a3->setText(QStringLiteral("foo bar"));
    a4->setAudioFile(QStringLiteral("file.mp3"));
    a2->setProgramFile(QStringLiteral("/usr/bin/foo"));
    a2->setProgramArguments(QStringLiteral("--play"));

    a1->setMailSubject(QStringLiteral("empty subject"));

    Person::List persons;
    persons << Person(QStringLiteral("a"), QStringLiteral("a@a.pt")) << Person(QStringLiteral("b"), QStringLiteral("b@b.pt"));
    a1->setMailAddresses(persons);
    a1->setMailAttachment(QStringLiteral("foo attachment"));
    a1->setMailText(QStringLiteral("mail body"));

    a1->setTime(QDateTime(QDate(2006, 8, 3), QTime(8, 0, 0), QTimeZone::UTC));
    a2->setStartOffset(Duration(7, Duration::Days));
    a3->setEndOffset(Duration(1, Duration::Days));

    a1->setSnoozeTime(Duration(1, Duration::Seconds));
    a1->setRepeatCount(50);
    a1->setEnabled(true);
    a2->setEnabled(true);
    a3->setHasLocationRadius(false);
    a3->setLocationRadius(100);

    QTest::newRow("alarm1") << a1;
    QTest::newRow("alarm2") << a2;
    QTest::newRow("alarm3") << a3;
    QTest::newRow("alarm4") << a4;
}

void AlarmTest::testSerializer()
{
    QFETCH(KCalendarCore::Alarm::Ptr, alarm);

    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << alarm; // Serialize

    Alarm::Ptr alarm2 = Alarm::Ptr(new Alarm(nullptr));
    QDataStream stream2(&array, QIODevice::ReadOnly);
    stream2 >> alarm2; // deserialize
    QVERIFY(*alarm == *alarm2);
}

#include "moc_testalarm.cpp"
