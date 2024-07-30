/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2006 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTALARM_H
#define TESTALARM_H

#include <QObject>

class AlarmTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testCompare();
    void testAssignment();
    void testCopyConstructor();
    void testSerializer_data();
    void testSerializer();
};

#endif
