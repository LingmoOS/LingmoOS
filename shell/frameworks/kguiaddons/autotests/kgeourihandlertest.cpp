/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../src/geo-scheme-handler/kgeourihandler.cpp"

#include <QTest>

class KGeoUriHandlerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testHandler_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("output");

        QTest::newRow("empty") << QString() << QStringLiteral("https://openstreetmap.org");
        QTest::newRow("incomplete-1") << QStringLiteral("geo:") << QStringLiteral("https://openstreetmap.org");
        QTest::newRow("incomplete-2") << QStringLiteral("geo:46.1") << QStringLiteral("https://openstreetmap.org");
        QTest::newRow("broken-1") << QStringLiteral("geo:a,b") << QStringLiteral("https://openstreetmap.org");
        QTest::newRow("broken-2") << QStringLiteral("geo:46.1;7.783") << QStringLiteral("https://openstreetmap.org");
        QTest::newRow("lat-out-of-range-1") << QStringLiteral("geo:91.0;-1.0") << QStringLiteral("https://openstreetmap.org");
        QTest::newRow("lat-out-of-range-2") << QStringLiteral("geo:-91.0;1.0") << QStringLiteral("https://openstreetmap.org");
        QTest::newRow("lon-out-of-range-1") << QStringLiteral("geo:1.0;181.0") << QStringLiteral("https://openstreetmap.org");
        QTest::newRow("lon-out-of-range-2") << QStringLiteral("geo:-1.0;-181.0") << QStringLiteral("https://openstreetmap.org");

        QTest::newRow("2d-coord-only") << QStringLiteral("geo:46.1,7.783") << QStringLiteral("https://www.openstreetmap.org/#map=18/46.1/7.783");
        QTest::newRow("3d-coord") << QStringLiteral("geo:46.1,7.783,1600") << QStringLiteral("https://www.openstreetmap.org/#map=18/46.1/7.783");
        QTest::newRow("2d-coord-with-uncertainty") << QStringLiteral("geo:46.1,7.783;u=100")
                                                   << QStringLiteral("https://www.openstreetmap.org/#map=18/46.1/7.783");
        QTest::newRow("2d-coord-with-z") << QStringLiteral("geo:46.1,7.783?z=19") << QStringLiteral("https://www.openstreetmap.org/#map=19/46.1/7.783");
        QTest::newRow("negative-coord") << QStringLiteral("geo:-34.59,-58.375") << QStringLiteral("https://www.openstreetmap.org/#map=18/-34.59/-58.375");

        QTest::newRow("query") << QStringLiteral("geo:0,0?q=Randa") << QStringLiteral("https://www.openstreetmap.org/search?query=Randa");
        QTest::newRow("query-with-coord") << QStringLiteral("geo:46.1,7.783?q=Randa") << QStringLiteral("https://www.openstreetmap.org/search?query=Randa");

        // explicit coordinate reference systems
        QTest::newRow("WGS84") << QStringLiteral("geo:37.78,-122.4;u=35;crs=wgs84") << QStringLiteral("https://www.openstreetmap.org/#map=18/37.78/-122.4");
        QTest::newRow("EPSG:32618") << QStringLiteral("geo:323482,4306480;crs=EPSG:32618;u=20") << QStringLiteral("https://openstreetmap.org");
        QTest::newRow("moon") << QStringLiteral("geo:37.786971,-122.399677;crs=Moon-2011;u=35") << QStringLiteral("https://openstreetmap.org");
    }

    void testHandler()
    {
        QFETCH(QString, input);
        QFETCH(QString, output);

        KGeoUriHandler handler;
        handler.setCoordinateTemplate(QStringLiteral("https://www.openstreetmap.org/#map=<Z>/<LAT>/<LON>"));
        handler.setQueryTemplate(QStringLiteral("https://www.openstreetmap.org/search?query=<Q>"));
        handler.setFallbackUrl(QStringLiteral("https://openstreetmap.org"));
        QCOMPARE(handler.handleUri(QUrl(input)), output);
    }
};

QTEST_APPLESS_MAIN(KGeoUriHandlerTest)

#include "kgeourihandlertest.moc"
