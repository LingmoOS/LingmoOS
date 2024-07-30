/*
 *  SPDX-FileCopyrightText: 2018 Frederik Gladhorn <gladhorn@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <QCoreApplication>
#include <QObject>
#include <QTest>

#include "../src/edid.h"

using namespace KScreen;

class TestEdid : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testInvalid();
    void testEdidParser_data();
    void testEdidParser();
};

void TestEdid::testInvalid()
{
    QScopedPointer<Edid> e(new Edid());
    QCOMPARE(e->isValid(), false);
    QCOMPARE(e->name(), QString());

    QScopedPointer<Edid> e2(e->clone());
    QCOMPARE(e2->isValid(), false);
    QCOMPARE(e2->name(), QString());

    QScopedPointer<Edid> e3(new Edid("some random data"));
    QCOMPARE(e3->isValid(), false);
}

void TestEdid::testEdidParser_data()
{
    // The raw edid data
    QTest::addColumn<QByteArray>("raw_edid");
    QTest::addColumn<QString>("deviceId");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("pnpId");
    // List of potential vendor names, this depends on the availablility
    // of pnp.ids, otherwise it will be a three letter abbreviation.
    QTest::addColumn<QStringList>("vendor");
    QTest::addColumn<QString>("serial");
    QTest::addColumn<QString>("eisaId");
    QTest::addColumn<QString>("hash");
    QTest::addColumn<uint>("width");
    QTest::addColumn<uint>("height");
    QTest::addColumn<qreal>("gamma");

    QTest::addColumn<QQuaternion>("red");
    QTest::addColumn<QQuaternion>("green");
    QTest::addColumn<QQuaternion>("blue");
    QTest::addColumn<QQuaternion>("white");
    // clang-format off
    QTest::addRow("cor")
        << QByteArray::fromBase64("AP///////wAN8iw0AAAAABwVAQOAHRB4CoPVlFdSjCccUFQAAAABAQEBAQEBAQEBAQEBAQEBEhtWWlAAGTAwIDYAJaQQAAAYEhtWWlAAGTAwIDYAJaQQAAAYAAAA/gBBVU8KICAgICAgICAgAAAA/gBCMTMzWFcwMyBWNCAKAIc=")
        << QStringLiteral("xrandr-Corollary Inc")
        << QStringLiteral("")
        << QStringLiteral("COR")
        << QStringList({QStringLiteral("COR"), QStringLiteral("Corollary Inc")})
        << QStringLiteral("")
        << QStringLiteral("B133XW03 V4")
        << QStringLiteral("82266089b3f9da3a8c48de1ec81b09e1")
        << 29U << 16U << 2.2
        << QQuaternion(1, QVector3D(0.580078, 0.339844, 0))
        << QQuaternion(1, QVector3D(0.320313, 0.549805, 0))
        << QQuaternion(1, QVector3D(0.155273, 0.110352, 0))
        << QQuaternion(1, QVector3D(0.313477, 0.329102, 0));

    QTest::addRow("dell")
        << QByteArray::fromBase64("AP///////wAQrBbwTExLQQ4WAQOANCB46h7Frk80sSYOUFSlSwCBgKlA0QBxTwEBAQEBAQEBKDyAoHCwI0AwIDYABkQhAAAaAAAA/wBGNTI1TTI0NUFLTEwKAAAA/ABERUxMIFUyNDEwCiAgAAAA/QA4TB5REQAKICAgICAgAToCAynxUJAFBAMCBxYBHxITFCAVEQYjCQcHZwMMABAAOC2DAQAA4wUDAQI6gBhxOC1AWCxFAAZEIQAAHgEdgBhxHBYgWCwlAAZEIQAAngEdAHJR0B4gbihVAAZEIQAAHowK0Iog4C0QED6WAAZEIQAAGAAAAAAAAAAAAAAAAAAAPg==")
        << QStringLiteral("xrandr-Dell Inc.-DELL U2410-F525M245AKLL")
        << QStringLiteral("DELL U2410")
        << QStringLiteral("DEL")
        << QStringList({QStringLiteral("DEL"), QStringLiteral("Dell Inc.")})
        << QStringLiteral("F525M245AKLL")
        << QStringLiteral("")
        << QStringLiteral("be55eeb5fcc1e775f321c1ae3aa02ef0")
        << 52U << 32U << 2.2
        << QQuaternion(1, QVector3D(0.679688, 0.308594, 0))
        << QQuaternion(1, QVector3D(0.206055, 0.693359, 0))
        << QQuaternion(1, QVector3D(0.151367, 0.0546875, 0))
        << QQuaternion(1, QVector3D(0.313477, 0.329102, 0));

    QTest::addRow("samsung") << QByteArray::fromBase64("AP///////wBMLcMFMzJGRQkUAQMOMx14Ku6Ro1RMmSYPUFQjCACBAIFAgYCVAKlAswABAQEBAjqAGHE4LUBYLEUA/h8RAAAeAAAA/QA4PB5REQAKICAgICAgAAAA/ABTeW5jTWFzdGVyCiAgAAAA/wBIOU1aMzAyMTk2CiAgAC4=")
        << QStringLiteral("xrandr-Samsung Electric Company-SyncMaster-H9MZ302196")
        << QStringLiteral("SyncMaster")
        << QStringLiteral("SAM")
        << QStringList({QStringLiteral("SAM"), QStringLiteral("Samsung Electric Company")})
        << QStringLiteral("H9MZ302196")
        << QStringLiteral("")
        << QStringLiteral("9384061b2b87ad193f841e07d60e9e1a")
        << 51U << 29U << 2.2
        << QQuaternion(1, QVector3D(0.639648, 0.328125, 0))
        << QQuaternion(1, QVector3D(0.299805, 0.599609, 0))
        << QQuaternion(1, QVector3D(0.150391, 0.0595703, 0))
        << QQuaternion(1, QVector3D(0.3125, 0.329102, 0));

    QTest::newRow("sharp")
        << QByteArray::fromBase64("AP///////wBNEEoUAAAAAB4ZAQSlHRF4Dt5Qo1RMmSYPUFQAAAABAQEBAQEBAQEBAQEBAQEBzZGAoMAINHAwIDUAJqUQAAAYpHSAoMAINHAwIDUAJqUQAAAYAAAA/gBSWE40OYFMUTEzM1oxAAAAAAACQQMoABIAAAsBCiAgAMw=")
        << QStringLiteral("xrandr-Sharp Corporation")
        << QString() // unsure why, this screen reports no name
        << QStringLiteral("SHP")
        << QStringList({QStringLiteral("SHP"), QStringLiteral("Sharp Corporation")})
        << QStringLiteral("")
        << QStringLiteral("RXN49-LQ133Z1")
        << QStringLiteral("3627c3534e4c82871967b57237bf5b83")
        << 29U << 17U << 2.2
        << QQuaternion(1, QVector3D(0.639648, 0.328125, 0))
        << QQuaternion(1, QVector3D(0.299805, 0.599609, 0))
        << QQuaternion(1, QVector3D(0.149414, 0.0595703, 0))
        << QQuaternion(1, QVector3D(0.3125, 0.328125, 0));
    // clang-format on
}

void TestEdid::testEdidParser()
{
    QFETCH(QByteArray, raw_edid);
    QFETCH(QString, deviceId);
    QFETCH(QString, name);
    QFETCH(QString, pnpId);
    QFETCH(QStringList, vendor);
    QFETCH(QString, serial);
    QFETCH(QString, eisaId);
    QFETCH(QString, hash);
    QFETCH(uint, width);
    QFETCH(uint, height);
    QFETCH(qreal, gamma);
    QFETCH(QQuaternion, red);
    QFETCH(QQuaternion, green);
    QFETCH(QQuaternion, blue);
    QFETCH(QQuaternion, white);

    QScopedPointer<Edid> e(new Edid(raw_edid));
    QCOMPARE(e->isValid(), true);

    // FIXME: we hard-code all deviceIds as xrandr-something, that makes no sense
    QCOMPARE(e->deviceId(), deviceId);
    QCOMPARE(e->name(), name);
    QCOMPARE(e->pnpId(), pnpId);
    // FIXME: needs to return at least the short ID
    //    QVERIFY2(vendor.contains(e->vendor()), qPrintable(QString::fromLatin1("%1 not in list").arg(e->vendor())));
    QCOMPARE(e->serial(), serial);
    QCOMPARE(e->eisaId(), eisaId);
    QCOMPARE(e->hash(), hash);
    QCOMPARE(e->width(), width);
    QCOMPARE(e->height(), height);
    QCOMPARE(e->gamma(), gamma);

    QVERIFY(qFuzzyCompare(e->red(), red));
    QVERIFY(qFuzzyCompare(e->green(), green));
    QVERIFY(qFuzzyCompare(e->blue(), blue));
    QVERIFY(qFuzzyCompare(e->white(), white));
}

QTEST_GUILESS_MAIN(TestEdid)

#include "testedid.moc"
