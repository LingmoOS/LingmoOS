// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/mem.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

/***************************************STUB begin*********************************************/

int stub_sscanf (const char *__restrict __s, const char *__restrict __format, ...)
{
    return 0;
}

FILE *stub_fopen_mem (const char *__restrict __filename,
 const char *__restrict __modes)
{
    return nullptr;
}

/***************************************STUB end**********************************************/


class UT_MemInfo: public ::testing::Test
{
public:
    UT_MemInfo() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new MemInfo();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    MemInfo *m_tester;
};

TEST_F(UT_MemInfo, initTest)
{
}

TEST_F(UT_MemInfo, test_copy_01)
{
    MemInfo copy(*m_tester);
}

TEST_F(UT_MemInfo, test_copy_02)
{
    const MemInfo rhs;
    m_tester->operator=(rhs);
}

TEST_F(UT_MemInfo, test_copy_03)
{
    MemInfo* rhs = m_tester;
    m_tester->operator= (*rhs);
}

TEST_F(UT_MemInfo, test_memTotal)
{
    m_tester->readMemInfo();
    EXPECT_NE(m_tester->memTotal(), 0);
}

TEST_F(UT_MemInfo, test_memAvailable)
{
    m_tester->readMemInfo();
    EXPECT_NE(m_tester->memAvailable(), 0);
}

TEST_F(UT_MemInfo, test_buffers)
{
    m_tester->readMemInfo();
    EXPECT_NE(m_tester->buffers(), 0);
}

TEST_F(UT_MemInfo, test_cached)
{
    m_tester->readMemInfo();
    EXPECT_NE(m_tester->cached(), 0);
}

TEST_F(UT_MemInfo, test_active)
{
    m_tester->readMemInfo();
    EXPECT_NE(m_tester->active(), 0);
}

TEST_F(UT_MemInfo, test_inactive)
{
    m_tester->readMemInfo();
    EXPECT_NE(m_tester->inactive(), 0);
}

TEST_F(UT_MemInfo, test_swapTotal)
{
    m_tester->readMemInfo();
//    EXPECT_NE(m_tester->swapTotal(), 0);
}

TEST_F(UT_MemInfo, test_swapFree)
{
    m_tester->readMemInfo();
//    EXPECT_NE(m_tester->swapFree(), 0);
}

TEST_F(UT_MemInfo, test_swapCached)
{
    m_tester->readMemInfo();
}

TEST_F(UT_MemInfo, test_shmem)
{
    m_tester->readMemInfo();
    EXPECT_NE(m_tester->shmem(), 0);
}

TEST_F(UT_MemInfo, test_slab)
{
    m_tester->readMemInfo();
    EXPECT_NE(m_tester->slab(), 0);
}

TEST_F(UT_MemInfo, test_dirty)
{
    m_tester->readMemInfo();
    EXPECT_NE(m_tester->dirty(), 0);
}

TEST_F(UT_MemInfo, test_mapped)
{
    m_tester->readMemInfo();
    EXPECT_NE(m_tester->mapped(), 0);
}

TEST_F(UT_MemInfo, test_readMemInfo_01)
{
    m_tester->readMemInfo();
    EXPECT_NE(m_tester->memTotal(), 0);
}

TEST_F(UT_MemInfo, test_readMemInfo_02)
{
    Stub stub;
    stub.set(sscanf, stub_sscanf);

    m_tester->readMemInfo();
}

TEST_F(UT_MemInfo, test_readMemInfo_03)
{
    Stub stub;
    stub.set(fopen, stub_fopen_mem);
    m_tester->readMemInfo();
}
