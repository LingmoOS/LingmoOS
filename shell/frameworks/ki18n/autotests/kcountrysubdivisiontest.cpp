/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KCountry>
#include <KCountrySubdivision>

#include <QObject>
#include <QStandardPaths>
#include <QTest>

void initEnvironment()
{
    qputenv("LANG", "fr_CH.UTF-8");
    QStandardPaths::setTestModeEnabled(true);
}

Q_CONSTRUCTOR_FUNCTION(initEnvironment)

class KCountrySubdivisionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testEmpty()
    {
        KCountrySubdivision s;
        QVERIFY(!s.isValid());
        QVERIFY(!s.country().isValid());
        QVERIFY(s.code().isEmpty());
        QVERIFY(!s.parent().isValid());
        QVERIFY(s.subdivisions().isEmpty());
        QVERIFY(s.timeZoneIds().isEmpty());
    }

    void testLookup()
    {
        auto s = KCountrySubdivision::fromCode(u"DE-BE");
        QVERIFY(s.isValid());
        QCOMPARE(s.country().alpha2(), QLatin1String("DE"));
        QVERIFY(!s.parent().isValid());
        QCOMPARE(s.name(), QStringLiteral("Berlin"));
        QCOMPARE(s.code(), QLatin1String("DE-BE"));

        s = KCountrySubdivision::fromCode("AT-9");
        QVERIFY(s.isValid());
        QCOMPARE(s.country().alpha2(), QLatin1String("AT"));
        QVERIFY(!s.parent().isValid());
        QCOMPARE(s.name(), QStringLiteral("Vienne"));
        QCOMPARE(s.code(), QLatin1String("AT-9"));

        s = KCountrySubdivision::fromCode(u"FR-ARA");
        QVERIFY(s.isValid());
        QCOMPARE(s.country().alpha2(), QLatin1String("FR"));
        QVERIFY(!s.parent().isValid());
        QCOMPARE(s.name(), QStringLiteral("Auvergne-Rhône-Alpes"));
        QCOMPARE(s.code(), QLatin1String("FR-ARA"));
        QCOMPARE(KCountrySubdivision::fromCode(u"FR-aRa"), s);

        s = KCountrySubdivision::fromCode(u"CZ-20A");
        QVERIFY(s.isValid());
        QCOMPARE(s.country().alpha2(), QLatin1String("CZ"));
        QCOMPARE(s.code(), QLatin1String("CZ-20A"));

        s = s.parent();
        QVERIFY(s.isValid());
        QCOMPARE(s.country().alpha2(), QLatin1String("CZ"));
        QVERIFY(!s.parent().isValid());
        QCOMPARE(s.code(), QLatin1String("CZ-20"));
        QCOMPARE(KCountrySubdivision::fromCode("cz-20"), s);

        s = KCountrySubdivision::fromCode("us-or");
        QVERIFY(s.isValid());
        QCOMPARE(s.country().alpha2(), QLatin1String("US"));
        QVERIFY(!s.parent().isValid());
        QCOMPARE(s.code(), QLatin1String("US-OR"));
        QCOMPARE(s.name(), QLatin1String("Oregon"));

        QVERIFY(!KCountrySubdivision::fromCode(u"ZZ-ABC").isValid());
        QVERIFY(!KCountrySubdivision::fromCode("NZ-999").isValid());
        QVERIFY(!KCountrySubdivision::fromCode("AT-9-9").isValid());
        QVERIFY(!KCountrySubdivision::fromCode("").isValid());
        QVERIFY(!KCountrySubdivision::fromCode(nullptr).isValid());
    }

    void testSubList()
    {
        auto s = KCountrySubdivision::fromCode(u"CZ-20");
        QVERIFY(s.isValid());
        const auto l = s.subdivisions();
        QVERIFY(l.size() > 10);
        for (const auto &sub : l) {
            QVERIFY(sub.isValid());
            QCOMPARE(sub.parent(), s);
            QCOMPARE(sub.country().alpha2(), QLatin1String("CZ"));
        }

        s = KCountrySubdivision::fromCode(u"DE-BE");
        QCOMPARE(s.subdivisions().size(), 0);
    }

    void testTopList()
    {
        auto c = KCountry::fromAlpha2(u"CZ");
        QVERIFY(c.isValid());
        auto l = c.subdivisions();
        QVERIFY(l.size() > 10);
        for (const auto &s : l) {
            QVERIFY(s.isValid());
            QVERIFY(!s.parent().isValid());
            QCOMPARE(s.country(), c);
        }

        c = KCountry::fromAlpha2(u"DE");
        QVERIFY(c.isValid());
        l = c.subdivisions();
        QCOMPARE(l.size(), 16);
        for (const auto &s : l) {
            QVERIFY(s.isValid());
            QVERIFY(!s.parent().isValid());
            QCOMPARE(s.subdivisions().size(), 0);
            QCOMPARE(s.country(), c);
        }
    }

    void testTimezone()
    {
        auto tzs = KCountrySubdivision::fromCode("DE-BE").timeZoneIds();
        QCOMPARE(tzs.size(), 1);
        QCOMPARE(tzs.at(0), "Europe/Berlin");

        tzs = KCountrySubdivision::fromCode("FR-IDF").timeZoneIds();
        QCOMPARE(tzs.size(), 1);
        QCOMPARE(tzs.at(0), "Europe/Paris");
        tzs = KCountrySubdivision::fromCode("NL-SX").timeZoneIds();
        QCOMPARE(tzs.size(), 1);
        QCOMPARE(tzs.at(0), "America/Lower_Princes");

        tzs = KCountrySubdivision::fromCode("ES-CN").timeZoneIds();
        QCOMPARE(tzs.size(), 1);
        QCOMPARE(tzs.at(0), "Atlantic/Canary");

        tzs = KCountrySubdivision::fromCode("US-OR").timeZoneIds();
        QCOMPARE(tzs.size(), 2);
        QCOMPARE(tzs.at(0), "America/Los_Angeles");
        QCOMPARE(tzs.at(1), "America/Boise");
    }

    void testFromLocation_data()
    {
        QTest::addColumn<float>("lat");
        QTest::addColumn<float>("lon");
        QTest::addColumn<QString>("code");

        QTest::newRow("invalid") << -91.0f << 361.0f << QString();
        QTest::newRow("out-of-coverage") << 90.0f << 0.0f << QString();

        QTest::newRow("US-AK") << 65.0f << -155.0f << QStringLiteral("US-AK");
        QTest::newRow("US-CA") << 37.7f << -122.0f << QStringLiteral("US-CA");
        QTest::newRow("FR-IDF") << 48.7f << 2.5f << QStringLiteral("FR-IDF");
        QTest::newRow("DE-BW") << 48.7f << 9.0f << QStringLiteral("DE-BW");
    }

    void testFromLocation()
    {
        QFETCH(float, lat);
        QFETCH(float, lon);
        QFETCH(QString, code);

        const auto s = KCountrySubdivision::fromLocation(lat, lon);
        QCOMPARE(s.code(), code);
        const auto c = KCountry::fromLocation(lat, lon);
        QCOMPARE(s.country(), c);
    }
};

QTEST_GUILESS_MAIN(KCountrySubdivisionTest)

#include "kcountrysubdivisiontest.moc"
