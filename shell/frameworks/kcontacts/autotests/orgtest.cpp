/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "orgtest.h"
#include "org.h"
#include "parametermap_p.h"
#include "vcardtool_p.h"
#include <QTest>

OrgTest::OrgTest(QObject *parent)
    : QObject(parent)
{
}

OrgTest::~OrgTest()
{
}

void OrgTest::shouldHaveDefaultValue()
{
    KContacts::Org org;
    QVERIFY(!org.isValid());
    QVERIFY(org.organization().isEmpty());
    QVERIFY(org.params().empty());
}

void OrgTest::shouldAssignValue()
{
    const QString organization(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    KContacts::Org org(organization);
    org.setParams(params);
    QVERIFY(org.isValid());
    QVERIFY(!org.organization().isEmpty());
    QCOMPARE(org.organization(), organization);
    QVERIFY(!org.params().empty());
    QCOMPARE(org.params(), params);
}

void OrgTest::shouldAssignExternal()
{
    KContacts::Org org;
    const QString organization(QStringLiteral("fr"));
    org.setOrganization(organization);
    QVERIFY(org.isValid());
    QCOMPARE(org.organization(), organization);
}

void OrgTest::shouldSerialized()
{
    KContacts::Org org;
    KContacts::Org result;
    const QString organization(QStringLiteral("fr"));
    org.setOrganization(organization);
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    org.setParams(params);

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << org;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> result;

    QVERIFY(org == result);
}

void OrgTest::shouldEqualOrg()
{
    KContacts::Org org;
    KContacts::Org result;
    const QString organization(QStringLiteral("fr"));
    org.setOrganization(organization);
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    org.setParams(params);

    result = org;
    QVERIFY(org == result);
}

void OrgTest::shouldParseOrg()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "Org:boo\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).extraOrganizationList().count(), 1);
    QCOMPARE(lst.at(0).organization(), QStringLiteral("boo"));
    QCOMPARE(lst.at(0).extraOrganizationList().at(0).organization(), QStringLiteral("boo"));
}

void OrgTest::shouldParseVcardWithTwoOrg()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "Org:boo\n"
        "Org:bla\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).extraOrganizationList().count(), 2);
    QCOMPARE(lst.at(0).extraOrganizationList().at(0).organization(), QStringLiteral("boo"));
    QCOMPARE(lst.at(0).extraOrganizationList().at(1).organization(), QStringLiteral("bla"));
}

void OrgTest::shouldCreateVCardWithSemiColon()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Org::List lstOrg;
    KContacts::Org org(QStringLiteral("fr;bla"));
    lstOrg << org;
    addr.setExtraOrganizationList(lstOrg);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "ORG:fr\\\\;bla\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void OrgTest::shouldParseWithoutOrg()
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
    QCOMPARE(lst.at(0).extraOrganizationList().count(), 0);
}

void OrgTest::shouldCreateVCard()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Org::List lstOrg;
    KContacts::Org org(QStringLiteral("fr"));
    lstOrg << org;
    addr.setExtraOrganizationList(lstOrg);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "ORG:fr\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void OrgTest::shouldCreateVCardWithTwoOrg()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Org::List lstOrg;
    KContacts::Org org(QStringLiteral("fr"));
    KContacts::Org org2(QStringLiteral("fr2"));
    lstOrg << org << org2;
    addr.setExtraOrganizationList(lstOrg);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "ORG:fr\r\n"
        "ORG:fr2\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void OrgTest::shouldCreateVCardWithParameters()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Org::List lstOrg;
    KContacts::Org org(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    org.setParams(params);
    lstOrg << org;
    addr.setExtraOrganizationList(lstOrg);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "ORG;FOO1=bla1,blo1;FOO2=bla2,blo2:fr\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

void OrgTest::shouldGenerateOrgForVCard3()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Org::List lstOrg;
    KContacts::Org org(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    org.setParams(params);
    lstOrg << org;
    addr.setExtraOrganizationList(lstOrg);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "ORG;FOO1=bla1,blo1;FOO2=bla2,blo2:fr\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

QTEST_MAIN(OrgTest)

#include "moc_orgtest.cpp"
