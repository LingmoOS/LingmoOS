/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "addressformat.h"
#include "addressformatparser.cpp"
#include "addressformatscript.cpp"
#include "addressformatter.cpp"

#include <KConfig>
#include <KConfigGroup>
#include <KCountry>

#include <QObject>
#include <QTest>

using namespace KContacts;

void initLocale()
{
    qputenv("LANGUAGE", "en_US");
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class AddressFormatTest : public QObject
{
    Q_OBJECT
private:
    AddressFormat parseFormat(QStringView f, QStringView u, QStringView country)
    {
        AddressFormat format;
        auto fmt = AddressFormatPrivate::get(format);
        fmt->upper = AddressFormatParser::parseFields(u);
        fmt->elements = AddressFormatParser::parseElements(f);
        fmt->country = country.toString();
        return format;
    }

private Q_SLOTS:
    void testRepostory()
    {
        auto fmt = AddressFormatRepository::formatForCountry(QStringLiteral("CA"), AddressFormatScriptPreference::Local);
        QCOMPARE(fmt.requiredFields(),
                 AddressFormatField::StreetAddress | AddressFormatField::Locality | AddressFormatField::PostalCode | AddressFormatField::Region);
        QCOMPARE(fmt.upperCaseFields(),
                 AddressFormatField::StreetAddress | AddressFormatField::Locality | AddressFormatField::PostalCode | AddressFormatField::Region
                     | AddressFormatField::Name | AddressFormatField::Organization | AddressFormatField::Country);
        QCOMPARE(fmt.usedFields(),
                 AddressFormatField::StreetAddress | AddressFormatField::Locality | AddressFormatField::PostalCode | AddressFormatField::Region
                     | AddressFormatField::Name | AddressFormatField::Organization | AddressFormatField::PostOfficeBox);
        const auto &elems = fmt.elements();
        QCOMPARE(elems.size(), 14);

        QCOMPARE(elems[0].isField(), true);
        QCOMPARE(elems[0].field(), AddressFormatField::Name);
        QCOMPARE(elems[1].isSeparator(), true);
        QCOMPARE(elems[2].isField(), true);
        QCOMPARE(elems[2].field(), AddressFormatField::Organization);
        QCOMPARE(elems[3].isSeparator(), true);
        QCOMPARE(elems[6].isLiteral(), true);
        QCOMPARE(elems[6].literal(), QLatin1String("PO BOX "));
        QCOMPARE(elems[9].isField(), true);
        QCOMPARE(elems[9].field(), AddressFormatField::Locality);
        QCOMPARE(elems[10].isLiteral(), true);
        QCOMPARE(elems[10].literal(), QLatin1String(" "));
        QCOMPARE(elems[11].isField(), true);
        QCOMPARE(elems[11].field(), AddressFormatField::Region);
        QCOMPARE(elems[12].isLiteral(), true);
        QCOMPARE(elems[12].literal(), QLatin1String(" "));
        QCOMPARE(elems[13].isField(), true);
        QCOMPARE(elems[13].field(), AddressFormatField::PostalCode);

        QVERIFY(!fmt.postalCodeRegularExpression().isEmpty());
        QCOMPARE(fmt.country(), QLatin1String("CA"));
    }

    void testFormatStyles()
    {
        Address addr;
        addr.setCountry(QStringLiteral("DE"));
        addr.setRegion(QStringLiteral("BE"));
        addr.setLocality(QStringLiteral("Berlin"));
        addr.setPostalCode(QStringLiteral("10969"));
        addr.setStreet(QStringLiteral("Prinzenstraße 85 F"));
        addr.setExtended(QStringLiteral("East wing"));

        auto fmt = parseFormat(u"%O%n%N%nPostfach %P%n%A%n%Z %C", u"R", u"DE");
        auto s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::Postal);
        QCOMPARE(s, QStringLiteral("KDE e.V.\nDr. Konqi\nEast wing\nPrinzenstraße 85 F\n10969 Berlin\n\nGERMANY"));
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::MultiLineInternational);
        QCOMPARE(s, QStringLiteral("KDE e.V.\nDr. Konqi\nEast wing\nPrinzenstraße 85 F\n10969 Berlin\nGermany"));
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::MultiLineDomestic);
        QCOMPARE(s, QStringLiteral("KDE e.V.\nDr. Konqi\nEast wing\nPrinzenstraße 85 F\n10969 Berlin"));
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::SingleLineInternational);
        QCOMPARE(s, QStringLiteral("KDE e.V., Dr. Konqi, East wing, Prinzenstraße 85 F, 10969 Berlin, Germany"));
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::SingleLineDomestic);
        QCOMPARE(s, QStringLiteral("KDE e.V., Dr. Konqi, East wing, Prinzenstraße 85 F, 10969 Berlin"));
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::GeoUriQuery);
        QCOMPARE(s, QStringLiteral("Prinzenstraße 85 F,10969 Berlin,DE"));

        addr.setExtended(QString());
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::Postal);
        QCOMPARE(s, QStringLiteral("KDE e.V.\nDr. Konqi\nPrinzenstraße 85 F\n10969 Berlin\n\nGERMANY"));
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::MultiLineInternational);
        QCOMPARE(s, QStringLiteral("KDE e.V.\nDr. Konqi\nPrinzenstraße 85 F\n10969 Berlin\nGermany"));
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::MultiLineDomestic);
        QCOMPARE(s, QStringLiteral("KDE e.V.\nDr. Konqi\nPrinzenstraße 85 F\n10969 Berlin"));
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::SingleLineInternational);
        QCOMPARE(s, QStringLiteral("KDE e.V., Dr. Konqi, Prinzenstraße 85 F, 10969 Berlin, Germany"));
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::SingleLineDomestic);
        QCOMPARE(s, QStringLiteral("KDE e.V., Dr. Konqi, Prinzenstraße 85 F, 10969 Berlin"));
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::GeoUriQuery);
        QCOMPARE(s, QStringLiteral("Prinzenstraße 85 F,10969 Berlin,DE"));

        addr.setPostOfficeBox(QStringLiteral("42"));
        addr.setStreet(QString());
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::Postal);
        QCOMPARE(s, QStringLiteral("KDE e.V.\nDr. Konqi\nPostfach 42\n10969 Berlin\n\nGERMANY"));
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::MultiLineInternational);
        QCOMPARE(s, QStringLiteral("KDE e.V.\nDr. Konqi\nPostfach 42\n10969 Berlin\nGermany"));
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::MultiLineDomestic);
        QCOMPARE(s, QStringLiteral("KDE e.V.\nDr. Konqi\nPostfach 42\n10969 Berlin"));
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::SingleLineInternational);
        QCOMPARE(s, QStringLiteral("KDE e.V., Dr. Konqi, Postfach 42, 10969 Berlin, Germany"));
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::SingleLineDomestic);
        QCOMPARE(s, QStringLiteral("KDE e.V., Dr. Konqi, Postfach 42, 10969 Berlin"));
        s = AddressFormatter::format(addr, QStringLiteral("Dr. Konqi"), QStringLiteral("KDE e.V."), fmt, AddressFormatStyle::GeoUriQuery);
        QCOMPARE(s, QStringLiteral("10969 Berlin,DE"));
    }

    void testFormatScript()
    {
        KContacts::Address address;
        address.setCountry(QStringLiteral("JP"));
        address.setRegion(QStringLiteral("東京"));
        address.setLocality(QStringLiteral("都港区"));
        address.setPostalCode(QStringLiteral("106-0047"));
        address.setStreet(QStringLiteral("南麻布 4-6-28"));
        auto fmt = parseFormat(u"〒%Z%n%S%C%A%n%O%n%N", u"SR", u"JP");
        auto s = AddressFormatter::format(address, QString(), QString(), fmt, AddressFormatStyle::SingleLineDomestic);
        QCOMPARE(s, QStringLiteral("〒106-0047東京都港区南麻布 4-6-28"));

        address = {};
        address.setCountry(QStringLiteral("EG"));
        address.setLocality(QStringLiteral("القاهرة"));
        address.setStreet(QStringLiteral("2005ج كورنيش النيل، رملة بولاق"));
        fmt = parseFormat(u"%N%n%O%n%A%n%C%n%S%n%Z", u"", u"EG");
        s = AddressFormatter::format(address, QString(), QString(), fmt, AddressFormatStyle::SingleLineDomestic);
        QCOMPARE(s, QStringLiteral("2005ج كورنيش النيل، رملة بولاق، القاهرة"));

        address = {};
        address.setCountry(QStringLiteral("KR"));
        address.setRegion(QStringLiteral("서울특별시"));
        address.setLocality(QStringLiteral("중구"));
        address.setPostalCode(QStringLiteral("04637"));
        address.setStreet(QStringLiteral("한강대로 416"));
        fmt = parseFormat(u"%R%n%S %C%D%n%A%n%O%n%N%n%Z", u"Z", u"KR");
        s = AddressFormatter::format(address, QString(), QString(), fmt, AddressFormatStyle::SingleLineDomestic);
        QCOMPARE(s, QStringLiteral("서울특별시 중구 한강대로 416 04637"));
        s = AddressFormatter::format(address, QString(), QString(), fmt, AddressFormatStyle::MultiLineDomestic);
        QCOMPARE(s, QStringLiteral("서울특별시 중구\n한강대로 416\n04637"));
        // the weird language mix is because we are in the wrong locale for KR here,
        // the important part is that the country is in the front when using Hangul
        s = AddressFormatter::format(address, QString(), QString(), fmt, AddressFormatStyle::SingleLineInternational);
        QCOMPARE(s, QStringLiteral("Korea, Republic of 서울특별시 중구 한강대로 416 04637"));
        s = AddressFormatter::format(address, QString(), QString(), fmt, AddressFormatStyle::MultiLineInternational);
        QCOMPARE(s, QStringLiteral("Korea, Republic of\n서울특별시 중구\n한강대로 416\n04637"));

        // reverse order address lines
        address.setExtended(QStringLiteral("서울스퀘어 11층"));
        s = AddressFormatter::format(address, QString(), QString(), fmt, AddressFormatStyle::SingleLineDomestic);
        QCOMPARE(s, QStringLiteral("서울특별시 중구 한강대로 416 서울스퀘어 11층 04637"));
        s = AddressFormatter::format(address, QString(), QString(), fmt, AddressFormatStyle::MultiLineDomestic);
        QCOMPARE(s, QStringLiteral("서울특별시 중구\n한강대로 416\n서울스퀘어 11층\n04637"));
    }

    void testIncompleteAddress()
    {
        KContacts::Address address;
        address.setLocality(QStringLiteral("City"));
        address.setPostalCode(QStringLiteral("XX 12345")); // region wrongly part of postal code field

        auto fmt = parseFormat(u"%C, %S %Z", u"", u"US");
        auto s = AddressFormatter::format(address, QString(), QString(), fmt, AddressFormatStyle::MultiLineDomestic);
        QCOMPARE(s, QLatin1String("City XX 12345"));
    }

    void validateRepository()
    {
        KConfig config(QStringLiteral(":/org.kde.kcontacts/addressformatrc"), KConfig::SimpleConfig);
        const auto groups = config.groupList();

        for (const auto &countryCode : groups) {
            // qDebug() << countryCode;
            QVERIFY(KCountry::fromAlpha2(countryCode).isValid());
            const auto group = config.group(countryCode);

            QVERIFY(group.hasKey("AddressFormat"));
            const auto upper = AddressFormatParser::parseFields(group.readEntry("Upper"));
            const auto required = AddressFormatParser::parseFields(group.readEntry("Required"));

            // TODO can we check all language variants as well?
            for (const auto &key : {"AddressFormat", "BusinessAddressFormat", "LatinAddressFormat", "LatinBusinessAddressFormat"}) {
                if (!group.hasKey(key)) {
                    continue;
                }
                const auto elems = AddressFormatParser::parseElements(group.readEntry(key, QString()));

                AddressFormatFields seen = AddressFormatField::NoField;
                for (const auto &elem : elems) {
                    if (!elem.isField()) {
                        continue;
                    }
                    QCOMPARE(seen & elem.field() & ~(int)AddressFormatField::SortingCode, 0);
                    seen |= elem.field();
                }

                QCOMPARE(seen & required, required);
                QCOMPARE(seen & upper & ~(int)AddressFormatField::Country, upper & ~(int)AddressFormatField::Country);
            }

            const auto zipRegEx = group.readEntry("PostalCodeFormat", QString());
            if (!zipRegEx.isEmpty()) {
                QVERIFY(QRegularExpression(zipRegEx).isValid());
            }
        }
    }
};

QTEST_GUILESS_MAIN(AddressFormatTest)

#include "addressformattest.moc"
