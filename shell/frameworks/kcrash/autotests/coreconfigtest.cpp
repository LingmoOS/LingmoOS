/*
    SPDX-FileCopyrightText: 2016-2021 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>

#include "../src/coreconfig.cpp"

class CoreConfigTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
    }

    void testExec()
    {
        KCrash::CoreConfig c(QFINDTESTDATA("core_patterns/exec"));
#if KCRASH_CORE_PATTERN_RAISE
        QCOMPARE(c.isProcess(), true);
        QCOMPARE(c.isCoredumpd(), true);
#else
        QCOMPARE(c.isProcess(), false);
        QCOMPARE(c.isCoredumpd(), false);
#endif
    }

    void testExecNot()
    {
#if !KCRASH_CORE_PATTERN_RAISE
        QSKIP("Not useful when opting out of re-raising.")
#endif

        KCrash::CoreConfig c(QFINDTESTDATA("core_patterns/exec-apport"));
        QCOMPARE(c.isProcess(), true);
        QCOMPARE(c.isCoredumpd(), false);
    }

    void testNoFile()
    {
        KCrash::CoreConfig c(QStringLiteral("/meow/kitteh/meow"));
        QCOMPARE(c.isProcess(), false);
    }

    void testNoExec()
    {
        KCrash::CoreConfig c(QFINDTESTDATA("core_patterns/no-exec"));
        QCOMPARE(c.isProcess(), false);
    }
};

QTEST_MAIN(CoreConfigTest)

#include "coreconfigtest.moc"
