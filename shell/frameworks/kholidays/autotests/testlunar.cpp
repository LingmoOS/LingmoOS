/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2004, 2007, 2009 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testlunar.h"

#include <QDebug>
#include <QTest>

QTEST_MAIN(LunarTest)

#include "kholidays/lunarphase.h"
using namespace KHolidays;

void LunarTest::test2005()
{
    QList<QDate> fQ2005;
    QList<QDate> fM2005;
    QList<QDate> lQ2005;
    QList<QDate> nM2005;
    // 2005 first quarter moons
    fQ2005.append(QDate(2005, 1, 17));
    fQ2005.append(QDate(2005, 2, 16));
    fQ2005.append(QDate(2005, 3, 17));
    fQ2005.append(QDate(2005, 4, 16));
    fQ2005.append(QDate(2005, 5, 16));
    fQ2005.append(QDate(2005, 6, 15));
    fQ2005.append(QDate(2005, 7, 14));
    fQ2005.append(QDate(2005, 8, 13));
    fQ2005.append(QDate(2005, 9, 11));
    fQ2005.append(QDate(2005, 10, 10));
    fQ2005.append(QDate(2005, 11, 9));
    fQ2005.append(QDate(2005, 12, 8));

    // 2005 full moons
    fM2005.append(QDate(2005, 1, 25));
    fM2005.append(QDate(2005, 2, 24));
    fM2005.append(QDate(2005, 3, 25));
    fM2005.append(QDate(2005, 4, 24));
    fM2005.append(QDate(2005, 5, 23));
    fM2005.append(QDate(2005, 6, 22));
    fM2005.append(QDate(2005, 7, 21));
    fM2005.append(QDate(2005, 8, 19));
    fM2005.append(QDate(2005, 9, 18));
    fM2005.append(QDate(2005, 10, 17));
    fM2005.append(QDate(2005, 11, 16));
    fM2005.append(QDate(2005, 12, 15));

    // 2005 last quarter moons
    lQ2005.append(QDate(2005, 1, 3));
    lQ2005.append(QDate(2005, 2, 2));
    lQ2005.append(QDate(2005, 3, 3));
    lQ2005.append(QDate(2005, 4, 2));
    lQ2005.append(QDate(2005, 5, 1));
    lQ2005.append(QDate(2005, 5, 30));
    lQ2005.append(QDate(2005, 6, 28));
    lQ2005.append(QDate(2005, 7, 28));
    lQ2005.append(QDate(2005, 8, 26));
    lQ2005.append(QDate(2005, 9, 25));
    lQ2005.append(QDate(2005, 10, 25));
    lQ2005.append(QDate(2005, 11, 23));
    lQ2005.append(QDate(2005, 12, 23));

    // 2005 new moons
    nM2005.append(QDate(2005, 1, 10));
    nM2005.append(QDate(2005, 2, 8));
    nM2005.append(QDate(2005, 3, 10));
    nM2005.append(QDate(2005, 4, 8));
    nM2005.append(QDate(2005, 5, 8));
    nM2005.append(QDate(2005, 6, 6));
    nM2005.append(QDate(2005, 7, 6));
    nM2005.append(QDate(2005, 8, 5));
    nM2005.append(QDate(2005, 9, 3));
    nM2005.append(QDate(2005, 10, 3));
    nM2005.append(QDate(2005, 11, 2));
    nM2005.append(QDate(2005, 12, 1));
    nM2005.append(QDate(2005, 12, 31));

    LunarPhase lp;
    QDate qd(2004, 12, 31);
    QList<QDate> firstQuarter;
    QList<QDate> fullMoon;
    QList<QDate> lastQuarter;
    QList<QDate> newMoon;
    for (int i = 1; i <= 365; ++i) {
        qd = qd.addDays(1);
        LunarPhase::Phase phase = lp.phaseAtDate(qd);
        if (phase == LunarPhase::FirstQuarter) {
            firstQuarter.append(qd);
            qDebug() << "first quarter" << qd;
        } else if (phase == LunarPhase::FullMoon) {
            fullMoon.append(qd);
            qDebug() << "full moon" << qd;
        } else if (phase == LunarPhase::LastQuarter) {
            lastQuarter.append(qd);
            qDebug() << "last quarter" << qd;
        } else if (phase == LunarPhase::NewMoon) {
            newMoon.append(qd);
            qDebug() << "new moon" << qd;
        }
    }
    QVERIFY(fQ2005 == firstQuarter);
    QVERIFY(fM2005 == fullMoon);
    QVERIFY(lQ2005 == lastQuarter);
    QVERIFY(nM2005 == newMoon);
}

void LunarTest::test2007()
{
    QList<QDate> fQ2007;
    QList<QDate> fM2007;
    QList<QDate> lQ2007;
    QList<QDate> nM2007;
    // 2007 first quarter moons
    fQ2007.append(QDate(2007, 1, 25));
    fQ2007.append(QDate(2007, 2, 24));
    fQ2007.append(QDate(2007, 3, 25));
    fQ2007.append(QDate(2007, 4, 24));
    fQ2007.append(QDate(2007, 5, 23));
    fQ2007.append(QDate(2007, 6, 22));
    fQ2007.append(QDate(2007, 7, 22));
    fQ2007.append(QDate(2007, 8, 20));
    fQ2007.append(QDate(2007, 9, 19));
    fQ2007.append(QDate(2007, 10, 19));
    fQ2007.append(QDate(2007, 11, 17));
    fQ2007.append(QDate(2007, 12, 17));

    // 2007 full moons
    fM2007.append(QDate(2007, 1, 3));
    fM2007.append(QDate(2007, 2, 2));
    fM2007.append(QDate(2007, 3, 3));
    fM2007.append(QDate(2007, 4, 2));
    fM2007.append(QDate(2007, 5, 2));
    fM2007.append(QDate(2007, 6, 1));
    fM2007.append(QDate(2007, 6, 30));
    fM2007.append(QDate(2007, 7, 30));
    fM2007.append(QDate(2007, 8, 28));
    fM2007.append(QDate(2007, 9, 26));
    fM2007.append(QDate(2007, 10, 26));
    fM2007.append(QDate(2007, 11, 24));
    fM2007.append(QDate(2007, 12, 24));

    // 2007 last quarter moons
    lQ2007.append(QDate(2007, 1, 11));
    lQ2007.append(QDate(2007, 2, 10));
    lQ2007.append(QDate(2007, 3, 12));
    lQ2007.append(QDate(2007, 4, 10));
    lQ2007.append(QDate(2007, 5, 10));
    lQ2007.append(QDate(2007, 6, 8));
    lQ2007.append(QDate(2007, 7, 7));
    lQ2007.append(QDate(2007, 8, 5));
    lQ2007.append(QDate(2007, 9, 4));
    lQ2007.append(QDate(2007, 10, 3));
    lQ2007.append(QDate(2007, 11, 1));
    lQ2007.append(QDate(2007, 12, 1));
    lQ2007.append(QDate(2007, 12, 31));

    // 2007 new moons
    nM2007.append(QDate(2007, 1, 19));
    nM2007.append(QDate(2007, 2, 17));
    nM2007.append(QDate(2007, 3, 19));
    nM2007.append(QDate(2007, 4, 17));
    nM2007.append(QDate(2007, 5, 16));
    nM2007.append(QDate(2007, 6, 15));
    nM2007.append(QDate(2007, 7, 14));
    nM2007.append(QDate(2007, 8, 12));
    nM2007.append(QDate(2007, 9, 11));
    nM2007.append(QDate(2007, 10, 11));
    nM2007.append(QDate(2007, 11, 9));
    nM2007.append(QDate(2007, 12, 9));

    LunarPhase lp;
    QDate qd(2006, 12, 31);
    QList<QDate> firstQuarter;
    QList<QDate> fullMoon;
    QList<QDate> lastQuarter;
    QList<QDate> newMoon;
    for (int i = 1; i <= 365; ++i) {
        qd = qd.addDays(1);
        LunarPhase::Phase phase = lp.phaseAtDate(qd);
        if (phase == LunarPhase::FirstQuarter) {
            firstQuarter.append(qd);
            qDebug() << "first quarter" << qd;
        } else if (phase == LunarPhase::FullMoon) {
            fullMoon.append(qd);
            qDebug() << "full moon" << qd;
        } else if (phase == LunarPhase::LastQuarter) {
            lastQuarter.append(qd);
            qDebug() << "last quarter" << qd;
        } else if (phase == LunarPhase::NewMoon) {
            newMoon.append(qd);
            qDebug() << "new moon" << qd;
        }
    }
    QVERIFY(fQ2007 == firstQuarter);
    QVERIFY(fM2007 == fullMoon);
    QVERIFY(lQ2007 == lastQuarter);
    QVERIFY(nM2007 == newMoon);
}

void LunarTest::testIntermediatePhases()
{
    struct {
        QDate date;
        LunarPhase::Phase phase;
    } const phase_changes[] = {
        {{2022, 1, 2}, LunarPhase::NewMoon},
        {{2022, 1, 3}, LunarPhase::WaxingCrescent},
        {{2022, 1, 9}, LunarPhase::FirstQuarter},
        {{2022, 1, 10}, LunarPhase::WaxingGibbous},
        {{2022, 1, 17}, LunarPhase::FullMoon},
        {{2022, 1, 18}, LunarPhase::WaningGibbous},
        {{2022, 1, 25}, LunarPhase::LastQuarter},
        {{2022, 1, 26}, LunarPhase::WaningCrescent},
        {{2022, 2, 1}, LunarPhase::NewMoon},
        {{2022, 2, 2}, LunarPhase::WaxingCrescent},
        {{2022, 2, 8}, LunarPhase::FirstQuarter},
        {{2022, 2, 9}, LunarPhase::WaxingGibbous},
        {{2022, 2, 16}, LunarPhase::FullMoon},
        {{2022, 2, 17}, LunarPhase::WaningGibbous},
        {{2022, 2, 23}, LunarPhase::LastQuarter},
        {{2022, 2, 24}, LunarPhase::WaningCrescent},
        {{2022, 3, 2}, LunarPhase::NewMoon},
    };

    QDate dt(2022, 1, 1);
    LunarPhase::Phase phase = LunarPhase::WaningCrescent;
    for (const auto &phaseChange : phase_changes) {
        for (auto i = phaseChange.date.daysTo(dt); i > 0; --i) {
            QCOMPARE(LunarPhase::phaseAtDate(phaseChange.date.addDays(i)), phase);
        }
        dt = phaseChange.date;
        phase = phaseChange.phase;
        QCOMPARE(LunarPhase::phaseAtDate(dt), phase);
        QVERIFY(!LunarPhase::phaseNameAtDate(dt).isEmpty());
    }
}

#include "moc_testlunar.cpp"
