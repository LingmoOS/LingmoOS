/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gendertest.h"
#include "gender.h"
#include "vcardtool_p.h"
#include <QTest>

GenderTest::GenderTest(QObject *parent)
    : QObject(parent)
{
}

GenderTest::~GenderTest()
{
}

void GenderTest::shouldHaveDefaultValue()
{
    KContacts::Gender gender;
    QVERIFY(!gender.isValid());
    QVERIFY(gender.gender().isEmpty());
    QVERIFY(gender.comment().isEmpty());
}

void GenderTest::shouldAssignValue()
{
    const QString genderStr(QStringLiteral("F"));
    KContacts::Gender gender(genderStr);
    const QString commentStr(QStringLiteral("foo"));
    gender.setComment(commentStr);
    QVERIFY(gender.isValid());
    QVERIFY(!gender.gender().isEmpty());
    QCOMPARE(gender.gender(), genderStr);
    QVERIFY(!gender.comment().isEmpty());
    QCOMPARE(gender.comment(), commentStr);
}

void GenderTest::shouldAssignExternal()
{
    KContacts::Gender gender;
    const QString genderStr(QStringLiteral("H"));
    gender.setGender(genderStr);
    QVERIFY(gender.isValid());
    QVERIFY(!gender.gender().isEmpty());
    QCOMPARE(gender.gender(), genderStr);
}

void GenderTest::shouldSerialized()
{
    KContacts::Gender gender;
    KContacts::Gender result;
    const QString genderStr(QStringLiteral("H"));
    gender.setGender(genderStr);
    gender.setComment(QStringLiteral("foo"));

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << gender;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> result;

    QVERIFY(gender == result);
}

void GenderTest::shouldEqualGender()
{
    KContacts::Gender gender;
    KContacts::Gender result;
    const QString genderStr(QStringLiteral("H"));
    gender.setGender(genderStr);
    gender.setComment(QStringLiteral("foo"));

    result = gender;
    QVERIFY(gender == result);
}

void GenderTest::shouldParseGender_data()
{
    QTest::addColumn<QByteArray>("vcarddata");
    QTest::addColumn<QString>("genre");
    QTest::addColumn<QString>("comment");
    QTest::addColumn<bool>("hasGender");

    QByteArray str(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");
    QTest::newRow("nogender") << str << QString() << QString() << false;

    str = QByteArray(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "GENDER:H\n"
        "END:VCARD\n");
    QTest::newRow("hasgenderbutnocomment") << str << QStringLiteral("H") << QString() << true;

    str = QByteArray(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "GENDER:;foo\n"
        "END:VCARD\n");
    QTest::newRow("hasgenderbutnotypebutcomment") << str << QString() << QStringLiteral("foo") << true;

    str = QByteArray(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "GENDER:H;foo\n"
        "END:VCARD\n");
    QTest::newRow("hasgendertypeandcomment") << str << QStringLiteral("H") << QStringLiteral("foo") << true;
}

void GenderTest::shouldParseGender()
{
    QFETCH(QByteArray, vcarddata);
    QFETCH(QString, genre);
    QFETCH(QString, comment);
    QFETCH(bool, hasGender);

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).gender().isValid(), hasGender);
    QCOMPARE(lst.at(0).gender().comment(), comment);
    QCOMPARE(lst.at(0).gender().gender(), genre);
}

QByteArray GenderTest::createCard(const QByteArray &gender)
{
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n");
    if (!gender.isEmpty()) {
        expected += gender + "\r\n";
    }
    expected += QByteArray(
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    return expected;
}

void GenderTest::shouldExportEmptyGender()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected = createCard(QByteArray());
    QCOMPARE(ba, expected);
}

void GenderTest::shouldExportOnlyGenderWithoutCommentGender()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Gender gender;
    gender.setGender(QStringLiteral("H"));
    addr.setGender(gender);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected = createCard(QByteArray("GENDER:H"));
    QCOMPARE(ba, expected);
}

void GenderTest::shouldExportOnlyGenderWithCommentGender()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Gender gender;
    gender.setGender(QStringLiteral("H"));
    gender.setComment(QStringLiteral("comment"));
    addr.setGender(gender);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected = createCard(QByteArray("GENDER:H;comment"));
    QCOMPARE(ba, expected);
}

void GenderTest::shouldExportOnlyGenderWithoutTypeCommentGender()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Gender gender;
    gender.setComment(QStringLiteral("comment"));
    addr.setGender(gender);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected = createCard(QByteArray("GENDER:;comment"));
    QCOMPARE(ba, expected);
}

void GenderTest::shouldNotExportInVcard3()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Gender gender;
    gender.setComment(QStringLiteral("comment"));
    addr.setGender(gender);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

QTEST_MAIN(GenderTest)

#include "moc_gendertest.cpp"
