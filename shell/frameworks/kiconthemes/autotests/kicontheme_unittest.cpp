/*
    SPDX-FileCopyrightText: 2016 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QStandardPaths>
#include <QTest>

#include <kicontheme.h>

class KIconTheme_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void init()
    {
        KIconTheme::forceThemeForTests(QString()); // Reset override.
    }

    void testForceThemeForTests()
    {
        auto forcedName = QStringLiteral("kitten");
        auto resolvedCurrent = KIconTheme::current();
        QVERIFY2(KIconTheme::current() != forcedName, "current theme initially expected to not be mangled");
        // Force a specific theme.
        KIconTheme::forceThemeForTests(forcedName);
        QCOMPARE(KIconTheme::current(), forcedName);
        // Reset override.
        KIconTheme::forceThemeForTests(QString());
        QCOMPARE(KIconTheme::current(), resolvedCurrent);
        // And then override again to make sure we still can.
        KIconTheme::forceThemeForTests(forcedName);
        QCOMPARE(KIconTheme::current(), forcedName);
    }
};

QTEST_MAIN(KIconTheme_UnitTest)

#include "kicontheme_unittest.moc"
