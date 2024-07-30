/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2006 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testcalfilter.h"
#include "calfilter.h"

#include <QTest>
QTEST_MAIN(CalFilterTest)

using namespace KCalendarCore;

void CalFilterTest::testValidity()
{
    CalFilter f;
    f.setName(QStringLiteral("testfilter"));
    QVERIFY(f.name() == QLatin1String("testfilter"));
    CalFilter g(QStringLiteral("fredfilter"));
    QVERIFY(g.name() == QLatin1String("fredfilter"));
    CalFilter f1;
    CalFilter f2;
    QVERIFY(f1 == f2);
}

void CalFilterTest::testCats()
{
    CalFilter f1;
    CalFilter f2;
    QStringList cats;
    cats << QStringLiteral("a") << QStringLiteral("b") << QStringLiteral("c");
    f1.setCategoryList(cats);
    f2.setCategoryList(cats);
    QVERIFY(f1.categoryList() == f2.categoryList());
}

#include "moc_testcalfilter.cpp"
