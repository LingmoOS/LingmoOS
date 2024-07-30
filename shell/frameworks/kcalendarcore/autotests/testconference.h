/*
  This file is part of the kcalcore library.
  SPDX-FileCopyrightText: 2020 Daniel Vrátil <dvratil@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTCONFERENCE_H
#define TESTCONFERENCE_H

#include <QObject>

class ConferenceTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testCompare();
    void testCopyConstructor();
    void testAssign();
    void testDataStream();
    void testLoading();
};

#endif
