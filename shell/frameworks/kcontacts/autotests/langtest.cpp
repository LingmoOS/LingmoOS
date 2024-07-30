/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "langtest.h"
#include "lang.h"
#include "parametermap_p.h"
#include "vcardtool_p.h"
#include <QTest>

LangTest::LangTest(QObject *parent)
    : QObject(parent)
{
}

LangTest::~LangTest()
{
}

void LangTest::shouldHaveDefaultValue()
{
    KContacts::Lang language;
    QVERIFY(!language.isValid());
    QVERIFY(language.language().isEmpty());
    QVERIFY(language.params().empty());
}

void LangTest::shouldAssignValue()
{
    const QString lang(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    KContacts::Lang language(lang);
    language.setParams(params);
    QVERIFY(language.isValid());
    QVERIFY(!language.language().isEmpty());
    QCOMPARE(language.language(), lang);
    QVERIFY(!language.params().empty());
    QCOMPARE(language.params(), params);
}

void LangTest::shouldAssignExternal()
{
    KContacts::Lang language;
    const QString lang(QStringLiteral("fr"));
    language.setLanguage(lang);
    QVERIFY(language.isValid());
    QCOMPARE(language.language(), lang);
}

void LangTest::shouldSerialized()
{
    KContacts::Lang language;
    KContacts::Lang result;
    const QString lang(QStringLiteral("fr"));
    language.setLanguage(lang);
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    language.setParams(params);

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << language;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> result;

    QVERIFY(language == result);
}

void LangTest::shouldEqualLanguage()
{
    KContacts::Lang language;
    KContacts::Lang result;
    const QString lang(QStringLiteral("fr"));
    language.setLanguage(lang);
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    language.setParams(params);

    result = language;
    QVERIFY(language == result);
}

void LangTest::shouldParseLanguage()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "LANG:fr\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).langs().count(), 1);
    QCOMPARE(lst.at(0).langs().at(0).language(), QStringLiteral("fr"));
}

void LangTest::shouldParseWithoutLanguage()
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
    QCOMPARE(lst.at(0).langs().count(), 0);
}

void LangTest::shouldCreateVCard()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Lang::List lstLang;
    KContacts::Lang lang(QStringLiteral("fr"));
    lstLang << lang;
    addr.setLangs(lstLang);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "LANG:fr\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void LangTest::shouldCreateVCardWithTwoLang()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Lang::List lstLang;
    KContacts::Lang lang(QStringLiteral("fr"));
    KContacts::Lang lang2(QStringLiteral("fr2"));
    lstLang << lang << lang2;
    addr.setLangs(lstLang);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "LANG:fr\r\n"
        "LANG:fr2\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void LangTest::shouldCreateVCardWithParameters()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Lang::List lstLang;
    KContacts::Lang lang(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    lang.setParams(params);
    lstLang << lang;
    addr.setLangs(lstLang);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "LANG;FOO1=bla1,blo1;FOO2=bla2,blo2:fr\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

void LangTest::shouldNotGenerateLangForVCard3()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Lang::List lstLang;
    KContacts::Lang lang(QStringLiteral("fr"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    lang.setParams(params);
    lstLang << lang;
    addr.setLangs(lstLang);
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

QTEST_MAIN(LangTest)

#include "moc_langtest.cpp"
