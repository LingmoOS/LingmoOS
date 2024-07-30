/*
    This file is part of kcontacts.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "birthdaytest.h"
#include "addressee.h"
#include "vcard_p.h"
#include <QTest>
#include <vcardtool_p.h>

BirthDayTest::BirthDayTest(QObject *parent)
    : QObject(parent)
{
}

BirthDayTest::~BirthDayTest()
{
}

void BirthDayTest::shouldParseBirthDay()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "BDAY:19760505T120505\r\n"
        "EMAIL:foo@kde.org\r\n"
        "EMAIL:bla@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QDateTime dt(QDate(1976, 5, 5), QTime(12, 5, 5));
    QCOMPARE(lst.at(0).birthday(), dt);
    QCOMPARE(lst.at(0).birthdayHasTime(), true);
}

void BirthDayTest::shouldParseBirthDayWithoutTime()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "BDAY:19760505\r\n"
        "EMAIL:foo@kde.org\r\n"
        "EMAIL:bla@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QDateTime dt(QDate(1976, 5, 5).startOfDay());

    QCOMPARE(lst.at(0).birthday(), dt);
    QCOMPARE(lst.at(0).birthdayHasTime(), false);
}

void BirthDayTest::shouldParseBirthDayWithoutTimeAndYear()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "BDAY:--0505\r\n"
        "EMAIL:foo@kde.org\r\n"
        "EMAIL:bla@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QDateTime dt(QDate(-1, 5, 5).startOfDay());
    QCOMPARE(lst.at(0).birthday(), dt);
    QCOMPARE(lst.at(0).birthdayHasTime(), false);
}

void BirthDayTest::shouldExportVcard4()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org") << QStringLiteral("bla@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    const QDateTime dt(QDate(1976, 5, 5), QTime(12, 5, 5));
    addr.setBirthday(dt);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "BDAY:19760505T120505\r\n"
        "EMAIL:foo@kde.org\r\n"
        "EMAIL:bla@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void BirthDayTest::shouldExportVcard4WithoutTime()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org") << QStringLiteral("bla@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    const QDate d(1976, 5, 5);
    addr.setBirthday(d);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "BDAY:19760505\r\n"
        "EMAIL:foo@kde.org\r\n"
        "EMAIL:bla@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void BirthDayTest::shouldExportVcard4WithoutTimeAndWithoutYear()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org") << QStringLiteral("bla@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    const QDate d(-1, 5, 5);
    addr.setBirthday(d);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "BDAY:--0505\r\n"
        "EMAIL:foo@kde.org\r\n"
        "EMAIL:bla@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void BirthDayTest::shouldExportVcard3()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org") << QStringLiteral("bla@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    const QDateTime dt(QDate(1976, 5, 5), QTime(12, 5, 5));
    addr.setBirthday(dt);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "BDAY:1976-05-05T12:05:05\r\n"
        "EMAIL:foo@kde.org\r\n"
        "EMAIL:bla@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void BirthDayTest::shouldExportVcard3WithoutTimeAndWithoutYear()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org") << QStringLiteral("bla@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    const QDate d(-1, 5, 5);
    addr.setBirthday(d);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "BDAY:--05-05\r\n"
        "EMAIL:foo@kde.org\r\n"
        "EMAIL:bla@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void BirthDayTest::shouldExportVcard3WithoutTime()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org") << QStringLiteral("bla@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    const QDate d(1976, 5, 5);
    addr.setBirthday(d);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "BDAY:1976-05-05\r\n"
        "EMAIL:foo@kde.org\r\n"
        "EMAIL:bla@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

QTEST_MAIN(BirthDayTest)

#include "moc_birthdaytest.cpp"
