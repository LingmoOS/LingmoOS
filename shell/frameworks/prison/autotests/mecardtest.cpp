/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#include <QtTest/QTest>

#include "mecard.h"

using namespace Prison;

class MeCardTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testWifiParse()
    {
        QVariantMap map;

        {
            auto m = MeCard::parse(QStringLiteral("WIFI:S:myssid;T:WPA;P:xxx123;;"));
            QVERIFY(m.has_value());
            QCOMPARE(m->header(), QLatin1String("WIFI"));
            QCOMPARE(m->headerView(), QLatin1String("WIFI"));
            QCOMPARE(m->value(u"S"), QLatin1String("myssid"));
            QCOMPARE(m->value(u"T"), QLatin1String("WPA"));
            QCOMPARE(m->value(u"P"), QLatin1String("xxx123"));
            map = QVariantMap(
                {{QLatin1String("S"), QLatin1String("myssid")}, {QLatin1String("T"), QLatin1String("WPA")}, {QLatin1String("P"), QLatin1String("xxx123")}});
            QCOMPARE(m->toVariantMap(), map);
        }

        {
            auto m = MeCard::parse(QStringLiteral("MECARD:N:Doe,John;TEL:13035551212;EMAIL:john.doe@example.com;;"));
            QVERIFY(m.has_value());
            QCOMPARE(m->header(), QLatin1String("MECARD"));
            QCOMPARE(m->headerView(), QLatin1String("MECARD"));
            QCOMPARE(m->value(u"N"), QLatin1String("Doe,John"));
            QCOMPARE(m->value(u"TEL"), QLatin1String("13035551212"));
            QCOMPARE(m->value(u"EMAIL"), QLatin1String("john.doe@example.com"));
            map = QVariantMap({{QLatin1String("N"), QLatin1String("Doe,John")},
                               {QLatin1String("TEL"), QLatin1String("13035551212")},
                               {QLatin1String("EMAIL"), QLatin1String("john.doe@example.com")}});
            QCOMPARE(m->toVariantMap(), map);
        }

        {
            auto m = MeCard::parse(QStringLiteral("MECARD:N:Doe,John;TEL:13035551212;EMAIL:john.doe@example.com;EMAIL:null@kde.org;;"));
            QVERIFY(m.has_value());
            QCOMPARE(m->header(), QLatin1String("MECARD"));
            QCOMPARE(m->headerView(), QLatin1String("MECARD"));
            QCOMPARE(m->value(u"N"), QLatin1String("Doe,John"));
            QCOMPARE(m->value(u"TEL"), QLatin1String("13035551212"));
            QCOMPARE(m->value(u"EMAIL"), QString());
            QCOMPARE(m->values(u"EMAIL"), QStringList({QLatin1String("john.doe@example.com"), QLatin1String("null@kde.org")}));
            map = QVariantMap({{QLatin1String("N"), QLatin1String("Doe,John")},
                               {QLatin1String("TEL"), QLatin1String("13035551212")},
                               {QLatin1String("EMAIL"), QString()},
                               {QLatin1String("EMAIL"), QStringList({QLatin1String("john.doe@example.com"), QLatin1String("null@kde.org")})}});
            QCOMPARE(m->toVariantMap(), map);
        }

        {
            auto m = MeCard::parse(QStringLiteral("WIFI:S:\\\"foo\\;bar\\\\baz\\\";P:\"ABCD\";;"));
            QVERIFY(m.has_value());
            QCOMPARE(m->value(u"P"), QLatin1String("ABCD"));
            QCOMPARE(m->value(u"S"), QLatin1String("\"foo;bar\\baz\""));
            map = QVariantMap({{QLatin1String("P"), QLatin1String("ABCD")}, {QLatin1String("S"), QLatin1String("\"foo;bar\\baz\"")}});
            QCOMPARE(m->toVariantMap(), map);
        }
    }

    void testInvalid_data()
    {
        QTest::addColumn<QString>("input");
        QTest::newRow("empty") << QString();
        QTest::newRow("no colon") << QStringLiteral("wifi");
        QTest::newRow("no content 1") << QStringLiteral("wifi:");
        QTest::newRow("no content 2") << QStringLiteral("wifi:;");
        QTest::newRow("no content 3") << QStringLiteral("wifi:;;");
        QTest::newRow("invalid element") << QStringLiteral("wifi:S:");
    }
    void testInvalid()
    {
        QFETCH(QString, input);
        auto m = MeCard::parse(input);
        QVERIFY(!m.has_value());
    }
};

QTEST_APPLESS_MAIN(MeCardTest)

#include "mecardtest.moc"
