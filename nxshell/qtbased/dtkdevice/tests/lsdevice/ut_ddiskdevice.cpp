// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "scan.h"
#include "ddiskdevice.h"
#include <gtest/gtest.h>
#include <stubext.h>

DDEVICE_USE_NAMESPACE

class  TestDDiskDevice : public  testing::Test
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
    device = new DDiskDevice; }
    static void TearDownTestCase() { delete device; }
    static DDiskDevice *device;
};
DDiskDevice *TestDDiskDevice::device = nullptr;

TEST_F(TestDDiskDevice, vendor)
{
    auto expected = device->vendor(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDDiskDevice, model)
{
    auto expected = device->model(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDDiskDevice, mediaType)
{
    auto expected = device->mediaType(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDDiskDevice, size)
{
    auto expected = device->size(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDDiskDevice, interface)
{
    auto expected = device->interface(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDDiskDevice, serialNumber)
{
    auto expected = device->serialNumber(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDDiskDevice, deviceFile)
{
    auto expected = device->deviceFile(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDDiskDevice, rotationRate)
{
    auto expected = device->rotationRate(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDDiskDevice, firmwareVersion)
{
    auto expected = device->firmwareVersion(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDDiskDevice, sectorSize)
{
    auto expected = device->sectorSize(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDDiskDevice, temperature)
{
    auto expected = device->temperature(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDDiskDevice, diskIoStat)
{
    auto expected = device->diskIoStat(0);
    EXPECT_GE(expected.readsMerged, 0);
}
