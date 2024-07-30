/*
 *   SPDX-FileCopyrightText: 2009 Petri Damstén <damu@iki.fi>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "valuetest.h"

#include <QStandardPaths>

using namespace KUnitConversion;

void ValueTest::initTestCase()
{
    qputenv("KF5UNITCONVERT_NO_DOWNLOAD", "1");
    QStandardPaths::setTestModeEnabled(true);
    QLocale::setDefault(QLocale::c());

    v1 = Value(3.1415, Kilometer);
    v2 = Value(6.1415, QStringLiteral("m"));
    v3 = Value(9.1415, v1.unit());
}

void ValueTest::testStrings()
{
    QCOMPARE(v1.unit().symbol(), QStringLiteral("km"));
    QCOMPARE(v2.toSymbolString(), QStringLiteral("6.1415 m"));
    QCOMPARE(v3.toString(), QStringLiteral("9.1415 kilometers"));
}

void ValueTest::testRound()
{
    v1.round(2);
    QCOMPARE(v1.number(), 3.14);
}

void ValueTest::testConvert()
{
    v1 = v1.convertTo(Meter);
    QCOMPARE(v1.number(), 3140.0);
    v1 = v1.convertTo(QStringLiteral("cm"));
    QCOMPARE(v1.number(), 314000.0);
    v1 = v1.convertTo(v2.unit());
    QCOMPARE(v1.number(), 3140.0);
}

void ValueTest::testInvalid()
{
    v1 = v1.convertTo(UnitId(99999));
    QCOMPARE(v1.number(), 0.0);
    QCOMPARE(v1.toSymbolString(), QLatin1String(""));
    v2 = v2.convertTo(QStringLiteral("don't exist"));
    QCOMPARE(v2.number(), 0.0);
    QCOMPARE(v2.toSymbolString(), QLatin1String(""));
}

void ValueTest::testCurrencyNotDownloaded()
{
    // ensure that no local conversion table is available
    const QString cache = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/libkunitconversion/currency.xml");
    if (QFile::exists(cache)) {
        QFile::remove(cache);
    }

    auto pounds = Value(100, Gbp);
    auto eur = pounds.convertTo(Eur);
    QVERIFY(!eur.isValid());
}

QTEST_MAIN(ValueTest)

#include "moc_valuetest.cpp"
