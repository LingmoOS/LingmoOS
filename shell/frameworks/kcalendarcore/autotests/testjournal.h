/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2007-2008 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTJOURNAL_H
#define TESTJOURNAL_H

#include <QObject>

class JournalTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testCompare();
    void testClone();
    void testRich();
    void testCopyConstructor();
    void testAssign();
    void testSerializer_data();
    void testSerializer();
};

#endif
