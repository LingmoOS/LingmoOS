/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "nicknametest.h"
#include "nickname.h"
#include "parametermap_p.h"
#include "vcardtool_p.h"
#include <QTest>

NickNameTest::NickNameTest(QObject *parent)
    : QObject(parent)
{
}

NickNameTest::~NickNameTest()
{
}

void NickNameTest::shouldHaveDefaultValue()
{
    KContacts::NickName nickname;
    QVERIFY(!nickname.isValid());
    QVERIFY(nickname.nickname().isEmpty());
    QVERIFY(nickname.params().empty());
}

void NickNameTest::shouldAssignValue()
{
    const QString lang(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    KContacts::NickName nickname(lang);
    nickname.setParams(params);
    QVERIFY(nickname.isValid());
    QVERIFY(!nickname.nickname().isEmpty());
    QCOMPARE(nickname.nickname(), lang);
    QVERIFY(!nickname.params().empty());
    QCOMPARE(nickname.params(), params);
}

void NickNameTest::shouldAssignExternal()
{
    KContacts::NickName nickname;
    const QString lang(QStringLiteral("fr"));
    nickname.setNickName(lang);
    QVERIFY(nickname.isValid());
    QCOMPARE(nickname.nickname(), lang);
}

void NickNameTest::shouldSerialized()
{
    KContacts::NickName nickname;
    KContacts::NickName result;
    const QString lang(QStringLiteral("fr"));
    nickname.setNickName(lang);
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    nickname.setParams(params);

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << nickname;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> result;

    QVERIFY(nickname == result);
}

void NickNameTest::shouldEqualNickName()
{
    KContacts::NickName nickname;
    KContacts::NickName result;
    const QString lang(QStringLiteral("fr"));
    nickname.setNickName(lang);
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    nickname.setParams(params);

    result = nickname;
    QVERIFY(nickname == result);
}

void NickNameTest::shouldParseNickName()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "NickName:boo\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).extraNickNameList().count(), 1);
    QCOMPARE(lst.at(0).extraNickNameList().at(0).nickname(), QStringLiteral("boo"));
    QCOMPARE(lst.at(0).nickName(), QStringLiteral("boo"));
}

void NickNameTest::shouldParseWithoutNickName()
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
    QCOMPARE(lst.at(0).extraNickNameList().count(), 0);
    QCOMPARE(lst.at(0).nickName(), QString());
}

void NickNameTest::shouldCreateVCard()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::NickName::List lstNickName;
    KContacts::NickName title(QStringLiteral("blo"));
    lstNickName << title;
    addr.setExtraNickNameList(lstNickName);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "NICKNAME:blo\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void NickNameTest::shouldCreateVCardWithTwoNickName()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::NickName::List lstNickName;
    KContacts::NickName title(QStringLiteral("fr"));
    KContacts::NickName title2(QStringLiteral("fr2"));
    lstNickName << title << title2;
    addr.setExtraNickNameList(lstNickName);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "NICKNAME:fr\r\n"
        "NICKNAME:fr2\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void NickNameTest::shouldCreateVCardWithParameters()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::NickName::List lstNickName;
    KContacts::NickName title(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    title.setParams(params);
    lstNickName << title;
    addr.setExtraNickNameList(lstNickName);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "NICKNAME;FOO1=bla1,blo1;FOO2=bla2,blo2:fr\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

void NickNameTest::shouldGenerateNickNameForVCard3()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::NickName::List lstNickName;
    KContacts::NickName title(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    title.setParams(params);
    lstNickName << title;
    addr.setExtraNickNameList(lstNickName);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "NICKNAME;FOO1=bla1,blo1;FOO2=bla2,blo2:fr\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

QTEST_MAIN(NickNameTest)

#include "moc_nicknametest.cpp"
