// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openFileWatcher.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>
#include <QFileSystemWatcher>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试CompressPage
class UT_OpenFileWatcher : public ::testing::Test
{
public:
    UT_OpenFileWatcher(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new OpenFileWatcher(nullptr);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    OpenFileWatcher *m_tester;
};

TEST_F(UT_OpenFileWatcher, initTest)
{

}

TEST_F(UT_OpenFileWatcher, test_reset)
{
    m_tester->m_mapFilePassword["1"] = "1";
    m_tester->reset();
    EXPECT_EQ(m_tester->m_pOpenFileWatcher->files().isEmpty(), true);
    EXPECT_EQ(m_tester->m_mapFilePassword.isEmpty(), true);
    EXPECT_EQ(m_tester->m_mapFileHasModified.isEmpty(), true);
    EXPECT_EQ(m_tester->m_mapFilePassword.isEmpty(), true);
}

TEST_F(UT_OpenFileWatcher, test_setCurOpenFile)
{
    m_tester->setCurOpenFile("1.txt");
    EXPECT_EQ(m_tester->m_strOpenFile, "1.txt");
}

TEST_F(UT_OpenFileWatcher, test_addCurOpenWatchFile)
{
    m_tester->setCurOpenFile("1.txt");
    m_tester->addCurOpenWatchFile();
    EXPECT_EQ(m_tester->m_mapFileHasModified["1.txt"], false);
}

TEST_F(UT_OpenFileWatcher, test_addWatchFile)
{
    m_tester->addWatchFile("1.txt");
    EXPECT_EQ(m_tester->m_mapFileHasModified["1.txt"], false);
}

TEST_F(UT_OpenFileWatcher, test_getFileHasModified)
{
    EXPECT_EQ(m_tester->getFileHasModified(), m_tester->m_mapFileHasModified);
}

TEST_F(UT_OpenFileWatcher, test_setCurFilePassword)
{
    m_tester->setCurOpenFile("1.txt");
    m_tester->addCurOpenWatchFile();
    m_tester->setCurFilePassword("123456");
    EXPECT_EQ(m_tester->m_mapFilePassword["1.txt"], "123456");
}

TEST_F(UT_OpenFileWatcher, testgetFilePassword)
{
    EXPECT_EQ(m_tester->getFilePassword(), m_tester->m_mapFilePassword);
}
