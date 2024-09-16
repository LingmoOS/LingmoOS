// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/block_device_info_db.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

//qt
#include <QString>

using namespace core::system;

/***************************************STUB begin*********************************************/

//void stub_ReadDeviceInfo()
//{
//    core::system::BlockDevice blockDevice;
//}

/***************************************STUB end**********************************************/

class UT_BlockDeviceInfoDB: public ::testing::Test
{
public:
    UT_BlockDeviceInfoDB() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new BlockDeviceInfoDB();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    BlockDeviceInfoDB *m_tester;
};

TEST_F(UT_BlockDeviceInfoDB, initTest)
{
}

TEST_F(UT_BlockDeviceInfoDB, test_readDiskInfo)
{
    BlockDevice block{};
    m_tester->m_deviceList.append(block);
    m_tester->readDiskInfo();
    EXPECT_NE(m_tester->m_deviceList.size(), 0);
}

TEST_F(UT_BlockDeviceInfoDB, test_update)
{
    m_tester->readDiskInfo();
    EXPECT_NE(m_tester->m_deviceList.size(), 0);
}
