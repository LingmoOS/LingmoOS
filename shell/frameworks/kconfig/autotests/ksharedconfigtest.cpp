/*
   This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2012 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

using namespace Qt::StringLiterals;

class KSharedConfigTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testUnicity();
    void testReadWrite();
    void testReadWriteSync();
    void testQrcFile();

#if !defined(Q_OS_WINDOWS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_OSX) // m_stateDirPath is only applicable on XDG systems
    void testState()
    {
        auto config = KSharedConfig::openStateConfig();
        config->group(u"Test"_s).writeEntry("Test", true);
        config->sync();
        QVERIFY(QFile::exists(m_stateDirPath + "/"_L1 + QCoreApplication::applicationName() + "staterc"_L1));
    }

    void testStateName()
    {
        auto config = KSharedConfig::openStateConfig(u"foobar"_s);
        config->group(u"Test"_s).writeEntry("Test", true);
        config->sync();
        QVERIFY(QFile::exists(m_stateDirPath + "/foobar"_L1));
    }

    void testStateAbsolute()
    {
        QDir().mkpath(m_stateDirPath);
        const QString path = m_stateDirPath + "/randomsubdir/hellostate"_L1;
        auto config = KSharedConfig::openStateConfig(path);
        config->group(u"Test"_s).writeEntry("Test", true);
        config->sync();
        QVERIFY(QFile::exists(path));
    }

    void testStateMigration()
    {
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        // Migrate from old file to new file if new file doesn't exist yet
        const QString oldPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/"_L1 + QCoreApplication::applicationName() + "staterc"_L1;
        QFile(oldPath).open(QFile::WriteOnly | QFile::Truncate);
        QVERIFY(QFile::exists(oldPath));
        const QString newPath = m_stateDirPath + "/"_L1 + QCoreApplication::applicationName() + "staterc"_L1;
        if (QFile::exists(newPath)) {
            QFile::remove(newPath); // make sure new file doesn't exist so we can write to it
        }

        auto config = KSharedConfig::openStateConfig();
        config->group(u"Test"_s).writeEntry("Test", true);
        config->sync();
        QVERIFY(!QFile::exists(oldPath));
        QVERIFY(QFile::exists(newPath));
    }

    void testStateMigrationClash()
    {
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        // Both old and new staterc exist -> keep both
        const QString oldPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/"_L1 + QCoreApplication::applicationName() + "staterc"_L1;
        QFile(oldPath).open(QFile::WriteOnly | QFile::Truncate);
        QVERIFY(QFile::exists(oldPath));
        QDir().mkpath(m_stateDirPath);
        const QString newPath = m_stateDirPath + "/"_L1 + QCoreApplication::applicationName() + "staterc"_L1;
        QFile(oldPath).open(QFile::WriteOnly | QFile::Truncate);
        QVERIFY(QFile::exists(oldPath));

        auto config = KSharedConfig::openStateConfig();
        config->group(u"Test"_s).writeEntry("Test", true);
        config->sync();
        QVERIFY(QFile::exists(oldPath));
        QVERIFY(QFile::exists(newPath));
    }
#endif

private:
    QString m_path;
    QString m_stateDirPath;
};

void KSharedConfigTest::initTestCase()
{
    constexpr auto xdgStateHomeVar = "XDG_STATE_HOME";
    const QString xdgStateHome = QDir::homePath() + u"/.qttest/state"_s;
    qputenv(xdgStateHomeVar, xdgStateHome.toUtf8());
    QStandardPaths::setTestModeEnabled(true);

    m_path =
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + QCoreApplication::applicationName() + QStringLiteral("rc");
    QFile::remove(m_path);

    m_stateDirPath = xdgStateHome;
    QFile::remove(m_stateDirPath);
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
        KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KSharedConfigTest"));
        cg.writeEntry("NumKey", value);
    }
    {
        KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KSharedConfigTest"));
        QCOMPARE(cg.readEntry("NumKey", 0), 1);
    }
}

void KSharedConfigTest::testReadWriteSync()
{
    const int value = 1;
    {
        KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KSharedConfigTest"));
        cg.writeEntry("NumKey", value);
    }
    QVERIFY(!QFile::exists(m_path));
    QVERIFY(KSharedConfig::openConfig()->sync());
    QVERIFY(QFile::exists(m_path));
    {
        KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KSharedConfigTest"));
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
