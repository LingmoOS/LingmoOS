/*
  This file is part of the kcalcore library.
  SPDX-FileCopyrightText: 2006, 2008 Allen Winter <winter@kde.org>
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testattendee.h"
#include "attendee.h"
#include "person.h"

#include <QDataStream>
#include <QTest>

QTEST_MAIN(AttendeeTest)

using namespace KCalendarCore;

void AttendeeTest::testValidity()
{
    Attendee attendee(QStringLiteral("fred"), QStringLiteral("fred@flintstone.com"));
    attendee.setRole(Attendee::Chair);
    QVERIFY(attendee.role() == Attendee::Chair);
}

void AttendeeTest::testType()
{
    Attendee attendee(QStringLiteral("fred"), QStringLiteral("fred@flintstone.com"));
    QCOMPARE(attendee.cuType(), Attendee::Individual);
    QCOMPARE(attendee.cuTypeStr(), QLatin1String("INDIVIDUAL"));

    attendee.setCuType(attendee.cuTypeStr());
    QCOMPARE(attendee.cuType(), Attendee::Individual);

    attendee.setCuType(QStringLiteral("INVALID"));
    QCOMPARE(attendee.cuType(), Attendee::Unknown);
    QCOMPARE(attendee.cuTypeStr(), QLatin1String("UNKNOWN"));

    attendee.setCuType(QStringLiteral("group"));
    QCOMPARE(attendee.cuType(), Attendee::Group);
    QCOMPARE(attendee.cuTypeStr(), QLatin1String("GROUP"));

    attendee.setCuType(QStringLiteral("resource"));
    QCOMPARE(attendee.cuType(), Attendee::Resource);
    QCOMPARE(attendee.cuTypeStr(), QLatin1String("RESOURCE"));

    attendee.setCuType(QStringLiteral("ROOM"));
    QCOMPARE(attendee.cuType(), Attendee::Room);
    QCOMPARE(attendee.cuTypeStr(), QLatin1String("ROOM"));

    attendee.setCuType(QStringLiteral("UNKNOWN"));
    QCOMPARE(attendee.cuType(), Attendee::Unknown);
    QCOMPARE(attendee.cuTypeStr(), QLatin1String("UNKNOWN"));

    attendee.setCuType(QStringLiteral("X-test"));
    QCOMPARE(attendee.cuType(), Attendee::Unknown);
    QCOMPARE(attendee.cuTypeStr(), QLatin1String("X-TEST"));

    attendee.setCuType(QStringLiteral("IANA-TEST"));
    QCOMPARE(attendee.cuType(), Attendee::Unknown);
    QCOMPARE(attendee.cuTypeStr(), QLatin1String("IANA-TEST"));

    attendee.setCuType(Attendee::Individual);
    QCOMPARE(attendee.cuType(), Attendee::Individual);

    attendee.setCuType(Attendee::Group);
    QCOMPARE(attendee.cuType(), Attendee::Group);

    attendee.setCuType(Attendee::Resource);
    QCOMPARE(attendee.cuType(), Attendee::Resource);

    attendee.setCuType(Attendee::Room);
    QCOMPARE(attendee.cuType(), Attendee::Room);

    attendee.setCuType(Attendee::Unknown);
    QCOMPARE(attendee.cuType(), Attendee::Unknown);
}

void AttendeeTest::testCompare()
{
    Attendee attendee1(QStringLiteral("fred"), QStringLiteral("fred@flintstone.com"));
    Attendee attendee2(QStringLiteral("wilma"), QStringLiteral("wilma@flintstone.com"));
    attendee1.setRole(Attendee::ReqParticipant);
    attendee2.setRole(Attendee::Chair);
    QVERIFY(!(attendee1 == attendee2));
    attendee2.setRole(Attendee::ReqParticipant);
    QVERIFY(!(attendee1 == attendee2));
    QVERIFY(attendee1.name() == QLatin1String("fred"));
}

void AttendeeTest::testCompareType()
{
    Attendee attendee1(QStringLiteral("fred"), QStringLiteral("fred@flintstone.com"));
    attendee1.setCuType(Attendee::Resource);
    Attendee attendee2 = attendee1;

    QCOMPARE(attendee2.cuType(), Attendee::Resource);
    QVERIFY(attendee1 == attendee2);

    attendee2.setCuType(Attendee::Individual);
    QVERIFY(!(attendee1 == attendee2));
}

void AttendeeTest::testAssign()
{
    Attendee attendee1(QStringLiteral("fred"), QStringLiteral("fred@flintstone.com"));
    Attendee attendee2;
    attendee2 = attendee1;
    QVERIFY(attendee1 == attendee2);

    attendee2.setRole(Attendee::NonParticipant);
    QVERIFY(!(attendee1 == attendee2));
}

void AttendeeTest::testCopyConstructor()
{
    Attendee attendee1(QStringLiteral("fred"), QStringLiteral("fred@flintstone.com"));
    Attendee attendee3(attendee1);
    QVERIFY(attendee3 == attendee1);
}

void AttendeeTest::testDataStreamOut()
{
    Attendee attendee1(QStringLiteral("fred"), QStringLiteral("fred@flintstone.com"));
    attendee1.setRSVP(true);
    attendee1.setRole(Attendee::Chair);
    attendee1.setUid(QStringLiteral("Shooby Doo Bop"));
    attendee1.setDelegate(QStringLiteral("I AM THE Delegate"));
    attendee1.setDelegator(QStringLiteral("AND I AM THE Delegator"));
    attendee1.setCuType(QStringLiteral("X-SPECIAL"));
    attendee1.setCustomProperty("name", QStringLiteral("value"));
    attendee1.setCustomProperty("foo", QStringLiteral("bar"));

    QByteArray byteArray;
    QDataStream out_stream(&byteArray, QIODevice::WriteOnly);

    out_stream << attendee1;

    QDataStream in_stream(&byteArray, QIODevice::ReadOnly);

    Person person;
    bool rsvp;
    QString delegate;
    QString delegator;
    QString cuType;
    QString uid;
    CustomProperties customProperties;
    Attendee::Role role;
    Attendee::PartStat status;
    uint role_int;
    uint status_int;

    in_stream >> person;
    QVERIFY(person.name() == attendee1.name());
    QVERIFY(person.email() == attendee1.email());

    in_stream >> rsvp;
    QVERIFY(rsvp == attendee1.RSVP());

    in_stream >> role_int;
    role = Attendee::Role(role_int);
    QVERIFY(role == attendee1.role());

    in_stream >> status_int;
    status = Attendee::PartStat(status_int);
    QVERIFY(status == attendee1.status());

    in_stream >> uid;
    QVERIFY(uid == attendee1.uid());

    in_stream >> delegate;
    QVERIFY(delegate == attendee1.delegate());

    in_stream >> delegator;
    QVERIFY(delegator == attendee1.delegator());

    in_stream >> cuType;
    QVERIFY(cuType == attendee1.cuTypeStr());

    in_stream >> customProperties;
    QVERIFY(customProperties == attendee1.customProperties());
}

void AttendeeTest::testDataStreamIn()
{
    Attendee attendee1(QStringLiteral("fred"), QStringLiteral("fred@flintstone.com"));
    attendee1.setRSVP(true);
    attendee1.setRole(Attendee::Chair);
    attendee1.setCuType(QStringLiteral("IANA-FOO"));
    attendee1.setUid(QStringLiteral("Shooby Doo Bop"));
    attendee1.setDelegate(QStringLiteral("I AM THE Delegate"));
    attendee1.setDelegator(QStringLiteral("AND I AM THE Delegator"));
    attendee1.setCustomProperty("name", QStringLiteral("value"));
    attendee1.setCustomProperty("foo", QStringLiteral("bar"));
    QVERIFY(!attendee1.isNull());

    QByteArray byteArray;
    QDataStream out_stream(&byteArray, QIODevice::WriteOnly);

    out_stream << attendee1;

    Attendee attendee2;
    QDataStream in_stream(&byteArray, QIODevice::ReadOnly);

    in_stream >> attendee2;

    QVERIFY(!attendee2.isNull());
    QVERIFY(attendee2.uid() == attendee1.uid());
    QVERIFY(attendee2.RSVP() == attendee1.RSVP());
    QVERIFY(attendee2.role() == attendee1.role());
    QVERIFY(attendee2.cuTypeStr() == attendee1.cuTypeStr());
    QVERIFY(attendee2.status() == attendee1.status());
    QVERIFY(attendee2.delegate() == attendee1.delegate());
    QVERIFY(attendee2.delegator() == attendee1.delegator());
    QVERIFY(attendee2.customProperties() == attendee1.customProperties());
    QVERIFY(attendee1 == attendee2);
}

void AttendeeTest::testUid()
{
    Attendee a(QStringLiteral("me"), QStringLiteral("test@dev.null"));
    QVERIFY(!a.uid().isEmpty());
    a.setUid(QStringLiteral("42"));
    QCOMPARE(a.uid(), QLatin1String("42"));
    a.setUid({});
    QVERIFY(!a.uid().isEmpty());
}

#include "moc_testattendee.cpp"
