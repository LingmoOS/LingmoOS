/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "roletest.h"
#include "parametermap_p.h"
#include "role.h"
#include "vcardtool_p.h"
#include <QTest>

RoleTest::RoleTest(QObject *parent)
    : QObject(parent)
{
}

RoleTest::~RoleTest()
{
}

void RoleTest::shouldHaveDefaultValue()
{
    KContacts::Role role;
    QVERIFY(!role.isValid());
    QVERIFY(role.role().isEmpty());
    QVERIFY(role.params().empty());
}

void RoleTest::shouldAssignValue()
{
    const QString lang(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    KContacts::Role role(lang);
    role.setParams(params);
    QVERIFY(role.isValid());
    QVERIFY(!role.role().isEmpty());
    QCOMPARE(role.role(), lang);
    QVERIFY(!role.params().empty());
    QCOMPARE(role.params(), params);
}

void RoleTest::shouldAssignExternal()
{
    KContacts::Role role;
    const QString lang(QStringLiteral("fr"));
    role.setRole(lang);
    QVERIFY(role.isValid());
    QCOMPARE(role.role(), lang);
}

void RoleTest::shouldSerialized()
{
    KContacts::Role role;
    KContacts::Role result;
    const QString lang(QStringLiteral("fr"));
    role.setRole(lang);
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

void RoleTest::shouldEqualRole()
{
    KContacts::Role role;
    KContacts::Role result;
    const QString lang(QStringLiteral("fr"));
    role.setRole(lang);
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    role.setParams(params);

    result = role;
    QVERIFY(role == result);
}

void RoleTest::shouldParseRole()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "Role:boo\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).extraRoleList().count(), 1);
    QCOMPARE(lst.at(0).role(), QStringLiteral("boo"));
    QCOMPARE(lst.at(0).extraRoleList().at(0).role(), QStringLiteral("boo"));
}

void RoleTest::shouldParseWithoutRole()
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
    QCOMPARE(lst.at(0).extraRoleList().count(), 0);
    QCOMPARE(lst.at(0).role(), QString());
}

void RoleTest::shouldCreateVCard()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Role::List lstRole;
    KContacts::Role role(QStringLiteral("fr"));
    lstRole << role;
    addr.setExtraRoleList(lstRole);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "ROLE:fr\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void RoleTest::shouldCreateVCardWithTwoRole()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Role::List lstRole;
    KContacts::Role role(QStringLiteral("fr"));
    KContacts::Role role2(QStringLiteral("fr2"));
    lstRole << role << role2;
    addr.setExtraRoleList(lstRole);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "ROLE:fr\r\n"
        "ROLE:fr2\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void RoleTest::shouldCreateVCardWithParameters()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Role::List lstRole;
    KContacts::Role role(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    role.setParams(params);
    lstRole << role;
    addr.setExtraRoleList(lstRole);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "ROLE;FOO1=bla1,blo1;FOO2=bla2,blo2:fr\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

void RoleTest::shouldGenerateRoleForVCard3()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Role::List lstRole;
    KContacts::Role role(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    role.setParams(params);
    lstRole << role;
    addr.setExtraRoleList(lstRole);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "ROLE;FOO1=bla1,blo1;FOO2=bla2,blo2:fr\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

QTEST_MAIN(RoleTest)

#include "moc_roletest.cpp"
