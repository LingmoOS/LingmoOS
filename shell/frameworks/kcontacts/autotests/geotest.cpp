/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 KDE-PIM team <kde-pim@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "geotest.h"
#include "kcontacts/geo.h"
#include "vcardtool_p.h"
#include <QTest>

QTEST_MAIN(GeoTest)

void GeoTest::constructor()
{
    KContacts::Geo geo(1.2f, 3.4f);

    QVERIFY((float)geo.latitude() == (float)1.2);
    QVERIFY((float)geo.longitude() == (float)3.4);
}

void GeoTest::isValid()
{
    KContacts::Geo geo;

    QVERIFY(!geo.isValid());

    geo.setLatitude(23);

    QVERIFY(!geo.isValid());

    geo.setLongitude(45);

    QVERIFY(geo.isValid());

    geo.clear();

    QVERIFY(!geo.isValid());
    QVERIFY(geo == KContacts::Geo());
}

void GeoTest::setData()
{
    KContacts::Geo geo;

    geo.setLatitude(22.5f);
    geo.setLongitude(45.1f);

    QVERIFY((float)geo.latitude() == (float)22.5);
    QVERIFY((float)geo.longitude() == (float)45.1);
}

void GeoTest::equals()
{
    KContacts::Geo geo1(22.5f, 33.7f);
    KContacts::Geo geo2(22.5f, 33.7f);

    QVERIFY(geo1 == geo2);
}

void GeoTest::differs()
{
    KContacts::Geo geo1(22.5f, 33.7f);
    KContacts::Geo geo2(22.5f, 33.6f);

    QVERIFY(geo1 != geo2);
}

void GeoTest::serialization()
{
    KContacts::Geo geo1(22.5f, 33.7f);
    QByteArray data;

    QDataStream s(&data, QIODevice::WriteOnly);
    s << geo1;

    KContacts::Geo geo2;
    QDataStream t(&data, QIODevice::ReadOnly);
    t >> geo2;

    QVERIFY(geo1 == geo2);
}

void GeoTest::shouldParseGeoVCard3()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "GEO:22.5;180.0\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QVERIFY(lst.at(0).geo().isValid());
    KContacts::Geo geo = lst.at(0).geo();
    QCOMPARE(geo.latitude(), 22.5);
    QCOMPARE(geo.longitude(), 180.0);
}

void GeoTest::shouldParseGeoVCard4()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:4.0\n"
        "N:LastName;FirstName;;;\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "GEO:geo:22.5,180.0\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QVERIFY(lst.at(0).geo().isValid());
    KContacts::Geo geo = lst.at(0).geo();
    QCOMPARE(geo.latitude(), 22.5);
    QCOMPARE(geo.longitude(), 180.0);
}

void GeoTest::shouldGenerateVCard3()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));

    KContacts::Geo geo;
    geo.setLongitude(180.0);
    geo.setLatitude(22.5);
    addr.setGeo(geo);

    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "GEO:22.500000;180.000000\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

void GeoTest::shouldGenerateVCard4()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));

    KContacts::Geo geo;
    geo.setLongitude(180.0);
    geo.setLatitude(22.5);
    addr.setGeo(geo);

    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "GEO:geo:22.500000,180.000000\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

void GeoTest::shouldGenerateWithoutGeo()
{
    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));

    lst << addr;
    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

#include "moc_geotest.cpp"
