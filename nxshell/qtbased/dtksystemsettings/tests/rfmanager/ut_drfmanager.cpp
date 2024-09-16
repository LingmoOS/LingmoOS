// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include <drfmanager.h>

DRFMGR_USE_NAMESPACE

class TestDRfmanager : public testing::Test
{
public:
    // 在测试套件的第一个测试用例开始前，SetUpTestCase 函数会被调用
    static void SetUpTestCase() { rfmanager = new DRfmanager(); }

    // 在测试套件中的最后一个测试用例运行结束后，TearDownTestCase 函数会被调用
    static void TearDownTestCase()
    {
        delete rfmanager;
        rfmanager = nullptr;
    }

    // 每个测试用例开始前，SetUp 函数都会被被调用
    void SetUp() override { }

    // 每个测试用例运行结束后，TearDown 函数都会被被调用
    void TearDown() override { }

    static DRfmanager *rfmanager;
};

DRfmanager *TestDRfmanager::rfmanager = nullptr;

TEST_F(TestDRfmanager, blockType)
{
    //    ASSERT_TRUE(rfmanager->block(DRfmanager::WIFI));
    //    ASSERT_TRUE(rfmanager->isWifiBlocked());
    //    ASSERT_TRUE(rfmanager->block(DRfmanager::WIFI, false));
    ASSERT_FALSE(rfmanager->isWifiBlocked());
}

TEST_F(TestDRfmanager, blockId)
{
    // invalid id
    ASSERT_FALSE(rfmanager->block(123456));
    //    ASSERT_TRUE(rfmanager->block(26));
    //    ASSERT_TRUE(rfmanager->block(26, false));
}

TEST_F(TestDRfmanager, blockbluetooth)
{
    //    ASSERT_TRUE(rfmanager->blockBluetooth());
    //    ASSERT_TRUE(rfmanager->isBluetoothBlocked());
    //    ASSERT_TRUE(rfmanager->blockBluetooth(false));
    ASSERT_FALSE(rfmanager->isBluetoothBlocked());
}

TEST_F(TestDRfmanager, blockWifi)
{
    //    ASSERT_TRUE(rfmanager->blockWifi());
    //    ASSERT_TRUE(rfmanager->isWifiBlocked());
    //    ASSERT_TRUE(rfmanager->blockWifi(false));
    ASSERT_FALSE(rfmanager->isWifiBlocked());
}

TEST_F(TestDRfmanager, blockAll)
{
    //    ASSERT_TRUE(rfmanager->blockAll());
    //    ASSERT_TRUE(rfmanager->isAllBlocked());
    //    ASSERT_TRUE(rfmanager->blockAll(false));
    ASSERT_FALSE(rfmanager->isAllBlocked());
}

TEST_F(TestDRfmanager, count)
{
    rfmanager->count();
}
