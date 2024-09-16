// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "scan.h"
#include "dcpudevice.h"
#include <gtest/gtest.h>
#include <stubext.h>

DDEVICE_USE_NAMESPACE


class  TestDCpuDevice : public  testing::Test
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
    device = new DCpuDevice; }
    static void TearDownTestCase() { delete device; }
    static DCpuDevice *device;
};
DCpuDevice *TestDCpuDevice::device = nullptr;

TEST_F(TestDCpuDevice, physicalCount)
{
    auto expected = device->physicalCount();
    EXPECT_GE(expected, 0);
}
TEST_F(TestDCpuDevice, coreCount)
{
    auto expected = device->coreCount(0);
    EXPECT_GE(expected, 0);
}
TEST_F(TestDCpuDevice, threadCount)
{
    auto expected = device->threadCount(0, 0);
    EXPECT_GE(expected, 0);
}
TEST_F(TestDCpuDevice, physicalID)
{
    auto expected = device->physicalID(0);
    EXPECT_GE(expected, 0);
}
TEST_F(TestDCpuDevice, coreID)
{
    auto expected = device->coreID(0);
    EXPECT_GE(expected, 0);
}
TEST_F(TestDCpuDevice, architecture)
{
    auto expected = device->architecture(0);
    EXPECT_GE(expected.size(), 0);
}

TEST_F(TestDCpuDevice, vendor)
{
    auto expected = device->vendor(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDCpuDevice, model)
{
    auto expected = device->model(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDCpuDevice, minFreq)
{
    auto expected = device->minFreq(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDCpuDevice, maxFreq)
{
    auto expected = device->maxFreq(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDCpuDevice, flags)
{
    auto expected = device->flags(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDCpuDevice, stepping)
{
    auto expected = device->stepping(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDCpuDevice, family)
{
    auto expected = device->family(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDCpuDevice, bogoMIPSlags)
{
    auto expected = device->bogoMIPS(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDCpuDevice, temperature)
{
    auto expected = device->temperature(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDCpuDevice, currentFreq)
{
    auto expected = device->currentFreq(0);
    EXPECT_GE(expected.size(), 0);
}
