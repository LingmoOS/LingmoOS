/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "relatedtest.h"

#include "parametermap_p.h"
#include "related.h"
#include "vcardtool_p.h"

#include <QTest>

RelatedTest::RelatedTest(QObject *parent)
    : QObject(parent)
{
}

RelatedTest::~RelatedTest()
{
}

void RelatedTest::shouldHaveDefaultValue()
{
    KContacts::Related related;
    QVERIFY(!related.isValid());
    QVERIFY(related.related().isEmpty());
    QVERIFY(related.params().empty());
}

void RelatedTest::shouldAssignValue()
{
    const QString relatedTo(QStringLiteral("friend"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    KContacts::Related related(relatedTo);
    related.setParams(params);
    QVERIFY(related.isValid());
    QVERIFY(!related.related().isEmpty());
    QCOMPARE(related.related(), relatedTo);
    QVERIFY(!related.params().empty());
    QCOMPARE(related.params(), params);
}

void RelatedTest::shouldAssignExternal()
{
    KContacts::Related related;
    const QString relatedTo(QStringLiteral("friend"));
    related.setRelated(relatedTo);
    QVERIFY(related.isValid());
    QCOMPARE(related.related(), relatedTo);
}

void RelatedTest::shouldSerialized()
{
    KContacts::Related related;
    KContacts::Related result;
    const QString relatedTo(QStringLiteral("friend"));
    related.setRelated(relatedTo);
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    related.setParams(params);

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << related;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> result;

    QVERIFY(related == result);
}

void RelatedTest::shouldEqualRelated()
{
    KContacts::Related related;
    KContacts::Related result;
    const QString relatedTo(QStringLiteral("friend"));
    related.setRelated(relatedTo);
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    related.setParams(params);

    result = related;
    QVERIFY(related == result);
}

void RelatedTest::shouldParseRelated()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:4.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "RELATED:friend\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).relationships().count(), 1);
    const KContacts::Related related = lst.at(0).relationships().at(0);
    QCOMPARE(related.related(), QStringLiteral("friend"));
}

void RelatedTest::shouldParseRelatedWithArgument()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:4.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "RELATED;FOO1=bla1,blo1;FOO2=bla2,blo2:friend\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).relationships().count(), 1);
    const KContacts::Related related = lst.at(0).relationships().at(0);
    QCOMPARE(related.related(), QStringLiteral("friend"));
    QCOMPARE(related.params().size(), 2);
}

void RelatedTest::shouldParseWithoutRelated()
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
    QCOMPARE(lst.at(0).relationships().count(), 0);
}

void RelatedTest::shouldCreateVCard4()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Related::List lstRelated;
    KContacts::Related related(QStringLiteral("friend"));
    lstRelated << related;
    addr.setRelationships(lstRelated);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "RELATED:friend\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void RelatedTest::shouldCreateVCardWithTwoRelated()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Related::List lstRelated;
    KContacts::Related related(QStringLiteral("friend"));
    KContacts::Related related2(QStringLiteral("kde"));
    lstRelated << related << related2;
    addr.setRelationships(lstRelated);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "RELATED:friend\r\n"
        "RELATED:kde\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void RelatedTest::shouldCreateVCardWithParameters()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Related::List lstRelated;
    KContacts::Related related(QStringLiteral("friend"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    related.setParams(params);
    lstRelated << related;
    addr.setRelationships(lstRelated);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "RELATED;FOO1=bla1,blo1;FOO2=bla2,blo2:friend\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

void RelatedTest::shouldNotExportInVcard3()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Related::List lstRelated;
    KContacts::Related related(QStringLiteral("friend"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    related.setParams(params);
    lstRelated << related;
    addr.setRelationships(lstRelated);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "RELATED;FOO1=bla1,blo1;FOO2=bla2,blo2:friend\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

QTEST_MAIN(RelatedTest)

#include "moc_relatedtest.cpp"
