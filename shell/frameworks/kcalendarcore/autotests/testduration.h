/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2006 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTDURATION_H
#define TESTDURATION_H

#include <QObject>

class DurationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testCompare();
    void testSerializer_data();
    void testSerializer();
    void testIsNull();
};

#endif
