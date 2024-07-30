/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KCountry>
#include <KTimeZone>

#include <QObject>
#include <QStandardPaths>
#include <QTest>

#include <cmath>

void initEnvironment()
{
    qputenv("LANG", "fr_CH");
    QStandardPaths::setTestModeEnabled(true);
}

Q_CONSTRUCTOR_FUNCTION(initEnvironment)

class KTimeZoneTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFromLocation_data()
    {
        QTest::addColumn<float>("lat");
        QTest::addColumn<float>("lon");
        QTest::addColumn<QString>("tz");
        QTest::addColumn<QString>("tzAlt"); // alternative equivalent answer in border regions

        QTest::newRow("invalid") << NAN << NAN << QString() << QString();
        QTest::newRow("out-of-coverage") << -90.0f << 180.0f << QString() << QString();

        // basic checks in all quadrants
        QTest::newRow("BR") << -8.0f << -35.0f << QStringLiteral("America/Recife") << QString();
        QTest::newRow("CA") << 44.0f << -79.5f << QStringLiteral("America/Toronto") << QString();
        QTest::newRow("DE") << 52.4f << 13.1f << QStringLiteral("Europe/Berlin") << QString();
        QTest::newRow("NZ") << -36.5f << 175.0f << QStringLiteral("Pacific/Auckland") << QString();

        // important KDE locations
        QTest::newRow("Randa") << 46.0998f << 7.781469f << QStringLiteral("Europe/Zurich") << QString();

        // Special case: Northern Vietnam has a Thai timezone
        QTest::newRow("VN") << 21.0f << 106.0f << QStringLiteral("Asia/Bangkok") << QString();

        // Eliat Airport (IL), close to JO, and with a minor timezone variation due to different weekends
        QTest::newRow("Eliat") << 29.72530f << 35.00598f << QString() << QStringLiteral("Asia/Jerusalem");

        // Cordoba (AR), AR has several sub-zones that are all equivalent
        QTest::newRow("AR") << -31.4f << -64.2f << QStringLiteral("America/Argentina/Buenos_Aires") << QStringLiteral("America/Argentina/Cordoba");

        // Maastricht (NL), very close to the BE border
        QTest::newRow("Maastricht") << 50.8505f << 5.6881f << QStringLiteral("Europe/Amsterdam") << QStringLiteral("Europe/Brussels");
        // Aachen, at the BE/DE/NL corner
        QTest::newRow("Aachen") << 50.7717f << 6.04235f << QStringLiteral("Europe/Berlin") << QStringLiteral("Europe/Brussels");
        // Geneva (CH), very close to the FR border
        QTest::newRow("Geneva") << 46.23213f << 6.10636f << QStringLiteral("Europe/Zurich") << QStringLiteral("Europe/Paris");
        // Busingen (DE), enclosed by CH
        QTest::newRow("Busingen") << 47.69947f << 8.68833f << QStringLiteral("Europe/Zurich") << QStringLiteral("Europe/Berlin");
        // Tijuana (MX), close to US
        QTest::newRow("Tijuana") << 32.54274f << -116.97505f << QStringLiteral("America/Tijuana") << QStringLiteral("America/Los_Angeles");

        // Baarle, the ultimate special case, NL/BE differs house by house
        QTest::newRow("Baarle") << 51.44344f << 4.93373f << QStringLiteral("Europe/Amsterdam") << QStringLiteral("Europe/Brussels");
    }

    void testFromLocation()
    {
        QFETCH(float, lat);
        QFETCH(float, lon);
        QFETCH(QString, tz);
        QFETCH(QString, tzAlt);

        const auto zoneId = QString::fromUtf8(KTimeZone::fromLocation(lat, lon));
        if (tzAlt.isEmpty()) {
            QCOMPARE(zoneId, tz);
        } else {
            qDebug() << zoneId;
            QVERIFY(zoneId == tz || zoneId == tzAlt);
        }
    }

    void testCountry()
    {
        // invalid
        QCOMPARE(KTimeZone::country(nullptr).alpha2(), QString());
        QCOMPARE(KTimeZone::country("").alpha2(), QString());
        QCOMPARE(KTimeZone::country("Moon/Dark_Side").alpha2(), QString());

        // 1:1 mapping
        QCOMPARE(KTimeZone::country("Europe/Brussels").alpha2(), QLatin1String("BE"));
        // timezones none of our methods would return
        QCOMPARE(KTimeZone::country("America/Argentina/Cordoba").alpha2(), QLatin1String("AR"));
        QCOMPARE(KTimeZone::country("Europe/Busingen").alpha2(), QLatin1String("DE"));
        // 1:N mapping
        QCOMPARE(KTimeZone::country("America/Toronto").alpha2(), QLatin1String("CA"));
        QCOMPARE(KTimeZone::country("Atlantic/Canary").alpha2(), QLatin1String("ES"));
        // ambiguous (also used in northern Vietnam)
        QCOMPARE(KTimeZone::country("Asia/Bangkok").alpha2(), QString());
    }
};

QTEST_GUILESS_MAIN(KTimeZoneTest)

#include "ktimezonetest.moc"
