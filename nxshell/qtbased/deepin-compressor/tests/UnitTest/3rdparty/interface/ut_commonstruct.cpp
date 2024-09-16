// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commonstruct.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>


/*******************************函数打桩************************************/

/*******************************函数打桩************************************/


// 测试FileEntry
class UT_FileEntry : public ::testing::Test
{
public:
    UT_FileEntry(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new FileEntry;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    FileEntry *m_tester;
};

TEST_F(UT_FileEntry, initTest)
{

}

TEST_F(UT_FileEntry, UT_FileEntry_reset)
{
    EXPECT_EQ(m_tester->iIndex, -1);
}


// 测试ArchiveData
class UT_ArchiveData : public ::testing::Test
{
public:
    UT_ArchiveData(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ArchiveData;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ArchiveData *m_tester;
};

TEST_F(UT_ArchiveData, initTest)
{

}

TEST_F(UT_ArchiveData, test_reset)
{
    EXPECT_EQ(m_tester->strPassword.isEmpty(), true);
}


// 测试UpdateOptions
class UT_UpdateOptions : public ::testing::Test
{
public:
    UT_UpdateOptions(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new UpdateOptions;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    UpdateOptions *m_tester;
};

TEST_F(UT_UpdateOptions, initTest)
{

}

TEST_F(UT_UpdateOptions, test_reset)
{
    EXPECT_EQ(m_tester->qSize, 0);
}
