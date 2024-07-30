/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2004, 2006-2007 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testseasons.h"
#include "astroseasons.h"
using namespace KHolidays;

#include <QTest>

QTEST_MAIN(SeasonsTest)

Q_DECLARE_METATYPE(AstroSeasons::Season)

void SeasonsTest::test_data()
{
    QTest::addColumn<QDate>("date");
    QTest::addColumn<AstroSeasons::Season>("season");

    // Test data obtained from https://data.giss.nasa.gov/ar5/srvernal.html
    // clang-format off
    QTest::newRow("2005-03-20") << QDate(2005,  3, 20) << AstroSeasons::MarchEquinox;
    QTest::newRow("2005-06-21") << QDate(2005,  6, 21) << AstroSeasons::JuneSolstice;
    QTest::newRow("2005-09-22") << QDate(2005,  9, 22) << AstroSeasons::SeptemberEquinox;
    QTest::newRow("2005-12-21") << QDate(2005, 12, 21) << AstroSeasons::DecemberSolstice;
    QTest::newRow("2005-01-22") << QDate(2005,  1, 22) << AstroSeasons::None;
    QTest::newRow("2005-12-31") << QDate(2005, 12, 31) << AstroSeasons::None;
    QTest::newRow("2007-03-21") << QDate(2007,  3, 21) << AstroSeasons::MarchEquinox;
    QTest::newRow("2007-06-21") << QDate(2007,  6, 21) << AstroSeasons::JuneSolstice;
    QTest::newRow("2007-09-23") << QDate(2007,  9, 23) << AstroSeasons::SeptemberEquinox;
    QTest::newRow("2007-12-22") << QDate(2007, 12, 22) << AstroSeasons::DecemberSolstice;
    QTest::newRow("2018-03-20") << QDate(2018,  3, 20) << AstroSeasons::MarchEquinox;
    QTest::newRow("2018-06-21") << QDate(2018,  6, 21) << AstroSeasons::JuneSolstice;
    QTest::newRow("2018-09-23") << QDate(2018,  9, 23) << AstroSeasons::SeptemberEquinox;
    QTest::newRow("2018-12-21") << QDate(2018, 12, 21) << AstroSeasons::DecemberSolstice;
    QTest::newRow("2020-03-20") << QDate(2020,  3, 20) << AstroSeasons::MarchEquinox;
    QTest::newRow("2020-06-20") << QDate(2020,  6, 20) << AstroSeasons::JuneSolstice;
    QTest::newRow("2020-09-22") << QDate(2020,  9, 22) << AstroSeasons::SeptemberEquinox;
    QTest::newRow("2020-12-21") << QDate(2020, 12, 21) << AstroSeasons::DecemberSolstice;
    QTest::newRow("2042-03-20") << QDate(2042,  3, 20) << AstroSeasons::MarchEquinox;
    QTest::newRow("2042-06-21") << QDate(2042,  6, 21) << AstroSeasons::JuneSolstice;
    QTest::newRow("2042-09-22") << QDate(2042,  9, 22) << AstroSeasons::SeptemberEquinox;
    QTest::newRow("2042-12-21") << QDate(2042, 12, 21) << AstroSeasons::DecemberSolstice;
    // clang-format on
}

void SeasonsTest::test()
{
    QFETCH(QDate, date);
    QFETCH(AstroSeasons::Season, season);

    AstroSeasons as;
    QCOMPARE(as.seasonAtDate(date), season);
}

#include "moc_testseasons.cpp"
