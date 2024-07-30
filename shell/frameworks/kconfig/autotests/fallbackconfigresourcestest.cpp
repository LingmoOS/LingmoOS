/*
   This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2015 Christoph Cullmann <cullmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

class FallbackConfigResourcesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testResourceFallbackFile();
};

void FallbackConfigResourcesTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void FallbackConfigResourcesTest::testResourceFallbackFile()
{
    KSharedConfig::Ptr sharedConfig = KSharedConfig::openConfig(QStringLiteral("kconfigtesting"), KConfig::NoGlobals);
    QVERIFY(sharedConfig);

    KConfigGroup cfg(sharedConfig, QStringLiteral("MainSection"));
    QCOMPARE(cfg.readEntry(QStringLiteral("TestEntry"), QStringLiteral("UnexpectedData")), QStringLiteral("ExpectedData"));
}

QTEST_MAIN(FallbackConfigResourcesTest)

#include "fallbackconfigresourcestest.moc"
