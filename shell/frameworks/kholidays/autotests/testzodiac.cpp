/*
    This file is part of kholidays library.

    SPDX-FileCopyrightText: 2005, 2006 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "zodiac.h"

#include <QCoreApplication>
#include <QDate>

#include <cstdlib>
#include <iostream>

using namespace KHolidays;
using namespace std;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    Zodiac::ZodiacSigns sign;
    QString pStr;

    int year = 2004;
    int days = (QDate::isLeapYear(year) ? 366 : 355);

    Zodiac zd(Zodiac::Tropical);
    QDate qd(year, 1, 1);
    for (int i = 1; i <= days; ++i) {
        sign = zd.signAtDate(qd);
        pStr = zd.signName(sign);
        if (!pStr.isNull()) {
            cout << qd.toString(Qt::TextDate).toLocal8Bit().constData() << ": " //
                 << pStr.toLocal8Bit().constData() //
                 << " [" << qPrintable(zd.signSymbol(sign)) << "]" //
                 << endl;
        }
        qd = qd.addDays(1);
    }

    Zodiac zd2(Zodiac::Sidereal);
    QDate qd2(year, 1, 1);
    for (int i = 1; i <= days; ++i) {
        sign = zd2.signAtDate(qd2);
        pStr = zd2.signName(sign);
        if (!pStr.isNull()) {
            cout << qd2.toString(Qt::TextDate).toLocal8Bit().constData() << ": " //
                 << pStr.toLocal8Bit().constData() //
                 << " [" << qPrintable(zd2.signSymbol(sign)) << "]" //
                 << endl;
        }
        qd2 = qd2.addDays(1);
    }
}
