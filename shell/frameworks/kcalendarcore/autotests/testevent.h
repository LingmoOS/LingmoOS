/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2006-2008 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTEVENT_H
#define TESTEVENT_H

#include <QObject>

class EventTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testSetRoles_data();
    void testSetRoles();
    void testValidity();
    void testCompare();
    void testDtEndEqual();
    void testCompareAlarms();
    void testClone();
    void testCopyConstructor();
    void testCopyIncidence();
    void testAssign();
    void testSerializer_data();
    void testSerializer();
    void testDurationDtEnd();
    void testDtStartChange();
    void testDtEndChange();
    void testIsMultiDay_data();
    void testIsMultiDay();
};

#endif
