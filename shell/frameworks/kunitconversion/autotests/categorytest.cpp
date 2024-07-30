/*
 *   SPDX-FileCopyrightText: 2009 Petri Damstén <damu@iki.fi>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "categorytest.h"
#include <QStandardPaths>
#include <chrono>
#include <currency_p.h>
#include <kunitconversion/unitcategory.h>
#include <unitcategory_p.h>
#include <QSignalSpy>

using namespace KUnitConversion;
using namespace std::chrono_literals;

void CategoryTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void CategoryTest::testInfo()
{
    UnitCategory cg = c.category(AreaCategory);
    QCOMPARE(cg.name(), QStringLiteral("Area"));
    QCOMPARE(cg.description(), QStringLiteral("Area"));
    QCOMPARE(cg.id(), AreaCategory);
}

void CategoryTest::testUnits()
{
    UnitCategory cg = c.category(MassCategory);
    QCOMPARE(cg.defaultUnit().symbol(), QStringLiteral("kg"));
    QCOMPARE(cg.hasUnit(QStringLiteral("g")), true);
    QCOMPARE(cg.unit(QStringLiteral("g")).symbol(), QStringLiteral("g"));
    QCOMPARE(cg.unit(Kilogram).symbol(), QStringLiteral("kg"));
    QVERIFY(cg.units().size() > 0);
    QVERIFY(cg.allUnits().size() > 0);
}

void CategoryTest::testConvert()
{
    UnitCategory cg = c.category(LengthCategory);
    Value v = cg.convert(Value(3.14, Kilometer), QStringLiteral("m"));
    QCOMPARE(v.number(), 3140.0);
    v = cg.convert(v, QStringLiteral("cm"));
    QCOMPARE(v.number(), 314000.0);
    v = cg.convert(v, cg.defaultUnit());
    QCOMPARE(v.number(), 3140.0);
}

void CategoryTest::testInvalid()
{
    UnitCategory cg = c.category(CategoryId(99999));
    QCOMPARE(cg.name(), QString());
    cg = c.category(QStringLiteral("don't exist"));
    QCOMPARE(cg.name(), QString());
}

void CategoryTest::testCurrencyTableUpdate()
{
    // Remove currency cache to force a download
    const QString cache = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/libkunitconversion/currency.xml");
    QFile::remove(cache);
    QVERIFY(Currency::lastConversionTableUpdate().isNull());

    UnitCategory category = c.category(CurrencyCategory);
    QCOMPARE(category.hasOnlineConversionTable(), true);
    {
        auto job = category.syncConversionTable(std::chrono::seconds::zero()); // no skip period = force update
        QSignalSpy finishedSpy(job, &UpdateJob::finished);
        finishedSpy.wait();

        QVERIFY(!Currency::lastConversionTableUpdate().isNull());
        QDateTime lastUpdate = Currency::lastConversionTableUpdate();
        QVERIFY(lastUpdate.secsTo(QDateTime::currentDateTime()) < std::chrono::seconds(1h).count());

        QCOMPARE(category.syncConversionTable(1h), nullptr);
        QVERIFY(!Currency::lastConversionTableUpdate().isNull());
        QDateTime newUpdate = Currency::lastConversionTableUpdate();
        QCOMPARE(newUpdate, lastUpdate);
    }
}

QTEST_MAIN(CategoryTest)

#include "moc_categorytest.cpp"
