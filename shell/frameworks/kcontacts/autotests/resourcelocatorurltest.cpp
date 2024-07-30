/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "resourcelocatorurltest.h"

#include "parametermap_p.h"
#include "resourcelocatorurl.h"
#include "vcardtool_p.h"
#include <QTest>

using namespace KContacts;
ResourceLocatorUrlTest::ResourceLocatorUrlTest(QObject *parent)
    : QObject(parent)
{
}

ResourceLocatorUrlTest::~ResourceLocatorUrlTest()
{
}

void ResourceLocatorUrlTest::shouldHaveDefaultValue()
{
    ResourceLocatorUrl ResourceLocatorUrl;
    QVERIFY(!ResourceLocatorUrl.isValid());
    QVERIFY(ResourceLocatorUrl.url().isEmpty());
    QVERIFY(ResourceLocatorUrl.params().empty());
}

void ResourceLocatorUrlTest::shouldAssignValue()
{
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    ResourceLocatorUrl ResourceLocatorUrl;
    ResourceLocatorUrl.setParams(params);
    QVERIFY(!ResourceLocatorUrl.isValid());
    QVERIFY(ResourceLocatorUrl.url().isEmpty());
    QVERIFY(!ResourceLocatorUrl.params().empty());
    QCOMPARE(ResourceLocatorUrl.params(), params);
}

void ResourceLocatorUrlTest::shouldAssignExternal()
{
    ResourceLocatorUrl resourcelocatorurl;
    QUrl url = QUrl(QStringLiteral("https://www.kde.org"));
    resourcelocatorurl.setUrl(url);
    QVERIFY(!resourcelocatorurl.url().isEmpty());
    QCOMPARE(resourcelocatorurl.url(), url);
}

void ResourceLocatorUrlTest::shouldSerialized()
{
    ResourceLocatorUrl resourcelocatorurl;
    ResourceLocatorUrl result;
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    resourcelocatorurl.setParams(params);
    resourcelocatorurl.setUrl(QUrl(QStringLiteral("mailto:foo@kde.org")));

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << resourcelocatorurl;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> result;

    QVERIFY(resourcelocatorurl == result);
}

void ResourceLocatorUrlTest::shouldEqualResourceLocatorUrl()
{
    ResourceLocatorUrl resourcelocatorurl;
    ResourceLocatorUrl result;
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    resourcelocatorurl.setUrl(QUrl(QStringLiteral("mailto:foo@kde.org")));
    resourcelocatorurl.setParams(params);

    result = resourcelocatorurl;
    QVERIFY(resourcelocatorurl == result);
}

void ResourceLocatorUrlTest::shouldParseResourceLocatorUrl()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "URL;PREF=1;TYPE=HOME:https://firsturl\n"
        "URL;TYPE=WORK,PREF:https://sherlockholmes.com/calendar/sherlockholmes\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).extraUrlList().count(), 2);
    auto calurl = lst.at(0).extraUrlList().at(0);
    QCOMPARE(calurl.url(), QUrl(QStringLiteral("https://firsturl")));
    QVERIFY(calurl.isPreferred());
    QCOMPARE(calurl.type(), KContacts::ResourceLocatorUrl::Home);
    QVERIFY(!calurl.params().empty());
    calurl = lst.at(0).extraUrlList().at(1);
    QCOMPARE(calurl.url(), QUrl(QStringLiteral("https://sherlockholmes.com/calendar/sherlockholmes")));
    QVERIFY(calurl.isPreferred());
    QCOMPARE(calurl.type(), KContacts::ResourceLocatorUrl::Work);
    QVERIFY(!calurl.params().empty());
}

void ResourceLocatorUrlTest::shouldGenerateVCard4()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::ResourceLocatorUrl webpage;
    webpage.setUrl(QUrl(QStringLiteral("https://www.kde.org")));
    addr.setUrl(webpage);
    ResourceLocatorUrl url;
    url.setUrl(QUrl(QStringLiteral("https://sherlockholmes.com/calendar/sherlockholmes")));
    addr.insertExtraUrl(url);
    url.setUrl(QUrl(QStringLiteral("https://foo.kde.org")));
    addr.insertExtraUrl(url);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected;
    expected = QByteArray(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "URL:https://www.kde.org\r\n"
        "URL:https://sherlockholmes.com/calendar/sherlockholmes\r\n"
        "URL:https://foo.kde.org\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void ResourceLocatorUrlTest::shouldGenerateVCardWithParameter()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::ResourceLocatorUrl webpage;
    webpage.setUrl(QUrl(QStringLiteral("https://www.kde.org")));
    addr.setUrl(webpage);
    ResourceLocatorUrl url;
    url.setUrl(QUrl(QStringLiteral("https://sherlockholmes.com/calendar/sherlockholmes")));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    url.setParams(params);
    addr.insertExtraUrl(url);
    url.setUrl(QUrl(QStringLiteral("https://foo.kde.org")));
    ParameterMap params2;
    url.setParams(params2);
    addr.insertExtraUrl(url);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected;
    expected = QByteArray(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "URL:https://www.kde.org\r\n"
        "URL;FOO2=bla2,blo2:https://sherlockholmes.com/calendar/sherlockholmes\r\n"
        "URL:https://foo.kde.org\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

void ResourceLocatorUrlTest::shouldGenerateVCard3()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::ResourceLocatorUrl webpage;
    webpage.setUrl(QUrl(QStringLiteral("https://www.kde.org")));
    addr.setUrl(webpage);
    ResourceLocatorUrl url;
    url.setUrl(QUrl(QStringLiteral("https://sherlockholmes.com/calendar/sherlockholmes")));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    url.setParams(params);
    addr.insertExtraUrl(url);
    url.setUrl(QUrl(QStringLiteral("https://foo.kde.org")));
    ParameterMap params2;
    url.setParams(params2);
    url.setType(ResourceLocatorUrl::Work);
    url.setPreferred(true);
    addr.insertExtraUrl(url);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected;
    expected = QByteArray(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "URL:https://www.kde.org\r\n"
        "URL;FOO2=bla2,blo2:https://sherlockholmes.com/calendar/sherlockholmes\r\n"
        "URL;TYPE=WORK,PREF:https://foo.kde.org\r\n"
        "END:VCARD\r\n\r\n");

    QCOMPARE(ba, expected);
}

QTEST_MAIN(ResourceLocatorUrlTest)

#include "moc_resourcelocatorurltest.cpp"
