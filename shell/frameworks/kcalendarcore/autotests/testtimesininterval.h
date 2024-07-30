/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-FileContributor: Sergio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTTIMESININTERVAL_H
#define TESTTIMESININTERVAL_H

#include <QObject>

class TimesInIntervalTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
    void testSubDailyRecurrenceIntervalInclusive();
    void testSubDailyRecurrence2();
    void testSubDailyRecurrenceIntervalLimits();
    void testLocalTimeHandlingNonAllDay();
    void testLocalTimeHandlingAllDay();
    void testByDayRecurrence();
    void testRDatePeriod();
};

#endif
