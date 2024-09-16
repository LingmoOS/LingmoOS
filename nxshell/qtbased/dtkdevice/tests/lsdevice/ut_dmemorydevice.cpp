// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "scan.h"
#include "dmemorydevice.h"
#include <gtest/gtest.h>
#include <stubext.h>


DDEVICE_USE_NAMESPACE


class  TestDMemoryDevice : public  testing::Test
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
    device = new DMemoryDevice; }
    static void TearDownTestCase() { delete device; }
    static DMemoryDevice *device;
};
DMemoryDevice *TestDMemoryDevice::device = nullptr;

TEST_F(TestDMemoryDevice, vendor)
{
    auto expected = device->vendor(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, model)
{
    auto expected = device->model(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, totalWidth)
{
    auto expected = device->totalWidth(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, dataWidth)
{
    auto expected = device->dataWidth(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, type)
{
    auto expected = device->type(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, speed)
{
    auto expected = device->speed(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, serialNumber)
{
    auto expected = device->serialNumber(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, size)
{
    auto expected = device->size(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, swapSize)
{
    auto expected = device->swapSize();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, buffers)
{
    auto expected = device->buffers();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, cached)
{
    auto expected = device->cached();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, active)
{
    auto expected = device->active();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, inactive)
{
    auto expected = device->inactive();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, sharedSize)
{
    auto expected = device->sharedSize();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, swapFree)
{
    auto expected = device->swapFree();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, swapCached)
{
    auto expected = device->swapCached();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, slab)
{
    auto expected = device->slab();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, dirty)
{
    auto expected = device->dirty();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDMemoryDevice, mapped)
{
    auto expected = device->mapped();
    EXPECT_GE(expected.size(), 0);
}

