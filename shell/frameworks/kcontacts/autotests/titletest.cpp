/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "titletest.h"
#include "parametermap_p.h"
#include "title.h"
#include "vcardtool_p.h"
#include <QTest>

TitleTest::TitleTest(QObject *parent)
    : QObject(parent)
{
}

TitleTest::~TitleTest()
{
}

void TitleTest::shouldHaveDefaultValue()
{
    KContacts::Title title;
    QVERIFY(!title.isValid());
    QVERIFY(title.title().isEmpty());
    QVERIFY(title.params().empty());
}

void TitleTest::shouldAssignValue()
{
    const QString lang(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    KContacts::Title title(lang);
    title.setParams(params);
    QVERIFY(title.isValid());
    QVERIFY(!title.title().isEmpty());
    QCOMPARE(title.title(), lang);
    QVERIFY(!title.params().empty());
    QCOMPARE(title.params(), params);
}

void TitleTest::shouldAssignExternal()
{
    KContacts::Title title;
    const QString lang(QStringLiteral("fr"));
    title.setTitle(lang);
    QVERIFY(title.isValid());
    QCOMPARE(title.title(), lang);
}

void TitleTest::shouldSerialized()
{
    KContacts::Title title;
    KContacts::Title result;
    const QString lang(QStringLiteral("fr"));
    title.setTitle(lang);
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    title.setParams(params);

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << title;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> result;

    QVERIFY(title == result);
}

void TitleTest::shouldEqualTitle()
{
    KContacts::Title title;
    KContacts::Title result;
    const QString lang(QStringLiteral("fr"));
    title.setTitle(lang);
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    title.setParams(params);

    result = title;
    QVERIFY(title == result);
}

void TitleTest::shouldParseTwoTitles()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "Title:boo\n"
        "Title:bla\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).extraTitleList().count(), 2);
    QCOMPARE(lst.at(0).title(), QStringLiteral("boo"));
    QCOMPARE(lst.at(0).extraTitleList().at(0).title(), QStringLiteral("boo"));
    QCOMPARE(lst.at(0).extraTitleList().at(1).title(), QStringLiteral("bla"));
}

void TitleTest::shouldParseTitle()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "Title:boo\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).extraTitleList().count(), 1);
    QCOMPARE(lst.at(0).title(), QStringLiteral("boo"));
    QCOMPARE(lst.at(0).extraTitleList().at(0).title(), QStringLiteral("boo"));
}

void TitleTest::shouldParseWithoutTitle()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).extraTitleList().count(), 0);
    QCOMPARE(lst.at(0).title(), QString());
}

void TitleTest::shouldCreateVCard()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Title::List lstTitle;
    KContacts::Title title(QStringLiteral("fr"));
    lstTitle << title;
    addr.setExtraTitleList(lstTitle);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "TITLE:fr\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void TitleTest::shouldCreateVCardWithTwoTitle()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Title::List lstTitle;
    KContacts::Title title(QStringLiteral("fr"));
    KContacts::Title title2(QStringLiteral("fr2"));
    lstTitle << title << title2;
    addr.setExtraTitleList(lstTitle);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "TITLE:fr\r\n"
        "TITLE:fr2\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void TitleTest::shouldCreateVCardWithParameters()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Title::List lstTitle;
    KContacts::Title title(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    title.setParams(params);
    lstTitle << title;
    addr.setExtraTitleList(lstTitle);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "TITLE;FOO1=bla1,blo1;FOO2=bla2,blo2:fr\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

void TitleTest::shouldGenerateTitleForVCard3()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Title::List lstTitle;
    KContacts::Title title(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    title.setParams(params);
    lstTitle << title;
    addr.setExtraTitleList(lstTitle);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "TITLE;FOO1=bla1,blo1;FOO2=bla2,blo2:fr\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

QTEST_MAIN(TitleTest)

#include "moc_titletest.cpp"
