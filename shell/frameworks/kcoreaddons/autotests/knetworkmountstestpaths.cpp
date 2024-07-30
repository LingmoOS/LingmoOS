/*
    This software is a contribution of the LiMux project of the city of Munich.
    SPDX-FileCopyrightText: 2021 Robert Hoffmann <robert@roberthoffmann.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "knetworkmountstestpaths.h"

#include <KNetworkMounts>

#include <QFile>
#include <QStandardPaths>
#include <QTest>

QTEST_MAIN(KNetworkMountsTestPaths)

void KNetworkMountsTestPaths::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    m_configFileName = QStringLiteral("%1/network_mounts").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));

    QFile::remove(m_configFileName);
    QVERIFY(!QFile::exists(m_configFileName));

    KNetworkMounts::self()->setEnabled(true);
    QVERIFY(KNetworkMounts::self()->isEnabled());
    KNetworkMounts::self()->sync();
    QVERIFY(QFile::exists(m_configFileName));
    QVERIFY(KNetworkMounts::self()->isEnabled());

    // nfs path
    const QString nfsPath = QStringLiteral("/mnt/nfs");
    const QString savedNfsPath = QStringLiteral("/mnt/nfs/");
    const QStringList savedNfsPaths = {savedNfsPath};
    KNetworkMounts::self()->addPath(nfsPath, KNetworkMounts::NfsPaths);

    QStringList allSavedPaths = savedNfsPaths;

    QCOMPARE(KNetworkMounts::self()->paths(), allSavedPaths);
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SmbPaths), QStringList());
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::NfsPaths), savedNfsPaths);
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SymlinkDirectory), QStringList());
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SymlinkToNetworkMount), QStringList());

    // smb shares
    const QStringList paths = {QStringLiteral("/mnt/server1"), QStringLiteral("/mnt/server2")};
    const QStringList savedSmbPaths = {QStringLiteral("/mnt/server1/"), QStringLiteral("/mnt/server2/")};
    KNetworkMounts::self()->setPaths(paths, KNetworkMounts::SmbPaths);

    allSavedPaths << savedSmbPaths;

    QCOMPARE(KNetworkMounts::self()->paths(), allSavedPaths);
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SmbPaths), savedSmbPaths);
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::NfsPaths), savedNfsPaths);
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SymlinkDirectory), QStringList());
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SymlinkToNetworkMount), QStringList());

    // symlink dir
    const QStringList symlinkDirs = {QStringLiteral("/home/user/netshares")};
    const QStringList savedSymlinkDirs = {QStringLiteral("/home/user/netshares/")};
    KNetworkMounts::self()->setPaths(symlinkDirs, KNetworkMounts::SymlinkDirectory);

    allSavedPaths << savedSymlinkDirs;

    QCOMPARE(KNetworkMounts::self()->paths(), allSavedPaths);
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SmbPaths), savedSmbPaths);
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SymlinkDirectory), savedSymlinkDirs);
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::NfsPaths), savedNfsPaths);
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SymlinkToNetworkMount), QStringList());

    // symlink to nfs or smb
    const QString symlinkToNfs = QStringLiteral("/somedir/symlinkToNfs");
    const QString savedSymlinkToNfs = QStringLiteral("/somedir/symlinkToNfs/");
    const QStringList savedSymlinkToNfsPaths = {savedSymlinkToNfs};
    KNetworkMounts::self()->addPath(symlinkToNfs, KNetworkMounts::SymlinkToNetworkMount);

    allSavedPaths << savedSymlinkToNfsPaths;

    QCOMPARE(KNetworkMounts::self()->paths(), allSavedPaths);
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SmbPaths), savedSmbPaths);
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SymlinkDirectory), savedSymlinkDirs);
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::NfsPaths), savedNfsPaths);
    QCOMPARE(KNetworkMounts::self()->paths(KNetworkMounts::SymlinkToNetworkMount), savedSymlinkToNfsPaths);
}

void KNetworkMountsTestPaths::cleanupTestCase()
{
    KNetworkMounts::self()->sync();
    QFile::remove(m_configFileName);
}

void KNetworkMountsTestPaths::testPaths_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("expected_path_option");
    QTest::addColumn<bool>("expected_path");
    QTest::addColumn<bool>("expected_symlink_dir");
    QTest::addColumn<bool>("expected_symlink_to_nfs_or_smb");
    QTest::addColumn<bool>("expected_nfs");
    QTest::addColumn<bool>("expected_smb");

    QTest::newRow("fast_path") << "/mnt" << false << false << false << false << false << false;
    QTest::newRow("fast_path_slash_end") << "/mnt/" << false << false << false << false << false << false;
    QTest::newRow("slow_path1") << "/mnt/server1" << true << true << false << false << false << true;
    QTest::newRow("slow_path2") << "/mnt/server2" << true << true << false << false << false << true;
    QTest::newRow("slow_path2_dir") << "/mnt/server2/dir" << true << true << false << false << false << true;
    QTest::newRow("slow_path2_dir_subdir") << "/mnt/server2/dir/subdir" << true << true << false << false << false << true;
    QTest::newRow("slow_path2_dir_subdir_slash_end") << "/mnt/server2/dir/subdir/" << true << true << false << false << false << true;
    QTest::newRow("slow_symlink_path") << "/home/user/netshares" << true << true << true << false << false << false;
    QTest::newRow("fast_path_root") << "/" << false << false << false << false << false << false;
    QTest::newRow("fast_path_home") << "/home" << false << false << false << false << false << false;
    QTest::newRow("fast_path_home_user") << "/home/user" << false << false << false << false << false << false;
    QTest::newRow("slow_symlink_path_subdir1") << "/home/user/netshares/subdir1" << true << true << true << false << false << false;
    QTest::newRow("slow_symlink_path_subdir1_subdir2") << "/home/user/netshares/subdir1/subdir2" << true << true << true << false << false << false;
    QTest::newRow("slow_symlink_path_subdir1_subdir2_slash_end") << "/home/user/netshares/subdir1/subdir2/" << true << true << true << false << false << false;
    QTest::newRow("slow_path_nfs") << "/mnt/nfs" << true << true << false << false << true << false;
    QTest::newRow("slow_path_nfs_dir") << "/mnt/nfs/dir" << true << true << false << false << true << false;
    QTest::newRow("slow_path_nfs_dir_subdir") << "/mnt/nfs/dir/subdir" << true << true << false << false << true << false;
    QTest::newRow("slow_path_nfs_dir_subdir_slash_end") << "/mnt/nfs/dir/subdir/" << true << true << false << false << true << false;
    QTest::newRow("slow_path_symlink_to_nfs") << "/somedir/symlinkToNfs" << true << true << false << true << false << false;
    QTest::newRow("slow_path_symlink_to_nfs_dir") << "/somedir/symlinkToNfs/dir" << true << true << false << true << false << false;
    QTest::newRow("slow_path_symlink_to_nfs_dir_subdir") << "/somedir/symlinkToNfs/dir/subdir" << true << true << false << true << false << false;
    QTest::newRow("slow_path_symlink_to_nfs_dir_subdir_slash_end") << "/somedir/symlinkToNfs/dir/subdir/" << true << true << false << true << false << false;
}

void KNetworkMountsTestPaths::testPaths()
{
    QFETCH(QString, path);
    QFETCH(bool, expected_path_option);
    QFETCH(bool, expected_path);
    QFETCH(bool, expected_symlink_dir);
    QFETCH(bool, expected_symlink_to_nfs_or_smb);
    QFETCH(bool, expected_nfs);
    QFETCH(bool, expected_smb);

    QCOMPARE(KNetworkMounts::self()->isOptionEnabledForPath(path, KNetworkMounts::SymlinkPathsUseCache), expected_path_option);
    QCOMPARE(KNetworkMounts::self()->isOptionEnabledForPath(path, KNetworkMounts::KDirWatchDontAddWatches), expected_path_option);
    QCOMPARE(KNetworkMounts::self()->isOptionEnabledForPath(path, KNetworkMounts::LowSideEffectsOptimizations), expected_path_option);
    QCOMPARE(KNetworkMounts::self()->isOptionEnabledForPath(path, KNetworkMounts::MediumSideEffectsOptimizations), expected_path_option);
    QCOMPARE(KNetworkMounts::self()->isOptionEnabledForPath(path, KNetworkMounts::StrongSideEffectsOptimizations), expected_path_option);

    QCOMPARE(KNetworkMounts::self()->isSlowPath(path), expected_path);

    QCOMPARE(KNetworkMounts::self()->isSlowPath(path, KNetworkMounts::KNetworkMountsType::Any), expected_path);

    QCOMPARE(KNetworkMounts::self()->isSlowPath(path, KNetworkMounts::SymlinkDirectory), expected_symlink_dir);

    QCOMPARE(KNetworkMounts::self()->isSlowPath(path, KNetworkMounts::SymlinkToNetworkMount), expected_symlink_to_nfs_or_smb);

    QCOMPARE(KNetworkMounts::self()->isSlowPath(path, KNetworkMounts::NfsPaths), expected_nfs);

    QCOMPARE(KNetworkMounts::self()->isSlowPath(path, KNetworkMounts::SmbPaths), expected_smb);
}

#include "moc_knetworkmountstestpaths.cpp"
