/*
   This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2012 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

class KSharedConfigTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testUnicity();
    void testReadWrite();
    void testReadWriteSync();
    void testQrcFile();

private:
    QString m_path;
};

void KSharedConfigTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    m_path =
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + QCoreApplication::applicationName() + QStringLiteral("rc");
    QFile::remove(m_path);
}

void KSharedConfigTest::testUnicity()
{
    KSharedConfig::Ptr cfg1 = KSharedConfig::openConfig();
    KSharedConfig::Ptr cfg2 = KSharedConfig::openConfig();
    QCOMPARE(cfg1.data(), cfg2.data());
}

void KSharedConfigTest::testReadWrite()
{
    const int value = 1;
    {
        KConfigGroup cg(KSharedConfig::openConfig(), "KSharedConfigTest");
        cg.writeEntry("NumKey", value);
    }
    {
        KConfigGroup cg(KSharedConfig::openConfig(), "KSharedConfigTest");
        QCOMPARE(cg.readEntry("NumKey", 0), 1);
    }
}

void KSharedConfigTest::testReadWriteSync()
{
    const int value = 1;
    {
        KConfigGroup cg(KSharedConfig::openConfig(), "KSharedConfigTest");
        cg.writeEntry("NumKey", value);
    }
    QVERIFY(!QFile::exists(m_path));
    QVERIFY(KSharedConfig::openConfig()->sync());
    QVERIFY(QFile::exists(m_path));
    {
        KConfigGroup cg(KSharedConfig::openConfig(), "KSharedConfigTest");
        QCOMPARE(cg.readEntry("NumKey", 0), 1);
    }
}

void KSharedConfigTest::testQrcFile()
{
    QVERIFY(QFile::exists(QStringLiteral(":/testdata/test.ini")));
    KSharedConfig::Ptr sharedConfig = KSharedConfig::openConfig(QStringLiteral(":/testdata/test.ini"), KConfig::NoGlobals);
    QVERIFY(sharedConfig);

    KConfigGroup cfg(sharedConfig, QStringLiteral("MainSection"));
    QCOMPARE(cfg.readEntry(QStringLiteral("TestEntry"), QStringLiteral("UnexpectedData")), QStringLiteral("ExpectedData"));
}

QTEST_MAIN(KSharedConfigTest)

#include "ksharedconfigtest.moc"
