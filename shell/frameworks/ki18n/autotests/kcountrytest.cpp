/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KCountry>
#include <KCountrySubdivision>

#include <QObject>
#include <QStandardPaths>
#include <QTest>

using namespace Qt::Literals;

void initEnvironment()
{
    qputenv("LANG", "fr_CH.UTF-8");
    QStandardPaths::setTestModeEnabled(true);
}

Q_CONSTRUCTOR_FUNCTION(initEnvironment)

class KCountryTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testEmpty()
    {
        KCountry c;
        QVERIFY(!c.isValid());
        QVERIFY(c.alpha2().isEmpty());
        QVERIFY(c.alpha3().isEmpty());
        QVERIFY(c.name().isEmpty());
        QVERIFY(c.emojiFlag().isEmpty());
        QVERIFY(c.subdivisions().isEmpty());
        QCOMPARE(c.country(), QLocale::AnyCountry);
        QVERIFY(c.currencyCode().isEmpty());
        QVERIFY(c.timeZoneIds().isEmpty());
    }

    void testLookup()
    {
        auto c = KCountry::fromAlpha2(u"NZ");
        QVERIFY(c.isValid());
        QCOMPARE(c.alpha2(), QLatin1String("NZ"));
        QCOMPARE(c.alpha3(), QLatin1String("NZL"));
        QCOMPARE(c.name(), QStringLiteral("Nouvelle-Zélande"));
        QCOMPARE(c.emojiFlag(), QStringLiteral("🇳🇿"));
        QCOMPARE(c.country(), QLocale::NewZealand);
        QCOMPARE(c.currencyCode(), QLatin1String("NZD"));

        c = KCountry::fromAlpha2("nz");
        QVERIFY(c.isValid());
        QCOMPARE(c.alpha2(), QLatin1String("NZ"));

        c = KCountry::fromAlpha3(u"NZL");
        QVERIFY(c.isValid());
        QCOMPARE(c.alpha2(), QLatin1String("NZ"));

        QVERIFY(!KCountry::fromAlpha2(QString()).isValid());
        QVERIFY(!KCountry::fromAlpha2(u"ZZ").isValid());
        QVERIFY(!KCountry::fromAlpha2(u"N").isValid());
        QVERIFY(!KCountry::fromAlpha2(u"NZL").isValid());
        QVERIFY(!KCountry::fromAlpha2(u"42").isValid());
        QVERIFY(!KCountry::fromAlpha2(nullptr).isValid());
        QVERIFY(!KCountry::fromAlpha2("N").isValid());
        QVERIFY(!KCountry::fromAlpha2("nzl").isValid());
        QVERIFY(!KCountry::fromAlpha2("\0\0").isValid());

        QVERIFY(!KCountry::fromAlpha3(u"ZZZ").isValid());
        QVERIFY(!KCountry::fromAlpha3(QString()).isValid());
        QVERIFY(!KCountry::fromAlpha3(u"NZ").isValid());
        QVERIFY(!KCountry::fromAlpha3(u"NEWZL").isValid());
        QVERIFY(!KCountry::fromAlpha3(u"123").isValid());
        QVERIFY(!KCountry::fromAlpha3(nullptr).isValid());
        QVERIFY(!KCountry::fromAlpha3("NZ").isValid());
        QVERIFY(!KCountry::fromAlpha3("nzla").isValid());
        QVERIFY(!KCountry::fromAlpha3("\0\0\0").isValid());

        QCOMPARE(KCountry::fromAlpha2(u"nz").alpha2(), QLatin1String("NZ"));
        QCOMPARE(KCountry::fromAlpha2(u"Nz").alpha2(), QLatin1String("NZ"));
        QCOMPARE(KCountry::fromAlpha3(u"nzl").alpha2(), QLatin1String("NZ"));
        QCOMPARE(KCountry::fromAlpha3(u"NzL").alpha2(), QLatin1String("NZ"));

        c = KCountry::fromQLocale(QLocale::NewZealand);
        QVERIFY(c.isValid());
        QCOMPARE(c.alpha2(), QLatin1String("NZ"));

        QVERIFY(!KCountry::fromQLocale(QLocale::AnyCountry).isValid());
        QVERIFY(!KCountry::fromQLocale(QLocale::World).isValid());
    }

    void benchmarkLookup()
    {
        QBENCHMARK {
            const auto c = KCountry::fromAlpha2(u"NZ");
            QVERIFY(c.isValid());
        }
    }

    void testList()
    {
        const auto l = KCountry::allCountries();
        QVERIFY(l.size() > 200);
        for (const auto &c : l) {
            QVERIFY(c.isValid());
            QVERIFY(!c.alpha2().isEmpty());
            QVERIFY(!c.alpha3().isEmpty());
            QVERIFY(!c.name().isEmpty());
            QVERIFY(!c.emojiFlag().isEmpty());
            QVERIFY(c.country() != QLocale::AnyCountry);
        }
    }

    void testTimezone()
    {
        auto tzs = KCountry::fromAlpha2("BE").timeZoneIds();
        QCOMPARE(tzs.size(), 1);
        QCOMPARE(tzs.at(0), "Europe/Brussels");

        tzs = KCountry::fromAlpha2("DE").timeZoneIds();
        QCOMPARE(tzs.size(), 1); // we don't want Europe/Busingen as well here
        QCOMPARE(tzs.at(0), "Europe/Berlin");

        tzs = KCountry::fromAlpha2("ES").timeZoneIds();
        QCOMPARE(tzs.size(), 3);
        QCOMPARE(tzs.at(0), "Europe/Madrid");
        QCOMPARE(tzs.at(1), "Africa/Ceuta");
        QCOMPARE(tzs.at(2), "Atlantic/Canary");
    }

    void testFromLocation_data()
    {
        QTest::addColumn<float>("lat");
        QTest::addColumn<float>("lon");
        QTest::addColumn<QString>("country");
        QTest::addColumn<bool>("canBeConflict"); // for test close to the border, no result is acceptable, a wrong one isn't

        QTest::newRow("invalid") << 400.0f << 25.0f << QString() << false;
        QTest::newRow("out-of-coverage") << -90.0f << 0.0f << QString() << false;

        // basic checks in all quadrants
        QTest::newRow("BR") << -8.0f << -35.0f << QStringLiteral("BR") << false;
        QTest::newRow("CA") << 44.0f << -79.5f << QStringLiteral("CA") << false;
        QTest::newRow("DE") << 52.4f << 13.1f << QStringLiteral("DE") << false;
        QTest::newRow("NZ") << -36.5f << 175.0f << QStringLiteral("NZ") << false;

        // important KDE locations
        QTest::newRow("Randa") << 46.0998f << 7.781469f << QStringLiteral("CH") << false;

        // Maastricht (NL), very close to the BE border
        QTest::newRow("Maastricht") << 50.8505f << 5.6881f << QStringLiteral("NL") << true;
        // Aachen, at the BE/DE/NL corner
        QTest::newRow("Aachen") << 50.7717f << 6.04235f << QStringLiteral("DE") << true;
        // Geneva (CH), very close to the FR border
        QTest::newRow("Geneva") << 46.23213f << 6.10636f << QStringLiteral("CH") << true;
        // Busingen (DE), enclosed by CH
        QTest::newRow("Busingen") << 47.69947f << 8.68833f << QStringLiteral("DE") << true;
        // Tijuana (MX), close to US
        QTest::newRow("Tijuana") << 32.54274f << -116.97505f << QStringLiteral("MX") << true;

        // Baarle, the ultimate special case, NL/BE differs house by house
        QTest::newRow("you-got-to-be-kidding-me") << 51.44344f << 4.93373f << QString() << false;
    }

    void testFromLocation()
    {
        QFETCH(float, lat);
        QFETCH(float, lon);
        QFETCH(QString, country);
        QFETCH(bool, canBeConflict);

        auto c = KCountry::fromLocation(lat, lon);
        if (!canBeConflict || c.isValid()) {
            QCOMPARE(c.alpha2(), country);
        }
    }

    void testFromName()
    {
        QVERIFY(!KCountry::fromName(u"").isValid());
        QVERIFY(!KCountry::fromName(u"Disneyland").isValid());

        QCOMPARE(KCountry::fromName(u"new zealand").alpha2(), QLatin1String("NZ"));
        QCOMPARE(KCountry::fromName(u"Nouvelle-Zélande").alpha2(), QLatin1String("NZ"));
        QCOMPARE(KCountry::fromName(u"NEUSEELAND").alpha2(), QLatin1String("NZ"));

        // diacritic normalization
        QCOMPARE(KCountry::fromName(u"Österreich").alpha2(), QLatin1String("AT"));
        QCOMPARE(KCountry::fromName(u"Østrig").alpha2(), QLatin1String("AT"));
        QCOMPARE(KCountry::fromName(u"osterreich").alpha2(), QLatin1String("AT"));
        QCOMPARE(KCountry::fromName(u"대한민국").alpha2(), QLatin1String("KR"));

        // unique prefix/suffix
        QCOMPARE(KCountry::fromName(u"United States").alpha2(), QLatin1String("US"));
        QCOMPARE(KCountry::fromName(u"Ünīted  States\nOf Amérìcâ").alpha2(), QLatin1String("US"));
        QCOMPARE(KCountry::fromName(u"Arab Emirates").alpha2(), QLatin1String("AE"));
        QCOMPARE(KCountry::fromName(u"United").alpha2(), QString());
        QCOMPARE(KCountry::fromName(u"Bundesrepuplik Deutschland").alpha2(), QLatin1String("DE"));

        // extremely short Vietnamese country names (those are special as they are affected by diacritic stripping, unlike other Asian scripts)
        QCOMPARE(KCountry::fromName(u"Ý").alpha2(), "IT"_L1);
        QCOMPARE(KCountry::fromName(u"Áo").alpha2(), "AT"_L1);
        QCOMPARE(KCountry::fromName(u"Nước Ý").alpha2(), "IT"_L1);

        // code fallbacks
        QCOMPARE(KCountry::fromName(u"USA").alpha2(), QLatin1String("US"));

        // Turkey is no longer recognized as Türkiye, but should not result in a mis-detection of anything else either
        QCOMPARE(KCountry::fromName(u"Turkey").alpha2(), QString());

        // Ambigous substrings of multiple countries
        QCOMPARE(KCountry::fromName(u"Korea").alpha2(), QString());

        // input that shouldn't match anything
        QCOMPARE(KCountry::fromName(u"A").alpha2(), QString());
        // Philippine region previously mis-detected as Palau (PW)
        QCOMPARE(KCountry::fromName(u"Palawan").alpha2(), QString());
    }

    void benchmarkFromName()
    {
        QBENCHMARK {
            QCOMPARE(KCountry::fromName(u"Nouvelle-Zélande").alpha2(), QLatin1String("NZ"));
        }
    }
};

QTEST_GUILESS_MAIN(KCountryTest)

#include "kcountrytest.moc"
