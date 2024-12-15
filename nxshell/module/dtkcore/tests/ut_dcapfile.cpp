// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "filesystem/dcapmanager.h"
#include "filesystem/dcapfile.h"

#include <gtest/gtest.h>
#include <QDir>
#include <QDir>
#include <QTemporaryDir>

DCORE_USE_NAMESPACE

#ifndef GTEST_SKIP
#define SKIP return GTEST_SUCCEED() << "Skip all tests"
#else
#define SKIP GTEST_SKIP() << "Skip all tests"
#endif

#define TMPCAP_PATH "/tmp/cap"

class ut_DCapFile : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DCapManager *manager;
    QFile *file;
};

void ut_DCapFile::SetUp()
{
     manager = DCapManager::instance();
     manager->removePath("/tmp");
     manager->appendPath(TMPCAP_PATH);

     file = new QFile(nullptr);
     QDir dir(TMPCAP_PATH);
     if (!dir.exists())
         ASSERT_TRUE(dir.mkdir(TMPCAP_PATH));
}

void ut_DCapFile::TearDown()
{
    manager->appendPath("/tmp");
    delete file;
    QDir dir(TMPCAP_PATH);
    if (dir.exists())
        ASSERT_TRUE(dir.removeRecursively());
}

TEST(ut_DCapManager, paths)
{
    auto size = DCapManager::instance()->paths().size();
    EXPECT_TRUE(size > 0);

    DCapManager::instance()->appendPaths({"/path/to/myCap"});
    EXPECT_TRUE(DCapManager::instance()->paths().contains("/path/to/myCap"));

    DCapManager::instance()->removePaths({"/path/to/myCap"});
    EXPECT_FALSE(DCapManager::instance()->paths().contains("/path/to/myCap"));
}

TEST(ut_DCapFileAndDir, testDCapFileOpen)
{
    DCapFile file("/tmp/test0");
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));  // Default, '/tmp' is allowable for writing.
    file.close();

    DCapManager::instance()->removePath("/tmp");
    ASSERT_FALSE(file.open(DCapFile::WriteOnly));
    DCapManager::instance()->appendPath("/tmp");
}

TEST(ut_DCapFileAndDir, testDCapFileOperation)
{
    auto CheckResult = [](bool result) {
        DCapFile file("/tmp/test0");
        ASSERT_EQ(file.open(DCapFile::WriteOnly), result);
        file.close();

        ASSERT_EQ(file.exists(), result);
        ASSERT_EQ(DCapFile::exists(file.fileName()), result);
        ASSERT_EQ(file.remove(), result);

        ASSERT_EQ(file.open(DCapFile::WriteOnly), result);
        file.close();
        ASSERT_EQ(DCapFile::remove(file.fileName()), result);

        ASSERT_EQ(file.open(DCapFile::WriteOnly), result);
        file.close();
        ASSERT_EQ(file.rename("/tmp/test1"), result);
        ASSERT_EQ(DCapFile::rename(file.fileName(), "/tmp/test0"), result);

        file.setFileName("/tmp/test0");
        ASSERT_EQ(file.link("/tmp/test_link0"), result);
        ASSERT_EQ(DCapFile::link(file.fileName(), "/tmp/test_link1"), result);
        ASSERT_EQ(DCapFile::remove("/tmp/test_link0"), result);
        ASSERT_EQ(DCapFile::remove("/tmp/test_link1"), result);

        file.setFileName("/tmp/test0");
        ASSERT_EQ(file.copy("/tmp/test_copy0"), result);
        ASSERT_EQ(DCapFile::copy(file.fileName(), "/tmp/test_copy1"), result);
        ASSERT_EQ(DCapFile::remove("/tmp/test_copy0"), result);
        ASSERT_EQ(DCapFile::remove("/tmp/test_copy1"), result);

        ASSERT_EQ(file.resize(10), result);
        ASSERT_EQ(file.size() == 10, result);
        ASSERT_EQ(DCapFile::resize(file.fileName(), 5), result);
        ASSERT_EQ(file.size() == 5, result);
        ASSERT_EQ(file.remove(), result);
    };

    CheckResult(true);
    DCapManager::instance()->removePath("/tmp");
    CheckResult(false);
    DCapManager::instance()->appendPath("/tmp");
}

TEST(ut_DCapFileAndDir, testDCapDirOperation)
{
    DCapDir dir("/tmp");
    ASSERT_TRUE(dir.exists());
    ASSERT_FALSE(dir.entryList().isEmpty());
    ASSERT_FALSE(dir.entryInfoList().isEmpty());

    DCapFile file(dir.path() + "/test0");
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.close();
    ASSERT_TRUE(dir.exists("test0"));

    ASSERT_TRUE(dir.mkdir("cap"));
    ASSERT_TRUE(dir.exists("cap"));
    ASSERT_TRUE(dir.rmdir("cap"));
    ASSERT_FALSE(dir.exists("cap"));

    dir.mkdir("cap");
    ASSERT_TRUE(dir.cd("cap"));
    ASSERT_TRUE(dir.exists());
    DCapManager::instance()->removePath("/tmp");
    DCapManager::instance()->appendPath(dir.path());
    ASSERT_FALSE(dir.cd(".."));
    dir.setPath("/tmp");

    ASSERT_TRUE(dir.entryList().isEmpty());
    ASSERT_TRUE(dir.entryInfoList().isEmpty());
    ASSERT_TRUE(dir.exists("cap"));
    ASSERT_FALSE(dir.remove("test0"));
    ASSERT_FALSE(dir.rename("test0", "test1"));

    ASSERT_TRUE(dir.mkpath(TMPCAP_PATH"/subdir"));
    ASSERT_TRUE(dir.rmpath(TMPCAP_PATH"/subdir"));

    DCapManager::instance()->appendPath("/tmp");
    ASSERT_TRUE(dir.rename("test0", "test1"));
    ASSERT_TRUE(dir.remove("test1"));
}
