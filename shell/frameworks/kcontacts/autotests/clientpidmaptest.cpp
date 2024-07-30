/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "clientpidmaptest.h"
#include "clientpidmap.h"
#include "parametermap_p.h"
#include "vcardtool_p.h"
#include <QTest>

ClientPidMapTest::ClientPidMapTest(QObject *parent)
    : QObject(parent)
{
}

ClientPidMapTest::~ClientPidMapTest()
{
}

void ClientPidMapTest::shouldHaveDefaultValue()
{
    KContacts::ClientPidMap role;
    QVERIFY(!role.isValid());
    QVERIFY(role.clientPidMap().isEmpty());
    QVERIFY(role.params().empty());
}

void ClientPidMapTest::shouldAssignValue()
{
    const QString lang(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    KContacts::ClientPidMap role(lang);
    role.setParams(params);
    QVERIFY(role.isValid());
    QVERIFY(!role.clientPidMap().isEmpty());
    QCOMPARE(role.clientPidMap(), lang);
    QVERIFY(!role.params().empty());
    QCOMPARE(role.params(), params);
}

void ClientPidMapTest::shouldAssignExternal()
{
    KContacts::ClientPidMap role;
    const QString lang(QStringLiteral("fr"));
    role.setClientPidMap(lang);
    QVERIFY(role.isValid());
    QCOMPARE(role.clientPidMap(), lang);
}

void ClientPidMapTest::shouldSerialized()
{
    KContacts::ClientPidMap role;
    KContacts::ClientPidMap result;
    const QString lang(QStringLiteral("fr"));
    role.setClientPidMap(lang);
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    role.setParams(params);

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << role;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> result;

    QVERIFY(role == result);
}

void ClientPidMapTest::shouldEqualClientPidMap()
{
    KContacts::ClientPidMap role;
    KContacts::ClientPidMap result;
    const QString lang(QStringLiteral("fr"));
    role.setClientPidMap(lang);
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    role.setParams(params);

    result = role;
    QVERIFY(role == result);
}

void ClientPidMapTest::shouldParseClientPidMap()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "CLIENTPIDMAP:boo\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).clientPidMapList().count(), 1);
    QCOMPARE(lst.at(0).clientPidMapList().at(0).clientPidMap(), QStringLiteral("boo"));
}

void ClientPidMapTest::shouldParseWithoutClientPidMap()
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
    QCOMPARE(lst.at(0).clientPidMapList().count(), 0);
}

void ClientPidMapTest::shouldCreateVCard()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::ClientPidMap::List lstClientPidMap;
    KContacts::ClientPidMap clientpidmap(QStringLiteral("fr"));
    lstClientPidMap << clientpidmap;
    addr.setClientPidMapList(lstClientPidMap);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "CLIENTPIDMAP:fr\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void ClientPidMapTest::shouldCreateVCardWithTwoClientPidMap()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::ClientPidMap::List lstClientPidMap;
    KContacts::ClientPidMap clientpidmap(QStringLiteral("fr"));
    KContacts::ClientPidMap clientpidmap2(QStringLiteral("fr2"));
    lstClientPidMap << clientpidmap << clientpidmap2;
    addr.setClientPidMapList(lstClientPidMap);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "CLIENTPIDMAP:fr\r\n"
        "CLIENTPIDMAP:fr2\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void ClientPidMapTest::shouldCreateVCardWithParameters()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::ClientPidMap::List lstClientPidMap;
    KContacts::ClientPidMap clientpidmap(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    clientpidmap.setParams(params);
    lstClientPidMap << clientpidmap;
    addr.setClientPidMapList(lstClientPidMap);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "CLIENTPIDMAP;FOO1=bla1,blo1;FOO2=bla2,blo2:fr\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

void ClientPidMapTest::shouldGenerateClientPidMapForVCard3()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::ClientPidMap::List lstClientPidMap;
    KContacts::ClientPidMap clientpidmap(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    clientpidmap.setParams(params);
    lstClientPidMap << clientpidmap;
    addr.setClientPidMapList(lstClientPidMap);
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

QTEST_MAIN(ClientPidMapTest)

#include "moc_clientpidmaptest.cpp"
