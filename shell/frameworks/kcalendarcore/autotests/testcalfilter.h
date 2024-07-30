/*
  This file is part of the kcalcore library.
  SPDX-FileCopyrightText: 2006 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTCALFILTER_H
#define TESTCALFILTER_H

#include <QObject>

class CalFilterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testCats();
};

#endif
