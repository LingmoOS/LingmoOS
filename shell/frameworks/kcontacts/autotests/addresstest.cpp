/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "addresstest.h"
#include "addresseelist.h"
#include "kcontacts/address.h"
#include "vcardtool_p.h"
#include <KConfig>
#include <KConfigGroup>
#include <QStandardPaths>
#include <QTest>

QTEST_MAIN(AddressTest)
void initLocale()
{
    qputenv("LANG", "it_CH.UTF-8");
    QLocale::setDefault(QLocale(QLocale::Italian, QLocale::Switzerland));
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

void AddressTest::initTestCase()
{
}

void AddressTest::emptyTest()
{
    KContacts::Address address;

    QVERIFY(address.isEmpty());
}

void AddressTest::storeTest()
{
    KContacts::Address address;

    address.setId(QStringLiteral("My Id"));
    address.setType(KContacts::Address::Home);
    address.setPostOfficeBox(QStringLiteral("1234"));
    address.setExtended(QStringLiteral("My Extended Label"));
    address.setStreet(QStringLiteral("My Street"));
    address.setLocality(QStringLiteral("My Locality"));
    address.setRegion(QStringLiteral("My Region"));
    address.setPostalCode(QStringLiteral("My PostalCode"));
    address.setCountry(QStringLiteral("My Country"));
    address.setLabel(QStringLiteral("My Label"));

    QCOMPARE(address.id(), QStringLiteral("My Id"));
    QCOMPARE(address.type(), KContacts::Address::Home);
    QCOMPARE(address.postOfficeBox(), QStringLiteral("1234"));
    QCOMPARE(address.extended(), QStringLiteral("My Extended Label"));
    QCOMPARE(address.street(), QStringLiteral("My Street"));
    QCOMPARE(address.locality(), QStringLiteral("My Locality"));
    QCOMPARE(address.region(), QStringLiteral("My Region"));
    QCOMPARE(address.postalCode(), QStringLiteral("My PostalCode"));
    QCOMPARE(address.country(), QStringLiteral("My Country"));
    QCOMPARE(address.label(), QStringLiteral("My Label"));
    QVERIFY(!address.geo().isValid());
}

void AddressTest::equalsTest()
{
    KContacts::Address address1;
    KContacts::Address address2;

    address1.setId(QStringLiteral("My Id"));
    address1.setType(KContacts::Address::Home);
    address1.setPostOfficeBox(QStringLiteral("1234"));
    address1.setExtended(QStringLiteral("My Extended Label"));
    address1.setStreet(QStringLiteral("My Street"));
    address1.setLocality(QStringLiteral("My Locality"));
    address1.setRegion(QStringLiteral("My Region"));
    address1.setPostalCode(QStringLiteral("My Postalcode"));
    address1.setCountry(QStringLiteral("My country"));
    address1.setLabel(QStringLiteral("My Label"));

    address2.setId(QStringLiteral("My Id"));
    address2.setType(KContacts::Address::Home);
    address2.setPostOfficeBox(QStringLiteral("1234"));
    address2.setExtended(QStringLiteral("My Extended Label"));
    address2.setStreet(QStringLiteral("My Street"));
    address2.setLocality(QStringLiteral("My Locality"));
    address2.setRegion(QStringLiteral("My Region"));
    address2.setPostalCode(QStringLiteral("My Postalcode"));
    address2.setCountry(QStringLiteral("My country"));
    address2.setLabel(QStringLiteral("My Label"));

    QVERIFY(address1 == address2);
}

void AddressTest::differsTest()
{
    KContacts::Address address1(KContacts::Address::Home);
    KContacts::Address address2(KContacts::Address::Work);

    QVERIFY(address1 != address2);
}

void AddressTest::assignmentTest()
{
    KContacts::Address address1;
    KContacts::Address address2;

    address1.setId(QStringLiteral("My Id"));
    address1.setType(KContacts::Address::Home);
    address1.setPostOfficeBox(QStringLiteral("1234"));
    address1.setExtended(QStringLiteral("My Extended Label"));
    address1.setStreet(QStringLiteral("My Street"));
    address1.setLocality(QStringLiteral("My Locality"));
    address1.setRegion(QStringLiteral("My Region"));
    address1.setPostalCode(QStringLiteral("My Postalcode"));
    address1.setCountry(QStringLiteral("My country"));
    address1.setLabel(QStringLiteral("My Label"));

    address2 = address1;

    QVERIFY(address1 == address2);
}

void AddressTest::serializeTest()
{
    KContacts::Address address1;
    KContacts::Address address2;

    address1.setType(KContacts::Address::Work);
    address1.setPostOfficeBox(QStringLiteral("1234"));
    address1.setExtended(QStringLiteral("My Extended Label"));
    address1.setStreet(QStringLiteral("My Street"));
    address1.setLocality(QStringLiteral("My Locality"));
    address1.setRegion(QStringLiteral("My Region"));
    address1.setPostalCode(QStringLiteral("My Postalcode"));
    address1.setCountry(QStringLiteral("My country"));
    address1.setLabel(QStringLiteral("My Label"));

    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    s << address1;

    QDataStream t(&data, QIODevice::ReadOnly);
    t >> address2;

    QVERIFY(address1 == address2);
}

void AddressTest::formatTest()
{
    {
        KContacts::Address address;
        address.setStreet(QStringLiteral("Lummerlandstr. 1"));
        address.setPostalCode(QStringLiteral("12345"));
        address.setLocality(QStringLiteral("Lummerstadt"));
        address.setCountry(QStringLiteral("Germany"));

        const QString result(
            QStringLiteral("Jim Knopf\nLummerlandstr. 1\n"
                           "12345 Lummerstadt\n\nGERMANIA"));

        QCOMPARE(address.formatted(KContacts::AddressFormatStyle::Postal, QStringLiteral("Jim Knopf")), result);
    }

    {
        KContacts::Address address;
        address.setStreet(QStringLiteral("457 Foobar Ave"));
        address.setPostalCode(QStringLiteral("1A2B3C"));
        address.setLocality(QStringLiteral("Nervousbreaktown"));
        address.setRegion(QStringLiteral("DC"));
        address.setCountry(QStringLiteral("United States of America"));

        const QString result(
            QStringLiteral("Huck Finn\n457 Foobar Ave\nNERVOUSBREAKTOWN,"
                           " DC 1A2B3C\n\nSTATI UNITI"));
        QCOMPARE(address.formatted(KContacts::AddressFormatStyle::Postal, QStringLiteral("Huck Finn")), result);
    }

    {
        KContacts::Address address;
        address.setStreet(QStringLiteral("Lummerlandstr. 1"));
        address.setPostalCode(QStringLiteral("12345"));
        address.setLocality(QStringLiteral("Lummerstadt"));
        address.setCountry(QStringLiteral("Deutschland"));

        const QString result(
            QStringLiteral("Jim Knopf\nLummerlandstr. 1\n"
                           "12345 Lummerstadt\n\nGERMANIA"));

        QCOMPARE(address.formatted(KContacts::AddressFormatStyle::Postal, QStringLiteral("Jim Knopf")), result);
    }

    {
        KContacts::Address address;
        address.setStreet(QStringLiteral("Lummerlandstr. 1"));
        address.setPostalCode(QStringLiteral("12345"));
        address.setLocality(QStringLiteral("Lummerstadt"));
        address.setCountry(QString());

        const QString result(QStringLiteral("Jim Knopf\nLummerlandstr. 1\n12345 Lummerstadt"));

        QCOMPARE(address.formatted(KContacts::AddressFormatStyle::Postal, QStringLiteral("Jim Knopf")), result);
    }

    {
        KContacts::Address address;
        address.setStreet(QStringLiteral("Haus Randa"));
        address.setPostalCode(QStringLiteral("1234"));
        address.setLocality(QStringLiteral("Randa"));
        address.setPostOfficeBox(QStringLiteral("5678"));
        address.setCountry(QStringLiteral("Schweiz"));

        // we want the Italian variant of the Swiss format for it_CH
        const QString result(QStringLiteral("Dr. Konqui\nCasella postale 5678\nHaus Randa\n1234 Randa\n\nSVIZZERA"));

        QCOMPARE(address.formatted(KContacts::AddressFormatStyle::Postal, QStringLiteral("Dr. Konqui")), result);
    }

    {
        KContacts::Address address;
        address.setStreet(QStringLiteral("Haus Randa"));
        address.setPostalCode(QStringLiteral("1234"));
        address.setLocality(QStringLiteral("Randa"));
        address.setPostOfficeBox(QStringLiteral("5678"));
        address.setCountry(QStringLiteral("CH"));

        // we want the Italian variant of the Swiss format for it_CH
        const QString result(QStringLiteral("Dr. Konqui\nCasella postale 5678\nHaus Randa\n1234 Randa\n\nSVIZZERA"));
        QCOMPARE(address.formatted(KContacts::AddressFormatStyle::Postal, QStringLiteral("Dr. Konqui")), result);
    }

    {
        KContacts::Address address;
        address.setCountry(QStringLiteral("CH"));
        QCOMPARE(address.formatted(KContacts::AddressFormatStyle::Postal, QString()), QLatin1String("SVIZZERA"));
    }

    {
        // business format for postal style
        KContacts::Address address;
        address.setCountry(QStringLiteral("AT"));
        address.setLocality(QStringLiteral("Berlin"));
        address.setPostalCode(QStringLiteral("10969"));
        address.setStreet(QStringLiteral("Prinzenstraße 85 F"));
        const auto result = QString::fromUtf8("KDE e.V.\nz.Hd. Dr. Konqi\nPrinzenstraße 85 F\n10969 Berlin\n\nAUSTRIA");
        QCOMPARE(address.formatted(KContacts::AddressFormatStyle::Postal, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V.")), result);
    }

    {
        // local vs latin script formats
        KContacts::Address address;
        address.setCountry(QStringLiteral("JP"));
        address.setRegion(QStringLiteral("Tokyo"));
        address.setLocality(QStringLiteral("Minato-ku"));
        address.setPostalCode(QStringLiteral("106-0047"));
        address.setStreet(QStringLiteral("4-6-28 Minami-Azabu"));
        auto result = QString::fromUtf8("4-6-28 Minami-Azabu, Minato-ku\nTOKYO 106-0047\n\nGIAPPONE");
        QCOMPARE(address.formatted(KContacts::AddressFormatStyle::Postal), result);

        address.setRegion(QStringLiteral("東京"));
        address.setLocality(QStringLiteral("都港区"));
        address.setPostalCode(QStringLiteral("106-0047"));
        address.setStreet(QStringLiteral("南麻布 4-6-28"));
        result = QString::fromUtf8("〒106-0047\n東京都港区南麻布 4-6-28\n\nGIAPPONE");
        QCOMPARE(address.formatted(KContacts::AddressFormatStyle::Postal), result);
    }
}

void AddressTest::shouldExportVcard3()
{
    KContacts::Address address;

    address.setId(QStringLiteral("My Id"));
    address.setType(KContacts::Address::Home);
    address.setPostOfficeBox(QStringLiteral("1234"));
    address.setExtended(QStringLiteral("My Extended Label"));
    address.setStreet(QStringLiteral("My Street"));
    address.setLocality(QStringLiteral("My Locality"));
    address.setRegion(QStringLiteral("My Region"));
    address.setPostalCode(QStringLiteral("My Postalcode"));
    address.setCountry(QStringLiteral("My country"));
    address.setLabel(QStringLiteral("My Label"));

    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    addr.insertAddress(address);
    lst << addr;

    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v3_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:3.0\r\n"
        "ADR;TYPE=home:1234;My Extended Label;My Street;My Locality;My Region;My Pos\r\n"
        " talcode;My country\r\n"
        "EMAIL:foo@kde.org\r\n"
        "LABEL;TYPE=home:My Label\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

void AddressTest::shouldParseAddressVCard3()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\n"
        "VERSION:3.0\n"
        "N:LastName;FirstName;;;\n"
        "ADR;TYPE=home:1234;My Extended Label;My Street;My Locality;My Region;My Pos\r\n"
        " talcode;My country\r\n"
        "UID:c80cf296-0825-4eb0-ab16-1fac1d522a33@xxxxxx.xx\n"
        "LABEL;TYPE=home:My Label\r\n"
        "REV:2015-03-14T09:24:45+00:00\n"
        "FN:FirstName LastName\n"
        "END:VCARD\n");

    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).addresses().count(), 1);
    KContacts::Address address = lst.at(0).addresses().at(0);
    QCOMPARE(address.type(), KContacts::Address::Home);
    QCOMPARE(address.postOfficeBox(), QStringLiteral("1234"));
    QCOMPARE(address.extended(), QStringLiteral("My Extended Label"));
    QCOMPARE(address.region(), QStringLiteral("My Region"));
    QCOMPARE(address.street(), QStringLiteral("My Street"));
    QCOMPARE(address.country(), QStringLiteral("My country"));
    QCOMPARE(address.postalCode(), QStringLiteral("My Postalcode"));
    QCOMPARE(address.label(), QStringLiteral("My Label"));
}

void AddressTest::shouldParseAddressVCard4()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "ADR;LABEL=\"My Label\";TYPE=home:1234;My Extended Label;My Street;My Locality\r\n"
        " ;My Region;My Postalcode;My country\r\nEMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).addresses().count(), 1);
    KContacts::Address address = lst.at(0).addresses().at(0);
    QCOMPARE(address.type(), KContacts::Address::Home);
    QCOMPARE(address.postOfficeBox(), QStringLiteral("1234"));
    QCOMPARE(address.extended(), QStringLiteral("My Extended Label"));
    QCOMPARE(address.region(), QStringLiteral("My Region"));
    QCOMPARE(address.street(), QStringLiteral("My Street"));
    QCOMPARE(address.country(), QStringLiteral("My country"));
    QCOMPARE(address.postalCode(), QStringLiteral("My Postalcode"));
    QCOMPARE(address.label(), QStringLiteral("My Label"));
}

void AddressTest::shouldParseAddressVCard4WithGeoPosition()
{
    QByteArray vcarddata(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "ADR;GEO=\"geo:22.500000,45.099998\";LABEL=\"My Label\";TYPE=home:1234;My Extend\r\n"
        " ed Label;My Street;My Locality;My Region;My Postalcode;My country\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    KContacts::VCardTool vcard;
    const KContacts::AddresseeList lst = vcard.parseVCards(vcarddata);
    QCOMPARE(lst.count(), 1);
    QCOMPARE(lst.at(0).addresses().count(), 1);
    KContacts::Address address = lst.at(0).addresses().at(0);
    QCOMPARE(address.type(), KContacts::Address::Home);
    QCOMPARE(address.postOfficeBox(), QStringLiteral("1234"));
    QCOMPARE(address.extended(), QStringLiteral("My Extended Label"));
    QCOMPARE(address.region(), QStringLiteral("My Region"));
    QCOMPARE(address.street(), QStringLiteral("My Street"));
    QCOMPARE(address.country(), QStringLiteral("My country"));
    QCOMPARE(address.postalCode(), QStringLiteral("My Postalcode"));
    QCOMPARE(address.label(), QStringLiteral("My Label"));
    KContacts::Geo geo = address.geo();
    QVERIFY(geo.isValid());
    QCOMPARE(geo.latitude(), 22.5f);
    QCOMPARE(geo.longitude(), 45.1f);
}

void AddressTest::shouldExportVCard4WithGeoPosition()
{
    KContacts::Address address;

    address.setId(QStringLiteral("My Id"));
    address.setType(KContacts::Address::Home);
    address.setPostOfficeBox(QStringLiteral("1234"));
    address.setExtended(QStringLiteral("My Extended Label"));
    address.setStreet(QStringLiteral("My Street"));
    address.setLocality(QStringLiteral("My Locality"));
    address.setRegion(QStringLiteral("My Region"));
    address.setPostalCode(QStringLiteral("My Postalcode"));
    address.setCountry(QStringLiteral("My country"));
    address.setLabel(QStringLiteral("My Label"));

    KContacts::Geo geo;

    geo.setLatitude(22.5f);
    geo.setLongitude(45.1f);
    address.setGeo(geo);

    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    addr.insertAddress(address);
    lst << addr;

    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "ADR;GEO=\"geo:22.500000,45.099998\";LABEL=\"My Label\";TYPE=home:1234;My Extend\r\n"
        " ed Label;My Street;My Locality;My Region;My Postalcode;My country\r\n"
        "EMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

void AddressTest::shouldExportVcard4()
{
    KContacts::Address address;

    address.setId(QStringLiteral("My Id"));
    address.setType(KContacts::Address::Home);
    address.setPostOfficeBox(QStringLiteral("1234"));
    address.setExtended(QStringLiteral("My Extended Label"));
    address.setStreet(QStringLiteral("My Street"));
    address.setLocality(QStringLiteral("My Locality"));
    address.setRegion(QStringLiteral("My Region"));
    address.setPostalCode(QStringLiteral("My Postalcode"));
    address.setCountry(QStringLiteral("My country"));
    address.setLabel(QStringLiteral("My Label"));

    KContacts::AddresseeList lst;
    KContacts::Addressee addr;
    addr.setEmails(QStringList() << QStringLiteral("foo@kde.org"));
    addr.setUid(QStringLiteral("testuid"));
    addr.insertAddress(address);
    lst << addr;

    KContacts::VCardTool vcard;
    const QByteArray ba = vcard.exportVCards(lst, KContacts::VCard::v4_0);
    QByteArray expected(
        "BEGIN:VCARD\r\n"
        "VERSION:4.0\r\n"
        "ADR;LABEL=\"My Label\";TYPE=home:1234;My Extended Label;My Street;My Locality\r\n"
        " ;My Region;My Postalcode;My country\r\nEMAIL:foo@kde.org\r\n"
        "N:;;;;\r\n"
        "UID:testuid\r\n"
        "END:VCARD\r\n\r\n");
    QCOMPARE(ba, expected);
}

void AddressTest::testGeoUri()
{
    KContacts::Address addr;
    addr.setCountry(QStringLiteral("DE"));
    addr.setRegion(QStringLiteral("BE"));
    addr.setLocality(QStringLiteral("Berlin"));
    addr.setPostalCode(QStringLiteral("10969"));
    addr.setStreet(QStringLiteral("Prinzenstraße 85 F"));
    addr.setExtended(QStringLiteral("East wing"));

    QCOMPARE(addr.geoUri(), QUrl(QStringLiteral("geo:0,0?q=Prinzenstraße 85 F,10969 Berlin,DE")));

    addr.setGeo({52.503, 13.41});
    QCOMPARE(addr.geoUri(), QUrl(QStringLiteral("geo:52.503,13.41")));
}

#include "moc_addresstest.cpp"
