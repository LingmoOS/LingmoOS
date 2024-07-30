/*
 * SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include <KRunner/AbstractRunnerTest>
#include <KUnitConversion/Converter>
#include <KUnitConversion/UnitCategory>
#include <QRegularExpression>
#include <QTest>

using namespace KUnitConversion;
using namespace KRunner;

class ConverterRunnerTest : public AbstractRunnerTest
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testMostCommonUnits();
    void testCurrency();
    void testLettersAndCurrency();
    void testFractionsWithoutSpecifiedTarget();
    void testQuery_data();
    void testQuery();
    void testInvalidQuery_data();
    void testInvalidQuery();
    void testRoundingOfCurrencies();
};

void ConverterRunnerTest::initTestCase()
{
    initProperties();
}

/**
 * Test if the most common units are displayed
 */
void ConverterRunnerTest::testMostCommonUnits()
{
    launchQuery(QStringLiteral("1m"));

    Converter converter;
    const auto lengthCategory = converter.category(KUnitConversion::LengthCategory);
    QCOMPARE(manager->matches().count(), lengthCategory.mostCommonUnits().count() - 1);
}

/**
 * Test of a currency gets converted to the most common currencies
 */
void ConverterRunnerTest::testCurrency()
{
    launchQuery(QStringLiteral("1$"));

    Converter converter;
    const auto currencyCategory = converter.category(KUnitConversion::CurrencyCategory);
    QList<Unit> currencyUnits = currencyCategory.mostCommonUnits();

    const QString currencyIsoCode = QLocale().currencySymbol(QLocale::CurrencyIsoCode);
    const KUnitConversion::Unit localCurrency = currencyCategory.unit(currencyIsoCode);
    if (localCurrency.isValid() && !currencyUnits.contains(localCurrency)) {
        currencyUnits << localCurrency;
    }
    QCOMPARE(manager->matches().count(), currencyUnits.count() - 1);
}

/**
 * Test a combination of currency symbols and letters that is not directly supported by the conversion backend
 */
void ConverterRunnerTest::testLettersAndCurrency()
{
    launchQuery(QStringLiteral("4us$>ca$"));

    QCOMPARE(manager->matches().count(), 1);
    QVERIFY2(manager->matches().constFirst().text().contains(QLatin1String("Canadian dollars (CAD)")), qUtf8Printable(manager->matches().constFirst().text()));
}

/**
 * Test if fractions with source unit, but without target unit get parsed
 */
void ConverterRunnerTest::testFractionsWithoutSpecifiedTarget()
{
    launchQuery(QStringLiteral("6/3 m"));

    Converter converter;
    const auto lengthCategory = converter.category(KUnitConversion::LengthCategory);
    QCOMPARE(manager->matches().count(), lengthCategory.mostCommonUnits().count() - 1);
}

void ConverterRunnerTest::testQuery_data()
{
    QTest::addColumn<QString>("query");
    QTest::addColumn<QString>("expectedText");

    QTest::newRow("test specific target unit") << QStringLiteral("1m > cm") << QStringLiteral("100 centimeters (cm)");
    QTest::newRow("test symbols (other than currencies)") << QStringLiteral("1000 µs as year") << QStringLiteral("3.17098E-11 year (y)");
    QTest::newRow("test negative value") << QStringLiteral("-4m as cm") << QStringLiteral("-400 centimeters (cm)");
    QTest::newRow("test fractions") << QStringLiteral("6/3m>cm") << QStringLiteral("200 centimeters (cm)");
    QTest::newRow("test case insensitive units") << QStringLiteral("1Liter in ML") << QStringLiteral("1,000 milliliters (ml)");
    // megaseconds (Ms) and milliseconds (ms)
    QTest::newRow("test case sensitive units") << QStringLiteral("1Ms as ms") << QStringLiteral("1,000,000,000 milliseconds (ms)");
    QTest::newRow("test case sensitive units") << QStringLiteral("1,000,000,000milliseconds>Ms") << QStringLiteral("1 megasecond (Ms)");
}

void ConverterRunnerTest::testQuery()
{
    QFETCH(QString, query);
    QFETCH(QString, expectedText);

    launchQuery(query);
    const QList<QueryMatch> matches = manager->matches();
    QCOMPARE(matches.count(), 1);
    QCOMPARE(matches.first().text(), expectedText);
}

void ConverterRunnerTest::testInvalidQuery_data()
{
    QTest::addColumn<QString>("query");

    QTest::newRow("test invalid fraction without unit") << QStringLiteral("1/2");
    QTest::newRow("test invalid fraction without unit but valid target unit") << QStringLiteral("4/4>cm");
    QTest::newRow("test invalid currency") << QStringLiteral("4us$>abc$");
}

void ConverterRunnerTest::testInvalidQuery()
{
    QFETCH(QString, query);
    launchQuery(query);
    QCOMPARE(manager->matches().count(), 0);
}

void ConverterRunnerTest::testRoundingOfCurrencies()
{
    launchQuery(QStringLiteral("40000000000000000000000000000000000000000000000000$"));
    QVERIFY(!manager->matches().isEmpty());
    QVERIFY(!manager->matches().constFirst().text().startsWith("-"));
    launchQuery(QStringLiteral("50.123$"));
    QVERIFY(!manager->matches().isEmpty());
    QRegularExpression hasTwoDecimalPrescision(QStringLiteral(R"(^\d+\.\d\d)"));
    QVERIFY2(manager->matches().constFirst().text().contains(hasTwoDecimalPrescision), qUtf8Printable(manager->matches().constFirst().text()));
}

QTEST_MAIN(ConverterRunnerTest)

#include "converterrunnertest.moc"
