/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "timezonetest.h"
#include "kcontacts/timezone.h"
#include "vcardtool_p.h"
#include <QTest>

QTEST_MAIN(TimeZoneTest)

void TimeZoneTest::emptyTest()
{
    KContacts::TimeZone timezone;

    QVERIFY(timezone.isValid() == false);
}

void TimeZoneTest::storeTest()
{
    KContacts::TimeZone timezone;

    timezone.setOffset(2);

    QVERIFY(timezone.offset() == 2);
}

void TimeZoneTest::equalsTest()
{
    KContacts::TimeZone timezone1;
    KContacts::TimeZone timezone2;

    timezone1.setOffset(2);
    timezone2.setOffset(2);

    QVERIFY(timezone1 == timezone2);
}

void TimeZoneTest::differsTest()
{
    KContacts::TimeZone timezone1(2);
    KContacts::TimeZone timezone2(3);

    QVERIFY(timezone1 != timezone2);
    QVERIFY(timezone1 != KContacts::TimeZone());
}

void TimeZoneTest::assignmentTest()
{
    KContacts::TimeZone timezone1;
    KContacts::TimeZone timezone2;

    timezone1.setOffset(2);
    timezone1 = timezone2;

    QVERIFY(timezone1 == timezone2);
}

void TimeZoneTest::serializeTest()
{
    KContacts::TimeZone timezone1;
    KContacts::TimeZone timezone2;

    timezone1.setOffset(2);

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << timezone1;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> timezone2;

    QVERIFY(timezone1 == timezone2);
}

void TimeZoneTest::shouldGenerateVCard3()
{
    KContacts::Addressee::List lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::TimeZone timezone;

    timezone.setOffset(2);
    addr.setTimeZone(timezone);

    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected;
    expected = QByteArray(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "TZ:+00:02\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

void TimeZoneTest::shouldGenerateVCard4()
{
    // TODO fixme.
    KContacts::Addressee::List lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::TimeZone timezone;

    timezone.setOffset(2);
    addr.setTimeZone(timezone);

    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    const QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "TZ:+00:02\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

#include "moc_timezonetest.cpp"
