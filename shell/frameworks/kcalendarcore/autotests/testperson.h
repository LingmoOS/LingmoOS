/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2006-2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTPERSON_H
#define TESTPERSON_H

#include <QObject>

class PersonTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testCompare();
    void testStringify();
    void testDataStreamIn();
    void testDataStreamOut();
};

#endif
