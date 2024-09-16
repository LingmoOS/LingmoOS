// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "scan.h"
#include "dlsdevice_p.h"
#include "dlsdevice.h"
#include <gtest/gtest.h>
#include <stubext.h>

DDEVICE_USE_NAMESPACE


class  TestDLsDevice : public  testing::Test
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
    device = new DlsDevice; }
    static void TearDownTestCase() { delete device; }
    static DlsDevice *device;
};
DlsDevice *TestDLsDevice::device = nullptr;

DlsDevice::DevClass devClass = DlsDevice::DtkCpu;


TEST_F(TestDLsDevice, devicesInfosAll)
{
    auto expected = device->devicesInfosAll();
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDLsDevice, deviceAttris)
{
    auto expected = device->deviceAttris(devClass);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDLsDevice, deviceInfo)
{
    auto expected = device->deviceInfo(devClass);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDLsDevice, devicesCount)
{
    auto expected = device->devicesCount();
    EXPECT_GE(expected, 0);
}
