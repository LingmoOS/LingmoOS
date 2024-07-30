/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "phonenumbertest.h"
#include "kcontacts/phonenumber.h"
#include <QTest>
#include <addressee.h>
#include <addresseelist.h>
#include <vcardtool_p.h>

QTEST_MAIN(PhoneNumberTest)

#ifndef Q_OS_WIN
void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)
#endif

void PhoneNumberTest::emptyTest()
{
    KContacts::PhoneNumber number;

    QVERIFY(number.isEmpty());
}

void PhoneNumberTest::storeTest()
{
    KContacts::PhoneNumber number;

    number.setId(QStringLiteral("My Id"));
    number.setType(KContacts::PhoneNumber::Work | KContacts::PhoneNumber::Cell);
    number.setNumber(QStringLiteral("2734826345"));

    QVERIFY(number.id() == QLatin1String("My Id"));
    QVERIFY(number.type() == (KContacts::PhoneNumber::Work | KContacts::PhoneNumber::Cell));
    QVERIFY(number.number() == QLatin1String("2734826345"));
}

void PhoneNumberTest::equalsTest()
{
    KContacts::PhoneNumber number1;
    KContacts::PhoneNumber number2;

    number1.setId(QStringLiteral("My Id"));
    number1.setType(KContacts::PhoneNumber::Work | KContacts::PhoneNumber::Cell);
    number1.setNumber(QStringLiteral("2734826345"));

    number2.setId(QStringLiteral("My Id"));
    number2.setType(KContacts::PhoneNumber::Work | KContacts::PhoneNumber::Cell);
    number2.setNumber(QStringLiteral("2734826345"));

    QVERIFY(number1 == number2);
}

void PhoneNumberTest::differsTest()
{
    KContacts::PhoneNumber number1(QStringLiteral("123"), KContacts::PhoneNumber::Home);
    KContacts::PhoneNumber number2(QStringLiteral("123"), KContacts::PhoneNumber::Work);

    QVERIFY(number1 != number2);
}

void PhoneNumberTest::assignmentTest()
{
    KContacts::PhoneNumber number1;
    KContacts::PhoneNumber number2;

    number1.setId(QStringLiteral("My Id"));
    number1.setType(KContacts::PhoneNumber::Work | KContacts::PhoneNumber::Cell);
    number1.setNumber(QStringLiteral("2734826345"));

    number1 = number2;

    QVERIFY(number1 == number2);
}

void PhoneNumberTest::serializeTest()
{
    KContacts::PhoneNumber number1;
    KContacts::PhoneNumber number2;

    number1.setId(QStringLiteral("My Id"));
    number1.setType(KContacts::PhoneNumber::Work | KContacts::PhoneNumber::Cell);
    number1.setNumber(QStringLiteral("2734826345"));

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << number1;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> number2;

    QVERIFY(number1 == number2);
}

void PhoneNumberTest::labelTest()
{
    QMap<KContacts::PhoneNumber::Type, QString> labels;

    const KContacts::PhoneNumber::TypeList types = KContacts::PhoneNumber::typeList();

    // check all types standalone
    for (KContacts::PhoneNumber::Type type : types) {
        const KContacts::PhoneNumber phone(QStringLiteral("1"), type);
        QCOMPARE(phone.type(), type);

        // Pref is special cased
        if (type != KContacts::PhoneNumber::Pref) {
            QCOMPARE(phone.typeLabel(), KContacts::PhoneNumber::typeFlagLabel((KContacts::PhoneNumber::TypeFlag)(int)type));
            labels.insert(type, phone.typeLabel());
        } else {
            labels.insert(type, KContacts::PhoneNumber::typeFlagLabel((KContacts::PhoneNumber::TypeFlag)(int)type));
        }
        QCOMPARE(KContacts::PhoneNumber::typeLabel(type), phone.typeLabel());
    }

    // combine all with Pref
    for (KContacts::PhoneNumber::Type type : std::as_const(types)) {
        KContacts::PhoneNumber::Type combinedType = type | KContacts::PhoneNumber::Pref;
        const KContacts::PhoneNumber phone(QLatin1String("1"), combinedType);
        QCOMPARE(phone.type(), combinedType);
        QCOMPARE(KContacts::PhoneNumber::typeLabel(combinedType), phone.typeLabel());

        if (type < KContacts::PhoneNumber::Pref) {
            const QString expectedCombinedString = QStringLiteral("%1/%2").arg(labels[type]).arg(labels[KContacts::PhoneNumber::Pref]);
            QCOMPARE(phone.typeLabel(), expectedCombinedString);
        } else if (type > KContacts::PhoneNumber::Pref) {
            const QString expectedCombinedString = QStringLiteral("%1/%2").arg(labels[KContacts::PhoneNumber::Pref]).arg(labels[type]);
            QCOMPARE(phone.typeLabel(), expectedCombinedString);
        }
    }

    // combine all with Fax
    for (KContacts::PhoneNumber::Type type : std::as_const(types)) {
        KContacts::PhoneNumber::Type combinedType = type | KContacts::PhoneNumber::Fax;
        const KContacts::PhoneNumber phone(QLatin1String("1"), combinedType);
        QCOMPARE(phone.type(), combinedType);
        QCOMPARE(KContacts::PhoneNumber::typeLabel(combinedType), phone.typeLabel());

        if (type == KContacts::PhoneNumber::Home || type == KContacts::PhoneNumber::Work) {
            // special cased
        } else if (type < KContacts::PhoneNumber::Fax) {
            const QString expectedCombinedString = QStringLiteral("%1/%2").arg(labels[type]).arg(labels[KContacts::PhoneNumber::Fax]);
            QCOMPARE(phone.typeLabel(), expectedCombinedString);
        } else if (type > KContacts::PhoneNumber::Fax) {
            const QString expectedCombinedString = QStringLiteral("%1/%2").arg(labels[KContacts::PhoneNumber::Fax]).arg(labels[type]);
            QCOMPARE(phone.typeLabel(), expectedCombinedString);
        }
    }

    // special cases
    QCOMPARE(KContacts::PhoneNumber::typeLabel(KContacts::PhoneNumber::Pref), QLatin1String("Preferred Number"));
    QCOMPARE(KContacts::PhoneNumber::typeLabel(KContacts::PhoneNumber::Home //
                                               | KContacts::PhoneNumber::Fax),
             QLatin1String("Home Fax"));
    QCOMPARE(KContacts::PhoneNumber::typeLabel(KContacts::PhoneNumber::Work //
                                               | KContacts::PhoneNumber::Fax),
             QLatin1String("Work Fax"));
    QCOMPARE(KContacts::PhoneNumber::typeLabel(KContacts::PhoneNumber::Home //
                                               | KContacts::PhoneNumber::Fax //
                                               | KContacts::PhoneNumber::Pref),
             QLatin1String("Home Fax/Preferred"));
    QCOMPARE(KContacts::PhoneNumber::typeLabel(KContacts::PhoneNumber::Work //
                                               | KContacts::PhoneNumber::Fax //
                                               | KContacts::PhoneNumber::Pref),
             QLatin1String("Work Fax/Preferred"));
}

void PhoneNumberTest::shouldParseVCard21()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:2.1\n"
        "EMAIL:foo@kde.org\n"
        "N:;;;;\n"
        "TEL;CELL;WORK:+1-919-676-9564\n"
        "UID:testuid\n"
        "END:VCARD\n"
        "\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    const KContacts::Addressee &addr = lst.at(0);
    QCOMPARE(addr.phoneNumbers().count(), 1);
    KContacts::PhoneNumber number1 = addr.phoneNumbers().at(0);
    QCOMPARE(number1.number(), QLatin1String("+1-919-676-9564"));
    QVERIFY(number1.supportsSms());
    QVERIFY(!number1.isPreferred());
}

void PhoneNumberTest::shouldExportVCard21()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));

    KContacts::PhoneNumber number1;

    number1.setId(QStringLiteral("My Id"));
    number1.setType(KContacts::PhoneNumber::Work | KContacts::PhoneNumber::Cell);
    number1.setNumber(QStringLiteral("+1-919-676-9564"));
    addr.setPhoneNumbers(KContacts::PhoneNumber::List() << number1);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v2_1);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:2.1\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "TEL;CELL;WORK:+1-919-676-9564\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n"
        "\r\n");
    QCOMPARE(ba, expected);
}

void PhoneNumberTest::shouldExportVCard3()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));

    KContacts::PhoneNumber number1;

    number1.setId(QStringLiteral("My Id"));
    number1.setType(KContacts::PhoneNumber::Work | KContacts::PhoneNumber::Cell);
    number1.setNumber(QStringLiteral("+1-919-676-9564"));
    addr.setPhoneNumbers(KContacts::PhoneNumber::List() << number1);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "TEL;TYPE=CELL,WORK:+1-919-676-9564\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n"
        "\r\n");
    QCOMPARE(ba, expected);
}

void PhoneNumberTest::shouldExportVCard4()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));

    KContacts::PhoneNumber number1;

    number1.setId(QStringLiteral("My Id"));
    number1.setType(KContacts::PhoneNumber::Work | KContacts::PhoneNumber::Cell);
    number1.setNumber(QStringLiteral("+1-919-676-9564"));
    addr.setPhoneNumbers(KContacts::PhoneNumber::List() << number1);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "TEL;TYPE=\"cell,work\":+1-919-676-9564\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n"
        "\r\n");
    QCOMPARE(ba, expected);
}

void PhoneNumberTest::shouldParseVcard3()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "TEL;VALUE=uri;PREF=1;TYPE=\"voice,home\":tel:+44-555-555-5555;ext=5555\r\n"
        "TEL;VALUE=uri;TYPE=\"voice,cell,text\":tel:+44-555-555-6666\r\n"
        "TEL;VALUE=uri;TYPE=\"voice,work\":tel:+44-555-555-7777\r\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "LANG:fr\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    const KContacts::Addressee &addr = lst.at(0);
    QCOMPARE(addr.phoneNumbers().count(), 3);
}

void PhoneNumberTest::shouldParseVcard4()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:4.0\n"
        "N:LastName;FirstName;;;\n"
        "TEL;VALUE=uri;PREF=1;TYPE=\"voice,home\":tel:+44-555-555-5555;ext=5555\r\n"
        "TEL;VALUE=uri;TYPE=\"voice,cell,text\":tel:+44-555-555-6666\r\n"
        "TEL;VALUE=uri;TYPE=\"voice,work\":tel:+44-555-555-7777\r\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "LANG:fr\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    const KContacts::Addressee &addr = lst.at(0);
    QCOMPARE(addr.phoneNumbers().count(), 3);
}

void PhoneNumberTest::testNormalizeNumber_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("empty") << QString() << QString();
    QTest::newRow("normalized") << QStringLiteral("+49123456789") << QStringLiteral("+49123456789");
    QTest::newRow("formatted") << QStringLiteral("+49(123) 456 - 789") << QStringLiteral("+49123456789");
}

void PhoneNumberTest::testNormalizeNumber()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);

    KContacts::PhoneNumber num;
    num.setNumber(input);
    QCOMPARE(num.normalizedNumber(), expected);
}

#include "moc_phonenumbertest.cpp"
