// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "scan.h"
#include "dmotherboarddevice.h"
#include <gtest/gtest.h>
#include <stubext.h>

DDEVICE_USE_NAMESPACE


class  TestDMotherBoardDevice : public  testing::Test
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
    device = new DMotherboardDevice; }
    static void TearDownTestCase() { delete device; }
    static DMotherboardDevice *device;
};
DMotherboardDevice *TestDMotherBoardDevice::device = nullptr;

TEST_F(TestDMotherBoardDevice, vendor)
{
    auto expected = device->vendor();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMotherBoardDevice, model)
{
    auto expected = device->model();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMotherBoardDevice, date)
{
    auto expected = device->date();
    EXPECT_GE(expected.size(), 0);
}
//TEST_F(TestDMotherBoardDevice, serialNumber)
//{
//    auto expected = device->serialNumber();
//    EXPECT_GE(expected.size(), 0);
//}
TEST_F(TestDMotherBoardDevice, biosInformation)
{
    auto expected = device->biosInformation();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMotherBoardDevice, chassisInformation)
{
    auto expected = device->chassisInformation();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMotherBoardDevice, PhysicalMemoryArrayInformation)
{
    auto expected = device->PhysicalMemoryArrayInformation();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMotherBoardDevice, temperature)
{
    auto expected = device->temperature();
    EXPECT_GE(expected.size(), 0);
}
