/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "emailtest.h"
#include "email.h"
#include "parametermap_p.h"
#include "vcardtool_p.h"
#include <QTest>

EmailTest::EmailTest(QObject *parent)
    : QObject(parent)
{
}

EmailTest::~EmailTest()
{
}

void EmailTest::shouldHaveDefaultValue()
{
    KContacts::Email email;
    QVERIFY(!email.isValid());
    QVERIFY(email.mail().isEmpty());
    QVERIFY(email.params().empty());
}

void EmailTest::shouldAssignValue()
{
    const QString mail(QStringLiteral("foo@kde.org"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    KContacts::Email email(mail);
    email.setParams(params);
    QVERIFY(email.isValid());
    QVERIFY(!email.mail().isEmpty());
    QCOMPARE(email.mail(), mail);
    QVERIFY(!email.params().empty());
    QCOMPARE(email.params(), params);
}

void EmailTest::shouldAssignExternal()
{
    KContacts::Email email;
    const QString mail(QStringLiteral("foo@kde.org"));
    email.setEmail(mail);
    QVERIFY(email.isValid());
    QVERIFY(!email.mail().isEmpty());
    QCOMPARE(email.mail(), mail);
}

void EmailTest::shouldSerialized()
{
    KContacts::Email email;
    KContacts::Email result;
    const QString mail(QStringLiteral("foo@kde.org"));
    email.setEmail(mail);
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    email.setParams(params);

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << email;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> result;

    QVERIFY(email == result);
}

void EmailTest::shouldEqualEmail()
{
    KContacts::Email email;
    KContacts::Email result;
    const QString mail(QStringLiteral("foo@kde.org"));
    email.setEmail(mail);
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    email.setParams(params);

    result = email;
    QVERIFY(email == result);
}

void EmailTest::shouldParseEmailVCard()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "EMAIL;TYPE=HOME,PREF;X-EVOLUTION-UI-SLOT=2:foo@foo.com\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QVERIFY(!lst.at(0).emailList().isEmpty());
    QCOMPARE(lst.at(0).emailList().count(), 1);
    KContacts::Email email = lst.at(0).emailList().at(0);
    QCOMPARE(email.mail(), QStringLiteral("foo@foo.com"));
    QCOMPARE(email.params().size(), 2);
}

void EmailTest::shouldParseEmailVCardWithMultiEmails()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "EMAIL;TYPE=WORK;X-EVOLUTION-UI-SLOT=2:foo@foo.com\n"
        "EMAIL;TYPE=HOME,PREF;X-EVOLUTION-UI-SLOT=2:bla@bla.com\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QVERIFY(!lst.at(0).emailList().isEmpty());
    QCOMPARE(lst.at(0).emailList().count(), 2);
    KContacts::Email email = lst.at(0).emailList().at(0);
    QCOMPARE(email.mail(), QStringLiteral("bla@bla.com"));
    QCOMPARE(email.type(), KContacts::Email::Home);
    QVERIFY(email.isPreferred());
    email = lst.at(0).emailList().at(1);
    QCOMPARE(email.mail(), QStringLiteral("foo@foo.com"));
    QCOMPARE(email.type(), KContacts::Email::Work);
    QVERIFY(!email.isPreferred());

    QCOMPARE(email.params().size(), 2);
}

void EmailTest::shouldParseEmailVCardWithoutEmail()
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
    QVERIFY(lst.at(0).emailList().isEmpty());
}

void EmailTest::shouldExportVcard()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org") << QStringLiteral("bla@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    lst << addr;
    KContacts::VCardTool vcard;
    QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "EMAIL:bla@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);

    KContacts::Addressee addr2;
    addr2.setUid(QStringLiteral("testuid"));
    KContacts::Email email1;
    KContacts::Email email2;
    email1.setEmail(QStringLiteral("foo@kde.org"));
    email1.setType(KContacts::Email::Work);
    email1.setPreferred(true);
    email2.setEmail(QStringLiteral("bla@kde.org"));
    email2.setType(KContacts::Email::Home);
    addr2.setEmailList({email1, email2});
    ba = vcard.exportVCards({addr2}, KContacts::VCard::v4_0);
    QByteArray expected2(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL;TYPE=WORK,PREF:foo@kde.org\r\n"
        "EMAIL;TYPE=HOME:bla@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected2);
}

QTEST_MAIN(EmailTest)

#include "moc_emailtest.cpp"
