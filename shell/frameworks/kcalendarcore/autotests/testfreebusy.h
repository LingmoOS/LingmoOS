/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2007 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTFREEBUSY_H
#define TESTFREEBUSY_H

#include <QObject>

class FreeBusyTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testAddSort();
    void testAssign();
    void testCopyConstructor();
    void testDataStream();
};

#endif
