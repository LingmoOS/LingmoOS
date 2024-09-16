// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dprocessdevice.h"
#include <gtest/gtest.h>
#include <stubext.h>

DDEVICE_USE_NAMESPACE


class  TestDProcessDevice : public  testing::Test
{
public:
    static void SetUpTestCase() { device = new DProcessDevice; }
    static void TearDownTestCase() { delete device; }
    static DProcessDevice *device;
};
DProcessDevice *TestDProcessDevice::device = nullptr;
TEST_F(TestDProcessDevice, allPids)
{
    auto expected = device->allPids();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDProcessDevice, groupName)
{
    auto expected = device->groupName(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDProcessDevice, baseInfo)
{
    auto expected = device->baseInfo(0);
    EXPECT_GE(expected.name.size(), 0);
}
TEST_F(TestDProcessDevice, memoryInfo)
{
    auto expected = device->memoryInfo(0);
    EXPECT_GE(expected.vmsize, 0);
}
TEST_F(TestDProcessDevice, ioInfo)
{
    auto expected = device->ioInfo(0);
    EXPECT_GE(expected.readBytes, 0);
}
TEST_F(TestDProcessDevice, networkInfo)
{
    auto expected = device->networkInfo(0);
    EXPECT_GE(expected.recvBytes, 0);
}
TEST_F(TestDProcessDevice, status)
{
    auto expected = device->status(0);
    EXPECT_GE(expected.processor, 0);
}
