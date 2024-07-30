/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2015 Sandro Knauß <sknauss@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTCALENDAROBSERVER_H
#define TESTCALENDAROBSERVER_H

#include <QObject>

class CalendarObserverTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testAdd();
    void testChange();
    void testDelete();
};

#endif
