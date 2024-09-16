// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/sys_info.h"
#include "system/packet.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_SysInfo: public ::testing::Test
{
public:
    UT_SysInfo() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new SysInfo();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    SysInfo *m_tester;
};

TEST_F(UT_SysInfo, initTest)
{
}

TEST_F(UT_SysInfo, test_copy)
{
    const SysInfo rhs;
    m_tester->operator=(rhs);
}

TEST_F(UT_SysInfo, test_instance)
{
    EXPECT_TRUE(m_tester->instance() != nullptr);
}

TEST_F(UT_SysInfo, test_readSockStat)
{
    SockStatMap m_sockStats {};
    m_tester->readSockStat(m_sockStats);
//    EXPECT_TRUE(m_tester->readSockStat(m_sockStats) != true);
}

TEST_F(UT_SysInfo, test_readSysInfo)
{
    EXPECT_TRUE(m_tester->d->version  != true);
}

TEST_F(UT_SysInfo, test_read_file_nr_01)
{
//    sscanf =1
    EXPECT_NE(m_tester->read_file_nr(), 0);
}

TEST_F(UT_SysInfo, test_read_file_nr_02)
{
    EXPECT_NE(m_tester->read_file_nr(), 0);
}

TEST_F(UT_SysInfo, test_read_threads)
{
    EXPECT_TRUE(m_tester->read_threads()  != 0);
}

TEST_F(UT_SysInfo, test_read_hostname)
{
    m_tester->read_hostname();
}


TEST_F(UT_SysInfo, test_read_arch)
{
    EXPECT_TRUE(m_tester->read_arch()  != "");
}

TEST_F(UT_SysInfo, test_read_version)
{
    EXPECT_TRUE(m_tester->read_version()  != "");
}

TEST_F(UT_SysInfo, test_read_uptime)
{
    m_tester->read_uptime(m_tester->d->uptime);
//    EXPECT_TRUE(m_tester->d->uptime.tv_sec  != 0 || m_tester->d->uptime.tv_usec  != 0);
}

TEST_F(UT_SysInfo, test_read_btime)
{
    m_tester->read_btime(m_tester->d->uptime);
//    EXPECT_TRUE(m_tester->d->uptime.tv_sec  != 0 || m_tester->d->uptime.tv_usec  != 0);
}

TEST_F(UT_SysInfo, test_btime)
{
    m_tester->btime();
}

TEST_F(UT_SysInfo, test_hostname)
{
    m_tester->hostname();
}

TEST_F(UT_SysInfo, test_read_loadavg)
{
    m_tester->read_loadavg(m_tester->d->loadAvg);
}
