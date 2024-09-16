// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "scan.h"
#include "dinputdevice.h"
#include <gtest/gtest.h>
#include <stubext.h>

DDEVICE_USE_NAMESPACE

class  TestDIputDevice : public  testing::Test
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
    device = new DInputDevice; }
    static void TearDownTestCase() { delete device; }
    static DInputDevice *device;
};
DInputDevice *TestDIputDevice::device = nullptr;

TEST_F(TestDIputDevice, vendor)
{
    auto expected = device->vendor(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDIputDevice, model)
{
    auto expected = device->model(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDIputDevice, interface)
{
    auto expected = device->interface(0);
    EXPECT_GE(expected.size(), 0);
}
TEST_F(TestDIputDevice, driver)
{
    auto expected = device->driver(0);
    EXPECT_GE(expected.size(), 0);
}
