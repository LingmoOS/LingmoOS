/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2006 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTEXCEPTION_H
#define TESTEXCEPTION_H

#include <QObject>

class ExceptionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
};

#endif
