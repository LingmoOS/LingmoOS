/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2006 Narayan Newton <narayannewton@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTPERIOD_H
#define TESTPERIOD_H

#include <QObject>

class PeriodTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testCompare();
    void testDataStreamOut();
    void testDataStreamIn();
};

#endif
