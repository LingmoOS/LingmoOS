/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2006 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testexception.h"
#include "exceptions.h"

#include <QTest>
QTEST_MAIN(ExceptionTest)

using namespace KCalendarCore;

void ExceptionTest::testValidity()
{
    Exception ef(Exception::LoadError);
    QVERIFY(ef.code() == Exception::LoadError);
}

#include "moc_testexception.cpp"
