// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/diskio_info.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_DiskIOInfo: public ::testing::Test
{
public:
    UT_DiskIOInfo() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DiskIOInfo();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    DiskIOInfo *m_tester;
};

TEST_F(UT_DiskIOInfo, initTest)
{
}

TEST_F(UT_DiskIOInfo, test_diskIoReadBps)
{
    m_tester->update();
    EXPECT_NE(m_tester->diskIoReadBps(), 0);
}

TEST_F(UT_DiskIOInfo, test_diskIoWriteBps)
{
    m_tester->update();
    EXPECT_NE(m_tester->diskIoWriteBps(), 0);
}

TEST_F(UT_DiskIOInfo, test_readDiskIOStats)
{
    m_tester->update();
    EXPECT_TRUE(m_tester->m_diskIoStatMap[1].size()>0);
}

TEST_F(UT_DiskIOInfo, test_calDiskIoStates)
{
    m_tester->update();
    EXPECT_NE(m_tester->diskIoReadBps(), 0);
}

TEST_F(UT_DiskIOInfo, test_update)
{
    m_tester->update();
    EXPECT_TRUE(m_tester->diskIoReadBps() != 0 || m_tester->diskIoWriteBps() != 0);
}

