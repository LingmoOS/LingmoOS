// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "datamanager.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>


/*******************************函数打桩************************************/

/*******************************函数打桩************************************/


// 测试DataManager
class UT_DataManager : public ::testing::Test
{
public:
    UT_DataManager(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DataManager;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    DataManager *m_tester;
};

TEST_F(UT_DataManager, initTest)
{

}

TEST_F(UT_DataManager, test_DataManager)
{
    DataManager tester1(*m_tester);
    EXPECT_EQ(tester1.m_stArchiveData.listRootEntry.isEmpty(), true);
}

TEST_F(UT_DataManager, UT_DataManagerre_setArchiveData)
{
    m_tester->m_stArchiveData.qSize = 100;
    m_tester->resetArchiveData();
    EXPECT_EQ(m_tester->m_stArchiveData.qSize, 0);
}

TEST_F(UT_DataManager, test_archiveData)
{
    m_tester->m_stArchiveData.qSize = 100;
    EXPECT_EQ(m_tester->archiveData().qSize, 100);
}

TEST_F(UT_DataManager, test_get_instance)
{
    EXPECT_EQ((&m_tester->get_instance()) == m_tester->m_instance, true);
}

