/*
    This software is a contribution of the LiMux project of the city of Munich.
    SPDX-FileCopyrightText: 2021 Robert Hoffmann <robert@roberthoffmann.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "knetworkmountsteststatic.h"

// include static functions
#include "knetworkmounts_p.h"

#include <KNetworkMounts>

#include <QFile>
#include <QStandardPaths>
#include <QTest>

QTEST_MAIN(KNetworkMountsTestStatic)

void KNetworkMountsTestStatic::testStaticFunctions_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QStringList>("paths");
    QTest::addColumn<bool>("expected_is_slash_added_to_path");
    QTest::addColumn<QString>("expected_path_str");
    QTest::addColumn<bool>("expected_is_slash_added_to_paths");
    QTest::addColumn<QStringList>("expected_paths_str");
    QTest::addColumn<QString>("expected_matching");

    QTest::newRow("empty1") << QString() << QStringList() << false << "" << false << QStringList() << QString();
    QTest::newRow("empty2") << "" << QStringList{QString(), QString()} << false << "" << false << QStringList{QString(), QString()} << QString();

    QTest::newRow("/1") << "/" << QStringList() << false << "/" << false << QStringList() << QString();
    QTest::newRow("/2") << "/" << QStringList{QString(), QString()} << false << "/" << false << QStringList{QString(), QString()} << QString();
    QTest::newRow("/3") << "/" << (QStringList{QStringLiteral("/")}) << false << "/" << false << QStringList{QStringLiteral("/")} << "/";
    QTest::newRow("/4") << "/" << QStringList{QStringLiteral("/"), QString()} << false << "/" << false << QStringList{QStringLiteral("/"), QString()} << "/";

    QTest::newRow("/mnt1") << "/mnt" << QStringList() << true << "/mnt/" << false << QStringList() << QString();
    QTest::newRow("/mnt2") << "/mnt" << QStringList{QStringLiteral("/mnt")} << true << "/mnt/" << true << QStringList{QStringLiteral("/mnt/")} << "/mnt";
    QTest::newRow("/mnt3") << "/mnt" << QStringList{QStringLiteral("/mnt/")} << true << "/mnt/" << false << QStringList{QStringLiteral("/mnt/")} << "/mnt/";

    QTest::newRow("/mnt/test1") << "/mnt" << (QStringList() << QStringLiteral("/mnt/test1") << QStringLiteral("/mnt/test2/")) << true << "/mnt/" << true
                                << QStringList{QStringLiteral("/mnt/test1/"), QStringLiteral("/mnt/test2/")} << "";
    QTest::newRow("/mnt/test2") << "/mnt/test2" << QStringList{QStringLiteral("/mnt/test1/"), QStringLiteral("/mnt/test2/")} << true << "/mnt/test2/" << false
                                << QStringList{QStringLiteral("/mnt/test1/"), QStringLiteral("/mnt/test2/")} << "/mnt/test2/";
    QTest::newRow("/mnt/test3") << "/mnt/test2/" << (QStringList() << QStringLiteral("/mnt/test1/") << QStringLiteral("/mnt/test2/")) << false << "/mnt/test2/"
                                << false << QStringList{QStringLiteral("/mnt/test1/"), QStringLiteral("/mnt/test2/")} << "/mnt/test2/";
}

void KNetworkMountsTestStatic::testStaticFunctions()
{
    QFETCH(QString, path);
    QFETCH(QStringList, paths);
    QFETCH(bool, expected_is_slash_added_to_path);
    QFETCH(QString, expected_path_str);
    QFETCH(bool, expected_is_slash_added_to_paths);
    QFETCH(QStringList, expected_paths_str);
    QFETCH(QString, expected_matching);

    QCOMPARE(getMatchingPath(path, paths), expected_matching);

    QCOMPARE(ensureTrailingSlash(&path), expected_is_slash_added_to_path);
    QCOMPARE(path, expected_path_str);
    QCOMPARE(ensureTrailingSlashes(&paths), expected_is_slash_added_to_paths);
    QCOMPARE(paths, expected_paths_str);
}

void KNetworkMountsTestStatic::testStaticKNetworkMountOptionToString_data()
{
    QTest::addColumn<KNetworkMounts::KNetworkMountOption>("option");
    QTest::addColumn<QString>("string");

    QTest::newRow("LowSideEffectsOptimizations") << KNetworkMounts::LowSideEffectsOptimizations << "LowSideEffectsOptimizations";
    QTest::newRow("MediumSideEffectsOptimizations") << KNetworkMounts::MediumSideEffectsOptimizations << "MediumSideEffectsOptimizations";
    QTest::newRow("StrongSideEffectsOptimizations") << KNetworkMounts::StrongSideEffectsOptimizations << "StrongSideEffectsOptimizations";
    QTest::newRow("KDirWatchDontAddWatches") << KNetworkMounts::KDirWatchDontAddWatches << "KDirWatchDontAddWatches";
    QTest::newRow("SymlinkPathsUseCache") << KNetworkMounts::SymlinkPathsUseCache << "SymlinkPathsUseCache";
}
void KNetworkMountsTestStatic::testStaticKNetworkMountOptionToString()
{
    QFETCH(KNetworkMounts::KNetworkMountOption, option);
    QFETCH(QString, string);

    QCOMPARE(enumToString(option), string);
}

void KNetworkMountsTestStatic::testStaticKNetworkMountsTypeToString_data()
{
    QTest::addColumn<KNetworkMounts::KNetworkMountsType>("type");
    QTest::addColumn<QString>("string");

    QTest::newRow("NfsPaths") << KNetworkMounts::NfsPaths << "NfsPaths";
    QTest::newRow("SmbPaths") << KNetworkMounts::SmbPaths << "SmbPaths";
    QTest::newRow("SymlinkDirectory") << KNetworkMounts::SymlinkDirectory << "SymlinkDirectory";
    QTest::newRow("SymlinkToNetworkMount") << KNetworkMounts::SymlinkToNetworkMount << "SymlinkToNetworkMount";
    QTest::newRow("Any") << KNetworkMounts::Any << "Any";
}
void KNetworkMountsTestStatic::testStaticKNetworkMountsTypeToString()
{
    QFETCH(KNetworkMounts::KNetworkMountsType, type);
    QFETCH(QString, string);

    QCOMPARE(enumToString(type), string);
}

#include "moc_knetworkmountsteststatic.cpp"
