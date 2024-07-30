/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "keytest.h"
#include "kcontacts/key.h"
#include "vcardtool_p.h"
#include <QTest>

QTEST_MAIN(KeyTest)

void KeyTest::storeTest()
{
    KContacts::Key key;

    key.setId(QStringLiteral("My Id"));
    key.setType(KContacts::Key::Custom);
    key.setCustomTypeString(QStringLiteral("GnuPG"));
    key.setTextData(QStringLiteral("That's my super secret key"));

    QVERIFY(key.id() == QLatin1String("My Id"));
    QVERIFY(key.type() == KContacts::Key::Custom);
    QVERIFY(key.customTypeString() == QLatin1String("GnuPG"));
    QVERIFY(key.textData() == QLatin1String("That's my super secret key"));
    QVERIFY(key.isBinary() == false);
}

void KeyTest::equalsTest()
{
    KContacts::Key key1;
    KContacts::Key key2;

    key1.setId(QStringLiteral("My Id"));
    key1.setType(KContacts::Key::Custom);
    key1.setCustomTypeString(QStringLiteral("GnuPG"));
    key1.setTextData(QStringLiteral("That's my super secret key"));

    key2.setId(QStringLiteral("My Id"));
    key2.setType(KContacts::Key::Custom);
    key2.setCustomTypeString(QStringLiteral("GnuPG"));
    key2.setTextData(QStringLiteral("That's my super secret key"));

    QVERIFY(key1 == key2);
}

void KeyTest::differsTest()
{
    KContacts::Key key1(QStringLiteral("TextKey"), KContacts::Key::PGP);
    KContacts::Key key2(QStringLiteral("TextKey"), KContacts::Key::Custom);

    QVERIFY(key1 != key2);
}

void KeyTest::assignmentTest()
{
    KContacts::Key key1;
    KContacts::Key key2;

    key1.setId(QStringLiteral("My Id"));
    key1.setType(KContacts::Key::Custom);
    key1.setCustomTypeString(QStringLiteral("GnuPG"));
    key1.setTextData(QStringLiteral("That's my super secret key"));

    key2 = key1;

    QVERIFY(key1 == key2);
}

void KeyTest::serializeTest()
{
    KContacts::Key key1;
    KContacts::Key key2;

    key1.setId(QStringLiteral("My Id"));
    key1.setType(KContacts::Key::Custom);
    key1.setCustomTypeString(QStringLiteral("GnuPG"));
    key1.setTextData(QStringLiteral("That's my super secret key"));

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << key1;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> key2;

    QVERIFY(key1 == key2);
}

void KeyTest::shouldExportVCard3()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Key key1(QStringLiteral("https://foo.org/sherlock-holmes.pub.asc"), KContacts::Key::PGP);
    addr.setKeys(KContacts::Key::List{key1});
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "KEY;TYPE=PGP:https://foo.org/sherlock-holmes.pub.asc\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void KeyTest::shouldExportVCard4()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Key key1(QStringLiteral("https://foo.org/sherlock-holmes.pub.asc"), KContacts::Key::PGP);
    addr.setKeys(KContacts::Key::List{key1});
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "KEY;MEDIATYPE=application/pgp-keys:https://foo.org/sherlock-holmes.pub.asc\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void KeyTest::shouldParseVcard3()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "KEY;TYPE=PGP:https://foo.org/sherlock-holmes.pub.asc\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QVERIFY(!lst.at(0).keys().isEmpty());
    QCOMPARE(lst.at(0).keys().count(), 1);

    KContacts::Key key = lst.at(0).keys().at(0);
    QCOMPARE(key.type(), KContacts::Key::PGP);
    QCOMPARE(key.textData(), QStringLiteral("https://foo.org/sherlock-holmes.pub.asc"));
}

void KeyTest::shouldParseVcard4()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "KEY;MEDIATYPE=application/pgp-keys:https://foo.org/sherlock-holmes.pub.asc\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QVERIFY(!lst.at(0).keys().isEmpty());
    QCOMPARE(lst.at(0).keys().count(), 1);

    KContacts::Key key = lst.at(0).keys().at(0);
    QCOMPARE(key.type(), KContacts::Key::PGP);
    QCOMPARE(key.textData(), QStringLiteral("https://foo.org/sherlock-holmes.pub.asc"));
}

#include "moc_keytest.cpp"
