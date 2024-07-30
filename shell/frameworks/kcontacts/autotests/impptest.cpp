/*
    This file is part of libkabc.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "impptest.h"
#include "impp.h"
#include "parametermap_p.h"
#include "vcardtool_p.h"
#include <QTest>

ImppTest::ImppTest(QObject *parent)
    : QObject(parent)
{
}

ImppTest::~ImppTest()
{
}

void ImppTest::shouldHaveDefaultValue()
{
    KContacts::Impp impp;
    QVERIFY(!impp.isValid());
    QVERIFY(impp.address().isEmpty());
    QVERIFY(impp.serviceType().isEmpty());
    QVERIFY(impp.params().empty());
}

void ImppTest::shouldAssignValue()
{
    const QUrl address(QStringLiteral("icq:address"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    KContacts::Impp impp;
    impp.setParams(params);
    impp.setAddress(address);
    QVERIFY(impp.isValid());
    QVERIFY(!impp.address().isEmpty());
    QCOMPARE(impp.address(), address);
    QCOMPARE(impp.serviceType(), QLatin1String("icq"));
    QCOMPARE(impp.serviceIcon(), QLatin1String("im-icq"));
    QVERIFY(!impp.params().empty());
    QCOMPARE(impp.params(), params);
}

void ImppTest::shouldSerialized()
{
    const QUrl address(QStringLiteral("icq:address"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    KContacts::Impp impp;
    impp.setParams(params);
    impp.setAddress(address);

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << impp;

    KContacts::Impp result;
    QDataStream t(&data, QIODevice::ReadOnly);
    t >> result;

    QVERIFY(impp == result);
}

void ImppTest::shouldEqualImpp()
{
    const QUrl address(QStringLiteral("icq:address"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    KContacts::Impp impp;
    impp.setParams(params);
    impp.setAddress(address);

    KContacts::Impp result(impp);
    QVERIFY(impp == result);
}

void ImppTest::shouldParseWithoutImpp()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "LANG:fr"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).imppList().count(), 0);
}

void ImppTest::shouldParseImpp()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "IMPP;X-SERVICE-TYPE=skype:skype:xxxxxxxx\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).imppList().count(), 1);
    KContacts::Impp impp = lst.at(0).imppList().at(0);
    QCOMPARE(impp.address(), QUrl(QStringLiteral("skype:xxxxxxxx")));
    QCOMPARE(impp.serviceType(), QLatin1String("skype"));
}

void ImppTest::shouldParseImppVcard4()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:4.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "IMPP;PREF=1:skype:xxxxxxxx\n"
        "IMPP:skype:1234567890\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).imppList().count(), 2);
    KContacts::Impp impp = lst.at(0).imppList().at(0);
    QCOMPARE(impp.address(), QUrl(QStringLiteral("skype:xxxxxxxx")));
    QCOMPARE(impp.serviceType(), QLatin1String("skype"));
    QVERIFY(impp.isPreferred());
    impp = lst.at(0).imppList().at(1);
    QCOMPARE(impp.address(), QUrl(QStringLiteral("skype:1234567890")));
    QCOMPARE(impp.serviceType(), QLatin1String("skype"));
    QVERIFY(!impp.isPreferred());
}

QByteArray createCard(const QString &type)
{
    QByteArray expected(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "EMAIL:foo@kde.org\n");
    if (!type.isEmpty()) {
        expected += "IMPP;X-SERVICE-TYPE=" + type.toLatin1() + ":" + type.toLatin1() + ":address\n";
    }
    expected += QByteArray(
        "N:;;;;\n"
        "UID:testuid\n"
        "END:VCARD\n\n");
    return expected;
}

void ImppTest::shouldParseServiceType_data()
{
    QTest::addColumn<QString>("type");
    QTest::addColumn<bool>("hasImpp");
    QTest::newRow("withoutimpp") << QString() << false;
    for (const auto &type : KContacts::Impp::serviceTypes()) {
        QTest::newRow(type.toLatin1().constData()) << type << true;
    }
}

void ImppTest::shouldParseServiceType()
{
    QFETCH(QString, type);
    QFETCH(bool, hasImpp);

    QByteArray vcarddata = createCard(type);

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);

    QCOMPARE(!lst.at(0).imppList().isEmpty(), hasImpp);
    if (hasImpp) {
        KContacts::Impp impp = lst.at(0).imppList().at(0);
        QCOMPARE(impp.address(), QUrl(QStringLiteral("%1:address").arg(type)));
        QCOMPARE(impp.serviceType(), type);
    }
}

QByteArray expectedVcard(const QString &type)
{
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n");
    if (!type.isEmpty()) {
        expected += "IMPP:" + type.toLatin1() + ":address\r\n";
    }

    expected +=
        ("N:;;;;\r\n"
         "UID:testuid\r\n"
         "END:VCARD\r\n\r\n");
    return expected;
}

void ImppTest::shouldExportEmptyType()
{
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QCOMPARE(ba, expected);
}

void ImppTest::shouldExportType_data()
{
    QTest::addColumn<QString>("type");
    for (const auto &type : KContacts::Impp::serviceTypes()) {
        QTest::newRow(type.toLatin1().constData()) << type;
    }
}

void ImppTest::shouldExportType()
{
    QFETCH(QString, type);

    QByteArray expected = expectedVcard(type);
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::Impp impp;
    impp.setAddress(QUrl(type + QStringLiteral(":address")));
    addr.insertImpp(impp);
    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QCOMPARE(ba, expected);
}

void ImppTest::shouldExportWithParameters()
{
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "IMPP;FOO1=bla1,blo1;FOO2=bla2,blo2:skype:address\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    KContacts::Impp impp;
    impp.setAddress(QUrl(QStringLiteral("skype:address")));
    impp.setParams(params);
    impp.setPreferred(false);
    addr.insertImpp(impp);
    lst << addr;

    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QCOMPARE(ba, expected);
}

void ImppTest::shouldShouldNotExportTwiceServiceType()
{
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "IMPP;FOO1=bla1,blo1;FOO2=bla2,blo2;PREF=1:skype:address\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    KContacts::ParameterMap params;
    params.push_back({QStringLiteral("Foo1"), {QStringLiteral("bla1"), QStringLiteral("blo1")}});
    params.push_back({QStringLiteral("Foo2"), {QStringLiteral("bla2"), QStringLiteral("blo2")}});
    params.push_back({QStringLiteral("X-SERVICE-TYPE"), QStringList() << QStringLiteral("aim")});
    KContacts::Impp impp;
    impp.setAddress(QUrl(QStringLiteral("skype:address")));
    impp.setParams(params);
    impp.setPreferred(true);
    addr.insertImpp(impp);
    lst << addr;

    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QCOMPARE(ba, expected);
}

void ImppTest::testProtocolInformation()
{
    const auto types = KContacts::Impp::serviceTypes();
    QVERIFY(types.size() > 10);
    QVERIFY(types.contains(QLatin1String("xmpp")));

    for (const auto &type : types) {
        QVERIFY(!KContacts::Impp::serviceLabel(type).isEmpty());
    }

    QCOMPARE(KContacts::Impp::serviceIcon(QStringLiteral("xmpp")), QLatin1String("im-jabber"));
}

QTEST_MAIN(ImppTest)

#include "moc_impptest.cpp"
