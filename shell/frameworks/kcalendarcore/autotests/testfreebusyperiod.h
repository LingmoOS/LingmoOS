/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTFREEBUSYPERIOD_H
#define TESTFREEBUSYPERIOD_H

#include <QObject>

class FreeBusyPeriodTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testAssign();
    void testCopyConstructor();
    void testDataStreamOut();
    void testDataStreamIn();
};

#endif
