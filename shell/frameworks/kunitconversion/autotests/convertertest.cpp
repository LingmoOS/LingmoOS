/*
 *   SPDX-FileCopyrightText: 2009 Petri Damstén <damu@iki.fi>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "convertertest.h"
#include <QList>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QThread>
#include <currency_p.h>
#include <kunitconversion/unitcategory.h>

using namespace KUnitConversion;
using namespace std::chrono_literals;

void ConverterTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    // Remove currency cache to force a download
    const QString cache = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/libkunitconversion/currency.xml");
    QFile::remove(cache);
}

void ConverterTest::testCategory()
{
    Converter c;
    QCOMPARE(c.categoryForUnit(QStringLiteral("km")).id(), LengthCategory);
    QCOMPARE(c.category(QStringLiteral("Length")).id(), LengthCategory);
    QCOMPARE(c.category(LengthCategory).name(), QStringLiteral("Length"));
    QVERIFY(c.categories().size() > 0);
}

void ConverterTest::testUnits()
{
    Converter c;
    QCOMPARE(c.unit(QStringLiteral("km")).symbol(), QStringLiteral("km"));
    QCOMPARE(c.unit(Kilogram).symbol(), QStringLiteral("kg"));
}

void ConverterTest::testConvert()
{
    Converter c;
    Value v = c.convert(Value(3.14, Kilometer), QStringLiteral("m"));
    QCOMPARE(v.number(), 3140.0);
    v = c.convert(v, QStringLiteral("cm"));
    QCOMPARE(v.number(), 314000.0);
    v = c.convert(v, c.category(LengthCategory).defaultUnit());
    QCOMPARE(v.number(), 3140.0);

    v = Value(8.0, LitersPer100Kilometers);
    v = c.convert(v, QStringLiteral("mpg"));
    QCOMPARE(v.number(), 29.401875);
    v = c.convert(v, QStringLiteral("mpg (imperial)"));
    QCOMPARE(v.number(), 35.310125);
    v = c.convert(v, QStringLiteral("kmpl"));
    QCOMPARE(v.number(), 12.5);
    v = c.convert(v, QStringLiteral("l/100 km"));
    QCOMPARE(v.number(), 8.0);

    v = Value(33.0, MilePerUsGallon);
    v = c.convert(v, QStringLiteral("mpg (imperial)"));
    QCOMPARE(v.number(), 39.63128627);
    v = c.convert(v, QStringLiteral("kmpl"));
    QCOMPARE(v.number(), 14.0297174925);
    v = c.convert(v, QStringLiteral("l/100 km"));
    QCOMPARE(v.number(), 7.12772727273);
    v = c.convert(v, QStringLiteral("mpg"));
    QCOMPARE(v.number(), 33.0);
}

void ConverterTest::testInvalid()
{
    Converter c;
    QCOMPARE(c.categoryForUnit(QStringLiteral("does not exist")).id(), InvalidCategory);
    QCOMPARE(c.unit(QStringLiteral("does not exist")).symbol(), QString());
    QCOMPARE(c.category(QStringLiteral("does not exist")).name(), QString());
}

class CurrencyTestThread : public QThread
{
public:
    CurrencyTestThread(Converter &c)
        : m_c(c)
    {
    }
    void run() override
    {
        Value input = Value(1000, Eur);
        Value v = m_c.convert(input, QStringLiteral("$"));
        number = v.number();
    }
    int number;
    Converter &m_c;
};

void ConverterTest::testCurrency()
{
    Converter c;
    auto job = c.category(CurrencyCategory).syncConversionTable();
    QSignalSpy finishedSpy(job, &UpdateJob::finished);
    finishedSpy.wait();

    // 2 threads is enough for tsan to notice races, let's not hammer the website with more concurrent requests
    const int numThreads = 2;
    QList<CurrencyTestThread *> threads;
    threads.resize(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        threads[i] = new CurrencyTestThread(c);
    }
    for (int i = 0; i < numThreads; ++i) {
        threads[i]->start();
    }
    for (int i = 0; i < numThreads; ++i) {
        threads[i]->wait();
        QVERIFY(threads.at(i)->number > 100);
    }
    qDeleteAll(threads);
}

QTEST_MAIN(ConverterTest)

#include "moc_convertertest.cpp"
