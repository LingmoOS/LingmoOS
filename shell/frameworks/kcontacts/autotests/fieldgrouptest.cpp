/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fieldgrouptest.h"
#include "fieldgroup.h"
#include "parametermap_p.h"
#include "vcardtool_p.h"
#include <QTest>

FieldGroupTest::FieldGroupTest(QObject *parent)
    : QObject(parent)
{
}

FieldGroupTest::~FieldGroupTest()
{
}

void FieldGroupTest::shouldHaveDefaultValue()
{
    KContacts::FieldGroup fieldgroup;
    QVERIFY(!fieldgroup.isValid());
    QVERIFY(fieldgroup.fieldGroupName().isEmpty());
    QVERIFY(fieldgroup.params().empty());
    QVERIFY(fieldgroup.value().isEmpty());
}

void FieldGroupTest::shouldAssignValue()
{
    const QString fielgroundname(QStringLiteral("fr"));
    const QString value(QStringLiteral("bla"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    KContacts::FieldGroup fieldGroup(fielgroundname);
    fieldGroup.setParams(params);
    fieldGroup.setValue(value);
    QVERIFY(fieldGroup.isValid());
    QVERIFY(!fieldGroup.fieldGroupName().isEmpty());
    QCOMPARE(fieldGroup.fieldGroupName(), fielgroundname);
    QVERIFY(!fieldGroup.params().empty());
    QCOMPARE(fieldGroup.params(), params);
    QCOMPARE(fieldGroup.value(), value);
}

void FieldGroupTest::shouldAssignExternal()
{
    KContacts::FieldGroup fieldgroup;
    const QString fieldgroundname(QStringLiteral("fr"));
    const QString value(QStringLiteral("bla"));
    fieldgroup.setValue(value);
    fieldgroup.setFieldGroupName(fieldgroundname);
    QVERIFY(fieldgroup.isValid());
    QCOMPARE(fieldgroup.fieldGroupName(), fieldgroundname);
    QCOMPARE(fieldgroup.value(), value);
}

void FieldGroupTest::shouldSerialized()
{
    KContacts::FieldGroup fieldGroup;
    KContacts::FieldGroup result;
    const QString lang(QStringLiteral("fr"));
    fieldGroup.setFieldGroupName(lang);
    const QString value(QStringLiteral("bla"));
    fieldGroup.setValue(value);

    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    fieldGroup.setParams(params);

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << fieldGroup;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> result;

    QVERIFY(fieldGroup == result);
}

void FieldGroupTest::shouldEqualFieldGroup()
{
    KContacts::FieldGroup fieldGroup;
    KContacts::FieldGroup result;
    const QString lang(QStringLiteral("fr"));
    const QString value(QStringLiteral("bla"));
    fieldGroup.setValue(value);
    fieldGroup.setFieldGroupName(lang);
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    fieldGroup.setParams(params);

    result = fieldGroup;
    QVERIFY(fieldGroup == result);
}

void FieldGroupTest::shouldParseFieldGroup()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "fr.foo:bla\n"
        "fr2.foo:bla\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).fieldGroupList().count(), 2);
}

void FieldGroupTest::shouldParseWithoutFieldGroup()
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
    QCOMPARE(lst.at(0).fieldGroupList().count(), 0);
}

void FieldGroupTest::shouldCreateVCard()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::FieldGroup::List lstFieldGroup;
    KContacts::FieldGroup fieldGroup(QStringLiteral("fr"));
    const QString value(QStringLiteral("bla"));
    fieldGroup.setValue(value);
    lstFieldGroup << fieldGroup;
    addr.setFieldGroupList(lstFieldGroup);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "fr:bla\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void FieldGroupTest::shouldCreateVCardWithTwoLang()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::FieldGroup::List lstFieldGroup;
    KContacts::FieldGroup fieldGroup1(QStringLiteral("fr"));
    const QString value(QStringLiteral("bla"));
    fieldGroup1.setValue(value);

    KContacts::FieldGroup fieldGroup2(QStringLiteral("fr2"));
    fieldGroup2.setValue(value);
    lstFieldGroup << fieldGroup1 << fieldGroup2;
    addr.setFieldGroupList(lstFieldGroup);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "fr:bla\r\n"
        "fr2:bla\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void FieldGroupTest::shouldCreateVCardWithParameters()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    const QString value(QStringLiteral("bla"));
    KContacts::FieldGroup::List lstFieldGroup;
    KContacts::FieldGroup fieldGroup(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    fieldGroup.setParams(params);
    fieldGroup.setValue(value);
    lstFieldGroup << fieldGroup;
    addr.setFieldGroupList(lstFieldGroup);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "fr;FOO1=bla1,blo1;FOO2=bla2,blo2:bla\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

void FieldGroupTest::shouldNotGenerateFieldGroupForVCard3()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::FieldGroup::List lstFieldGroup;
    KContacts::FieldGroup fieldGroup(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    fieldGroup.setParams(params);
    const QString value(QStringLiteral("bla"));
    fieldGroup.setValue(value);
    lstFieldGroup << fieldGroup;
    addr.setFieldGroupList(lstFieldGroup);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "fr;FOO1=bla1,blo1;FOO2=bla2,blo2:bla\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

QTEST_MAIN(FieldGroupTest)

#include "moc_fieldgrouptest.cpp"
