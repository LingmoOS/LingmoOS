/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Canonical Ltd
    SPDX-FileContributor: Aurélien Gâteau <aurelien.gateau@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "test_kconfigutils.h"

#include <qdebug.h>
#include <qtest.h>

// Local
#include "kconfigutils.h"

QTEST_GUILESS_MAIN(TestKConfigUtils)

void TestKConfigUtils::testParseGroupString_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QStringList>("expected");
    QTest::addColumn<bool>("expectedOk");

    QTest::newRow("simple-group") << " group  " << (QStringList() << QStringLiteral("group")) << true;

    QTest::newRow("sub-group") << "[group][sub]" << (QStringList() << QStringLiteral("group") << QStringLiteral("sub")) << true;

    QTest::newRow("crazy-sub-group") << "[a\\ttab\\x5d[and some hex esc\\x61pe]"
                                     << (QStringList() << QStringLiteral("a\ttab") << QStringLiteral("and some hex escape")) << true;

    QTest::newRow("missing-closing-brace") << "[group][sub" << QStringList() << false;

    QTest::newRow("invalid-escape-string") << "[a\\z]" << QStringList() << false;
}

void TestKConfigUtils::testParseGroupString()
{
    QFETCH(QString, input);
    QFETCH(QStringList, expected);
    QFETCH(bool, expectedOk);

    bool ok;
    QString error;
    QStringList output = KConfigUtils::parseGroupString(input, &ok, &error);
    QCOMPARE(output, expected);
    QCOMPARE(ok, expectedOk);
    if (ok) {
        QVERIFY(error.isEmpty());
    } else {
        QVERIFY(!error.isEmpty());
        qDebug() << error;
    }
}

void TestKConfigUtils::testUnescapeString_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<bool>("expectedOk");

    QTest::newRow("plain") << "Some text"
                           << "Some text" << true;

    QTest::newRow("single-char-escapes") << "01\\s23\\t45\\n67\\r89\\\\"
                                         << "01 23\t45\n67\r89\\" << true;

    QTest::newRow("hex-escapes") << "kd\\x65"
                                 << "kde" << true;

    QTest::newRow("unfinished-hex-escape") << "kd\\x6"
                                           << "" << false;

    QTest::newRow("invalid-hex-escape") << "kd\\xzz"
                                        << "" << false;

    QTest::newRow("invalid-escape-sequence") << "Foo\\a"
                                             << "" << false;

    QTest::newRow("unfinished-escape-sequence") << "Foo\\"
                                                << "" << false;
}

void TestKConfigUtils::testUnescapeString()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);
    QFETCH(bool, expectedOk);

    bool ok;
    QString error;
    QString output = KConfigUtils::unescapeString(input, &ok, &error);
    QCOMPARE(output, expected);
    QCOMPARE(ok, expectedOk);
    if (ok) {
        QVERIFY(error.isEmpty());
    } else {
        QVERIFY(!error.isEmpty());
        qDebug() << error;
    }
}
