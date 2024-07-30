/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "customidentifiertest.h"
#include "addressee.h"
#include "vcardtool_p.h"
#include <QTest>

CustomIdentifierTest::CustomIdentifierTest(QObject *parent)
    : QObject(parent)
{
}

CustomIdentifierTest::~CustomIdentifierTest()
{
}

void CustomIdentifierTest::shouldHaveEmptyCustoms()
{
    KContacts::Addressee addresseeWithMail;
    addresseeWithMail.addEmail(QStringLiteral("foo@bar.org"));
    QVERIFY(!addresseeWithMail.isEmpty());
    QVERIFY(addresseeWithMail.customs().isEmpty());
}

void CustomIdentifierTest::shouldExportVcard3()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org") << QStringLiteral("bla@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    addr.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-SpousesName"), QStringLiteral("foo"));
    lst << addr;
    KContacts::VCardTool vcard;
    QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "EMAIL:bla@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "X-KADDRESSBOOK-X-SpousesName:foo\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);

    addr.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-ANNIVERSARY"), QStringLiteral("19960415"));
    lst.clear();
    lst << addr;
    ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    expected = QByteArray(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "EMAIL:bla@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "X-KADDRESSBOOK-X-ANNIVERSARY:19960415\r\n"
        "X-KADDRESSBOOK-X-SpousesName:foo\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void CustomIdentifierTest::shouldExportVcard4()
{
    // TODO don't work to export custom!
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org") << QStringLiteral("bla@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    addr.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-SpousesName"), QStringLiteral("foo"));
    lst << addr;
    KContacts::VCardTool vcard;
    QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "EMAIL:bla@kde.org\r\n"
        "N:;;;;\r\n"
        "RELATED;TYPE=spouse;VALUE=foo:;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);

    QDate dt(12, 9, 3);
    addr.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-ANNIVERSARY"), dt.toString(Qt::ISODate));
    lst.clear();
    lst << addr;
    ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    expected = QByteArray(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "ANNIVERSARY:00120903\r\n"
        "EMAIL:foo@kde.org\r\n"
        "EMAIL:bla@kde.org\r\n"
        "N:;;;;\r\n"
        "RELATED;TYPE=spouse;VALUE=foo:;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

QTEST_MAIN(CustomIdentifierTest)

#include "moc_customidentifiertest.cpp"
