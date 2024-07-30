/*
 *   SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "currencytableinittest.h"
#include <QStandardPaths>
#include <cmath>

using namespace KUnitConversion;

void CurrencyTableInitTest::testCategoryInit()
{
    QStandardPaths::setTestModeEnabled(true);
    const QString cache = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/libkunitconversion/currency.xml");

    QVERIFY(QFile::exists(QLatin1String(":/currency.xml")));
    if (QFile::exists(cache)) {
        QFile::remove(cache);
    }
    // note: copy of file updates the file's modified timestamp and thus file is seen as recently downloaded file
    QVERIFY(QFile::copy(QLatin1String(":/currency.xml"), cache));

    Converter c;
    Value input = Value(1000, Eur);
    Value v = c.convert(input, QStringLiteral("$"));
    qDebug() << "converted value to:" << v.number();
    QVERIFY(v.isValid());
    QVERIFY(!std::isnan(v.number()));
}

QTEST_MAIN(CurrencyTableInitTest)

#include "moc_currencytableinittest.cpp"
