/*
    This software is a contribution of the LiMux project of the city of Munich.
    SPDX-FileCopyrightText: 2021 Robert Hoffmann <robert@roberthoffmann.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "knetworkmountstestnoconfig.h"

#include <KNetworkMounts>

#include <QFile>
#include <QStandardPaths>
#include <QTest>

QTEST_MAIN(KNetworkMountsTestNoConfig)

void KNetworkMountsTestNoConfig::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    m_configFileName = QStringLiteral("%1/network_mounts").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));

    QFile::remove(m_configFileName);
    QVERIFY(!QFile::exists(m_configFileName));
}

void KNetworkMountsTestNoConfig::cleanupTestCase()
{
    QVERIFY(!QFile::exists(m_configFileName));
    QVERIFY(!KNetworkMounts::self()->isEnabled());

    KNetworkMounts::self()->sync();
    QFile::remove(m_configFileName);
}

void KNetworkMountsTestNoConfig::testNoConfigPathTypes_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<KNetworkMounts::KNetworkMountsType>("type");

    QTest::newRow("NfsPaths/") << "/" << KNetworkMounts::NfsPaths;
    QTest::newRow("SmbPaths/") << "/" << KNetworkMounts::SmbPaths;
    QTest::newRow("SymlinkDirectory/") << "/" << KNetworkMounts::SymlinkDirectory;
    QTest::newRow("SymlinkToNetworkMount/") << "/" << KNetworkMounts::SymlinkToNetworkMount;
    QTest::newRow("Any/") << "/" << KNetworkMounts::Any;

    QTest::newRow("NfsPaths/mnt") << "/mnt" << KNetworkMounts::NfsPaths;
    QTest::newRow("SmbPaths/mnt") << "/mnt" << KNetworkMounts::SmbPaths;
    QTest::newRow("SymlinkDirectory/mnt") << "/mnt" << KNetworkMounts::SymlinkDirectory;
    QTest::newRow("SymlinkToNetworkMount/mnt") << "/mnt" << KNetworkMounts::SymlinkToNetworkMount;
    QTest::newRow("Any/mnt") << "/mnt" << KNetworkMounts::Any;

    QTest::newRow("NfsPaths/mnt/") << "/mnt/" << KNetworkMounts::NfsPaths;
    QTest::newRow("SmbPaths/mnt/") << "/mnt/" << KNetworkMounts::SmbPaths;
    QTest::newRow("SymlinkDirectory/mnt/") << "/mnt/" << KNetworkMounts::SymlinkDirectory;
    QTest::newRow("SymlinkToNetworkMount/mnt/") << "/mnt/" << KNetworkMounts::SymlinkToNetworkMount;
    QTest::newRow("Any/mnt/") << "/mnt/" << KNetworkMounts::Any;
}

void KNetworkMountsTestNoConfig::testNoConfigPathTypes()
{
    QFETCH(QString, path);
    QFETCH(KNetworkMounts::KNetworkMountsType, type);

    QVERIFY(!QFile::exists(m_configFileName));
    QVERIFY(!KNetworkMounts::self()->isEnabled());

    QCOMPARE(KNetworkMounts::self()->paths(type), QStringList());
    QCOMPARE(KNetworkMounts::self()->paths(), QStringList());
    QCOMPARE(KNetworkMounts::self()->canonicalSymlinkPath(path), path);
    QVERIFY(!KNetworkMounts::self()->isSlowPath(path, type));
    QVERIFY(!KNetworkMounts::self()->isSlowPath(path));
}

void KNetworkMountsTestNoConfig::testNoConfigPathOptions_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<KNetworkMounts::KNetworkMountOption>("option");

    QTest::newRow("LowSideEffectsOptimizations/") << "/" << KNetworkMounts::LowSideEffectsOptimizations;
    QTest::newRow("MediumSideEffectsOptimizations/") << "/" << KNetworkMounts::MediumSideEffectsOptimizations;
    QTest::newRow("StrongSideEffectsOptimizations/") << "/" << KNetworkMounts::StrongSideEffectsOptimizations;
    QTest::newRow("KDirWatchDontAddWatches/") << "/" << KNetworkMounts::KDirWatchDontAddWatches;
    QTest::newRow("SymlinkPathsUseCache/") << "/" << KNetworkMounts::SymlinkPathsUseCache;

    QTest::newRow("LowSideEffectsOptimizations/mnt") << "/mnt" << KNetworkMounts::LowSideEffectsOptimizations;
    QTest::newRow("MediumSideEffectsOptimizations/mnt") << "/mnt" << KNetworkMounts::MediumSideEffectsOptimizations;
    QTest::newRow("StrongSideEffectsOptimizations/mnt") << "/mnt" << KNetworkMounts::StrongSideEffectsOptimizations;
    QTest::newRow("KDirWatchDontAddWatches/mnt") << "/mnt" << KNetworkMounts::KDirWatchDontAddWatches;
    QTest::newRow("SymlinkPathsUseCache/mnt") << "/mnt" << KNetworkMounts::SymlinkPathsUseCache;

    QTest::newRow("LowSideEffectsOptimizations/mnt/") << "/mnt/" << KNetworkMounts::LowSideEffectsOptimizations;
    QTest::newRow("MediumSideEffectsOptimizations/mnt/") << "/mnt/" << KNetworkMounts::MediumSideEffectsOptimizations;
    QTest::newRow("StrongSideEffectsOptimizations/mnt/") << "/mnt/" << KNetworkMounts::StrongSideEffectsOptimizations;
    QTest::newRow("KDirWatchDontAddWatches/mnt/") << "/mnt/" << KNetworkMounts::KDirWatchDontAddWatches;
    QTest::newRow("SymlinkPathsUseCache/mnt/") << "/mnt/" << KNetworkMounts::SymlinkPathsUseCache;
}

void KNetworkMountsTestNoConfig::testNoConfigPathOptions()
{
    QFETCH(QString, path);
    QFETCH(KNetworkMounts::KNetworkMountOption, option);

    QVERIFY(!KNetworkMounts::self()->isOptionEnabledForPath(path, option));
}

void KNetworkMountsTestNoConfig::testNoConfigOptions_data()
{
    QTest::addColumn<KNetworkMounts::KNetworkMountOption>("option");
    QTest::addColumn<bool>("default_value");
    QTest::addColumn<bool>("expected_value");

    QTest::newRow("LowSideEffectsOptimizations_false") << KNetworkMounts::LowSideEffectsOptimizations << false << false;
    QTest::newRow("LowSideEffectsOptimizations_true") << KNetworkMounts::LowSideEffectsOptimizations << true << true;

    QTest::newRow("MediumSideEffectsOptimizationss_false") << KNetworkMounts::MediumSideEffectsOptimizations << false << false;
    QTest::newRow("MediumSideEffectsOptimizations_true") << KNetworkMounts::MediumSideEffectsOptimizations << true << true;

    QTest::newRow("StrongSideEffectsOptimizations_false") << KNetworkMounts::StrongSideEffectsOptimizations << false << false;
    QTest::newRow("StrongSideEffectsOptimizationss_true") << KNetworkMounts::StrongSideEffectsOptimizations << true << true;

    QTest::newRow("KDirWatchDontAddWatches_false") << KNetworkMounts::KDirWatchDontAddWatches << false << false;
    QTest::newRow("KDirWatchDontAddWatches_true") << KNetworkMounts::KDirWatchDontAddWatches << true << true;

    QTest::newRow("SymlinkPathsUseCache_false") << KNetworkMounts::SymlinkPathsUseCache << false << false;
    QTest::newRow("SymlinkPathsUseCache_true") << KNetworkMounts::SymlinkPathsUseCache << true << true;
}

void KNetworkMountsTestNoConfig::testNoConfigOptions()
{
    QFETCH(KNetworkMounts::KNetworkMountOption, option);
    QFETCH(bool, default_value);
    QFETCH(bool, expected_value);

    QCOMPARE(KNetworkMounts::self()->isOptionEnabled(option, default_value), expected_value);
}

#include "moc_knetworkmountstestnoconfig.cpp"
