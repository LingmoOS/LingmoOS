/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef SAFESTRINGTEST_H
#define SAFESTRINGTEST_H

#include <QObject>
#include <QTest>

#include "safestring.h"

using namespace KTextTemplate;

class TestSafeString : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCombining();
    void testAppending();
    void testChopping();
    void testReplacing();
};

void TestSafeString::testCombining()
{
    auto string1 = QStringLiteral("this & that");
    auto string2 = QStringLiteral(" over & under");
    SafeString safeString1unsafe(string1);
    SafeString safeString2unsafe(string2);
    SafeString safeString1safe(string1, SafeString::IsSafe);
    SafeString safeString2safe(string2, SafeString::IsSafe);

    SafeString combined;
    combined = safeString1safe + safeString2safe;
    QVERIFY(combined.isSafe());
    combined = safeString1safe + safeString1unsafe;
    QVERIFY(!combined.isSafe());
    combined = safeString1safe + string1;
    QVERIFY(!combined.isSafe());
    combined = safeString1unsafe + safeString1safe;
    QVERIFY(!combined.isSafe());
    combined = safeString1unsafe + safeString2unsafe;
    QVERIFY(!combined.isSafe());
    combined = safeString1unsafe + string1;
    QVERIFY(!combined.isSafe());
}

void TestSafeString::testAppending()
{
    auto string1 = QStringLiteral("this & that");
    SafeString safeString1unsafe(string1);
    SafeString safeString1safe(string1, SafeString::IsSafe);
    SafeString safeString2safe(string1, SafeString::IsSafe);

    SafeString result;
    result = safeString1safe.get().append(safeString1safe);
    QVERIFY(safeString1safe.isSafe());
    QVERIFY(result.isSafe());
    QCOMPARE(result.get(), safeString1safe.get());
    result = safeString1safe.get().append(string1);
    QVERIFY(!safeString1safe.isSafe());
    QVERIFY(!result.isSafe());
    QCOMPARE(result.get(), safeString1safe.get());
    result = safeString1unsafe.get().append(string1);
    QVERIFY(!safeString1unsafe.isSafe());
    QVERIFY(!result.isSafe());
    QCOMPARE(result.get(), safeString1unsafe.get());
}

void TestSafeString::testChopping()
{
    auto string1 = QStringLiteral("this & that");
    SafeString safeString1unsafe(string1);
    SafeString safeString1safe(string1, SafeString::IsSafe);

    safeString1safe.get().chop(4);
    QVERIFY(!safeString1safe.isSafe());
    QCOMPARE(QStringLiteral("this & "), QString(safeString1safe.get()));
    safeString1unsafe.get().chop(4);
    QVERIFY(!safeString1unsafe.isSafe());
    QCOMPARE(QStringLiteral("this & "), QString(safeString1unsafe.get()));
}

void TestSafeString::testReplacing()
{
    auto string1 = QStringLiteral("x&\ny");
    SafeString safeString1safe(string1, SafeString::IsSafe);

    auto result = safeString1safe.get().replace(QLatin1Char('\n'), QStringLiteral("<br />"));
    QVERIFY(!result.isSafe());
    QVERIFY(!safeString1safe.isSafe());
    QCOMPARE(result.get(), safeString1safe.get());
}

QTEST_MAIN(TestSafeString)
#include "testsafestring.moc"

#endif
