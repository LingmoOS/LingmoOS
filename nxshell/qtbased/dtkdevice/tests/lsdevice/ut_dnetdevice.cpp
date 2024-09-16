// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "scan.h"
#include "dnetdevice.h"
#include <gtest/gtest.h>
#include <stubext.h>

DDEVICE_USE_NAMESPACE


class  TestDNetDevice : public  testing::Test
{
public:
        static void SetUpTestCase() { 
    stub_ext::StubExt stub;
    typedef bool (*fsetptr) (hwNode & system);;
    fsetptr pathaddr = (fsetptr)&scan_system;
    stub.set_lamda(pathaddr, [] {
        __DBG_STUB_INVOKE__
        return true;
    }); 
    device = new DNetDevice; }
    static void TearDownTestCase() { delete device; }
    static DNetDevice *device;
};
DNetDevice *TestDNetDevice::device = nullptr;

TEST_F(TestDNetDevice, count)
{
    auto expected = device->count();
    EXPECT_GE(expected, 0);
}
//TEST_F(TestDNetDevice, name)
//{
//    auto expected = device->name(0);
//    EXPECT_GE(expected.size(), 0);
//}
TEST_F(TestDNetDevice, vendor)
{
    auto expected = device->vendor(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDNetDevice, model)
{
    auto expected = device->model(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDNetDevice, type)
{
    auto expected = device->type(0);
    EXPECT_GE(expected.size(), 0);
}

TEST_F(TestDNetDevice, macAddress)
{
    auto expected = device->macAddress(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDNetDevice, driver)
{
    auto expected = device->driver(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDNetDevice, speed)
{
    auto expected = device->speed(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDNetDevice, status)
{
    auto expected = device->status(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDNetDevice, inetAddr4)
{
    auto expected = device->inetAddr4(0);
    EXPECT_GE(expected.family, 0);
}
TEST_F(TestDNetDevice, inetAddr6)
{
    auto expected = device->inetAddr6(0);
    EXPECT_GE(expected.family, 0);
}
TEST_F(TestDNetDevice, netifInfo)
{
    auto expected = device->netifInfo(0);
    EXPECT_GE(expected.rxPackets, 0);
}
TEST_F(TestDNetDevice, portStatus)
{
    auto expected = device->portStatus(0, 0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDNetDevice, applicationPorts)
{
    auto expected = device->applicationPorts(0);
    EXPECT_GE(expected.size(), 0);
}
