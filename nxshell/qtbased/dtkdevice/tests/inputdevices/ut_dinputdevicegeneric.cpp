// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicegeneric.h"
#include "dinputdevicegeneric_p.h"
#include <gtest/gtest.h>

DDEVICE_USE_NAMESPACE

class TestDInputDeviceGeneric : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        device = new DInputDeviceGeneric;
        d_ptr = dynamic_cast<DInputDeviceGenericPrivate *>(qGetPtrHelper(device->d_d_ptr));
    }
    static void TearDownTestCase()
    {
        d_ptr = nullptr;
        delete device;
    }
    static DInputDeviceGeneric *device;
    static DInputDeviceGenericPrivate *d_ptr;
};

DInputDeviceGeneric *TestDInputDeviceGeneric::device = nullptr;
DInputDeviceGenericPrivate *TestDInputDeviceGeneric::d_ptr = nullptr;

TEST_F(TestDInputDeviceGeneric, id)
{
    ASSERT_NE(nullptr, d_ptr);
    d_ptr->m_info.id = 10;
    EXPECT_EQ(10, device->id());
}

TEST_F(TestDInputDeviceGeneric, name)
{
    ASSERT_NE(nullptr, d_ptr);
    d_ptr->m_info.name = "Test";
    EXPECT_EQ("Test", device->name());
}

TEST_F(TestDInputDeviceGeneric, type)
{
    ASSERT_NE(nullptr, d_ptr);
    d_ptr->m_info.type = DeviceType::Mouse;
    EXPECT_EQ(DeviceType::Mouse, device->type());
}

TEST_F(TestDInputDeviceGeneric, enabled)
{
    ASSERT_NE(nullptr, d_ptr);
    d_ptr->m_enabled = true;
    EXPECT_EQ(true, device->enabled());
    d_ptr->m_enabled = false;
    EXPECT_EQ(false, device->enabled());
}

TEST_F(TestDInputDeviceGeneric, reset)
{
    auto result = device->reset();
    ASSERT_FALSE(result.hasValue());
    EXPECT_EQ(ErrorCode::InvalidCall, result.error().getErrorCode());
    EXPECT_EQ("Generic device does not have a reset method!", result.error().getErrorMessage());
}

TEST_F(TestDInputDeviceGeneric, setDeviceInfo)
{
    DeviceInfo info{100, "test", DeviceType::Keyboard};
    device->setDeviceInfo(info);
    ASSERT_NE(nullptr, d_ptr);
    EXPECT_EQ(100, d_ptr->m_info.id);
    EXPECT_EQ("test", d_ptr->m_info.name);
    EXPECT_EQ(DeviceType::Keyboard, d_ptr->m_info.type);
}

TEST_F(TestDInputDeviceGeneric, setEnabled)
{
    ASSERT_NE(nullptr, d_ptr);
    d_ptr->m_enabled = false;
    device->setEnabled(true);
    EXPECT_EQ(true, d_ptr->m_enabled);
}

TEST_F(TestDInputDeviceGeneric, setId)
{
    ASSERT_NE(nullptr, d_ptr);
    d_ptr->m_info.id = 0;
    device->setId(12345);
    EXPECT_EQ(12345, d_ptr->m_info.id);
}

TEST_F(TestDInputDeviceGeneric, setName)
{
    ASSERT_NE(nullptr, d_ptr);
    d_ptr->m_info.name = "";
    device->setName("Test");
    EXPECT_EQ("Test", d_ptr->m_info.name);
}

TEST_F(TestDInputDeviceGeneric, setType)
{
    ASSERT_NE(nullptr, d_ptr);
    d_ptr->m_info.type = DeviceType::Generic;
    device->setType(DeviceType::Mouse);
    EXPECT_EQ(DeviceType::Mouse, d_ptr->m_info.type);
}
