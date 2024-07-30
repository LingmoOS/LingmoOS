/*
    This file is part of the kholidays library.

    SPDX-FileCopyrightText: 2012 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTSUNRISE_H
#define TESTSUNRISE_H

#include <QObject>

class SunriseTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void TestSunrise();
    void TestSunset();
    void TestDawn();
    void TestDusk();
    void TestPolarDayNight();
};

#endif
