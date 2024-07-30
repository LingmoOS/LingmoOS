/*
  This file is part of the kcalcore library.
  SPDX-FileCopyrightText: 2006, 2008 Allen Winter <winter@kde.org>
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTATTENDEE_H
#define TESTATTENDEE_H

#include <QObject>

class AttendeeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testType();
    void testCompare();
    void testCompareType();
    void testAssign();
    void testCopyConstructor();
    void testDataStreamOut();
    void testDataStreamIn();
    void testUid();
};

#endif
