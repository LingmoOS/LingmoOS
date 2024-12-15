// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/device_db.h"
#include "system/netif_info_db.h"
#include "system/block_device_info_db.h"
#include "system/mem.h"
#include "system/cpu_set.h"
#include "system/diskio_info.h"
#include "system/net_info.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_DeviceDB: public ::testing::Test
{
public:
    UT_DeviceDB() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DeviceDB();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    DeviceDB *m_tester;
};

TEST_F(UT_DeviceDB, initTest)
{
}

TEST_F(UT_DeviceDB, test_update)
{
    m_tester->update();
    sleep(2);
    m_tester->update();

}

TEST_F(UT_DeviceDB, test_instance)
{
    EXPECT_TRUE(m_tester->instance() != nullptr);
}

TEST_F(UT_DeviceDB, test_blockDeviceInfoDB)
{
    m_tester->update();
    EXPECT_NE(m_tester->blockDeviceInfoDB()->deviceList().size(), 0);
}

TEST_F(UT_DeviceDB, test_netifInfoDB)
{
}

TEST_F(UT_DeviceDB, test_memInfo)
{
    m_tester->update();
    EXPECT_NE(m_tester->memInfo()->memTotal(), 0);
}

TEST_F(UT_DeviceDB, test_cpuSet)
{
    m_tester->update();
    EXPECT_NE(m_tester->cpuSet()->cpuCount(), 0);
}

TEST_F(UT_DeviceDB, test_diskIoInfo)
{
    m_tester->update();
    EXPECT_TRUE(m_tester->diskIoInfo()->m_readBps != 0 || m_tester->diskIoInfo()->m_writeBps != 0);
}

TEST_F(UT_DeviceDB, test_netInfo)
{
    m_tester->update();
    sleep(2);
    m_tester->update();

}


