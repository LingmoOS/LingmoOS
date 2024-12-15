// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QDir>
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
#include <filesystem>  //Avoid changing the access control of the standard library
#endif
#define private public
#include "filesystem/dfilesystemwatcher.h"
#undef private

DCORE_USE_NAMESPACE


class ut_DFileSystemWatcher : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DFileSystemWatcher *fileSystemWatcher = nullptr;

};

void ut_DFileSystemWatcher::SetUp()
{
    fileSystemWatcher = new DFileSystemWatcher(nullptr);
    QDir dir0("/tmp/etc0/");
    if (!dir0.exists())
        dir0.mkdir("/tmp/etc0/");
    QDir dir1("/tmp/etc1/");
    if (!dir1.exists())
        dir1.mkdir("/tmp/etc1/");
}

void ut_DFileSystemWatcher::TearDown()
{
    if (fileSystemWatcher) {
        delete fileSystemWatcher;
        fileSystemWatcher = nullptr;
    }
    QDir dir0("/tmp/etc0/");
    if (dir0.exists())
        dir0.remove("/tmp/etc0/");
    QDir dir1("/tmp/etc1/");
    if (dir1.exists())
        dir1.remove("/tmp/etc1/");
}

TEST_F(ut_DFileSystemWatcher, testDFileSystemWatcherAddPath)
{
    if (!fileSystemWatcher->d_func()) return;

    fileSystemWatcher->addPath("/tmp/etc0");

    QStringList dirs = fileSystemWatcher->directories();
    ASSERT_TRUE(dirs.contains("/tmp/etc0"));
}

TEST_F(ut_DFileSystemWatcher, testDFileSystemWatcherAddPaths)
{
    if (!fileSystemWatcher->d_func()) return;

    fileSystemWatcher->addPaths( QStringList() << "/tmp/etc0" << "/tmp/etc1");
    QStringList dirs = fileSystemWatcher->directories();
    ASSERT_TRUE(dirs.contains("/tmp/etc0"));
    ASSERT_TRUE(dirs.contains("/tmp/etc1"));
}

TEST_F(ut_DFileSystemWatcher, testDFileSystemWatcherRemovePath)
{
    if (!fileSystemWatcher->d_func()) return;

    fileSystemWatcher->addPath("/tmp/etc0");
    QStringList dirs0 = fileSystemWatcher->directories();
    ASSERT_TRUE(dirs0.contains("/tmp/etc0"));
    fileSystemWatcher->removePath("/tmp/etc0");
    QStringList dirs1 = fileSystemWatcher->directories();
    ASSERT_FALSE(dirs1.contains("/tmp/etc0"));
}

TEST_F(ut_DFileSystemWatcher, testDFileSystemWatcherRemovePaths)
{
    if (!fileSystemWatcher->d_func()) return;

    fileSystemWatcher->addPaths( QStringList() << "/tmp/etc0" << "/tmp/etc1");
    QStringList dirs0 = fileSystemWatcher->directories();
    ASSERT_TRUE(dirs0.contains("/tmp/etc0"));
    ASSERT_TRUE(dirs0.contains("/tmp/etc1"));
    fileSystemWatcher->removePaths(QStringList() << "/tmp/etc0" << "/tmp/etc1");
    QStringList dirs1 = fileSystemWatcher->directories();
    ASSERT_FALSE(dirs1.contains("/tmp/etc0"));
    ASSERT_FALSE(dirs1.contains("/tmp/etc1"));
}
