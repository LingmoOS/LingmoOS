// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicemouse.h"
#include "mouseadaptor.h"
#include "mouseservice.h"
#include <gtest/gtest.h>

DDEVICE_USE_NAMESPACE

class TestDInputDeviceMouse : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        deviceMouse = new DInputDeviceMouse;
        mouseService = new MouseService;
    }
    static void TearDownTestCase()
    {
        delete deviceMouse;
        delete mouseService;
    }
    static DInputDeviceMouse *deviceMouse;
    static MouseService *mouseService;
};

DInputDeviceMouse *TestDInputDeviceMouse::deviceMouse = nullptr;
MouseService *TestDInputDeviceMouse::mouseService = nullptr;

TEST_F(TestDInputDeviceMouse, naturalScroll)
{
    mouseService->m_mouseAdaptor->m_naturalScroll = false;
    ASSERT_EQ(false, mouseService->m_mouseAdaptor->NaturalScroll());
    EXPECT_EQ(false, deviceMouse->naturalScroll());
    mouseService->m_mouseAdaptor->m_naturalScroll = true;
    ASSERT_EQ(true, mouseService->m_mouseAdaptor->NaturalScroll());
    EXPECT_EQ(true, deviceMouse->naturalScroll());
}

TEST_F(TestDInputDeviceMouse, setNaturalScroll)
{
    mouseService->m_mouseAdaptor->m_naturalScroll = false;
    deviceMouse->setNaturalScroll(true);
    EXPECT_EQ(true, mouseService->m_mouseAdaptor->m_naturalScroll);
}

TEST_F(TestDInputDeviceMouse, middleButtonEmulation)
{
    mouseService->m_mouseAdaptor->m_middleButtonEmulation = false;
    ASSERT_EQ(false, mouseService->m_mouseAdaptor->MiddleButtonEmulation());
    EXPECT_EQ(false, deviceMouse->middleButtonEmulation());
    mouseService->m_mouseAdaptor->m_middleButtonEmulation = true;
    ASSERT_EQ(true, mouseService->m_mouseAdaptor->MiddleButtonEmulation());
    EXPECT_EQ(true, deviceMouse->middleButtonEmulation());
}

TEST_F(TestDInputDeviceMouse, setMiddleButtonEmulation)
{
    mouseService->m_mouseAdaptor->m_middleButtonEmulation = false;
    deviceMouse->setMiddleButtonEmulation(true);
    EXPECT_EQ(true, mouseService->m_mouseAdaptor->m_middleButtonEmulation);
}

TEST_F(TestDInputDeviceMouse, leftHanded)
{
    mouseService->m_mouseAdaptor->m_leftHanded = false;
    ASSERT_EQ(false, mouseService->m_mouseAdaptor->LeftHanded());
    EXPECT_EQ(false, deviceMouse->leftHanded());
    mouseService->m_mouseAdaptor->m_leftHanded = true;
    ASSERT_EQ(true, mouseService->m_mouseAdaptor->LeftHanded());
    EXPECT_EQ(true, deviceMouse->leftHanded());
}

TEST_F(TestDInputDeviceMouse, setLeftHanded)
{
    mouseService->m_mouseAdaptor->m_leftHanded = false;
    deviceMouse->setLeftHanded(true);
    EXPECT_EQ(true, mouseService->m_mouseAdaptor->m_leftHanded);
}

TEST_F(TestDInputDeviceMouse, scrollMethod)
{
    EXPECT_EQ(ScrollMethod::ScrollOnButtonDown, deviceMouse->scrollMethod());
}

TEST_F(TestDInputDeviceMouse, setScrollMethod)
{
    deviceMouse->setScrollMethod(ScrollMethod::NoScroll);
    EXPECT_EQ(ScrollMethod::ScrollOnButtonDown, deviceMouse->scrollMethod());
}

TEST_F(TestDInputDeviceMouse, accelerationProfile)
{
    mouseService->m_mouseAdaptor->m_adaptiveAccelProfile = false;
    ASSERT_EQ(false, mouseService->m_mouseAdaptor->AdaptiveAccelProfile());
    EXPECT_EQ(AccelerationProfile::Flat, deviceMouse->accelerationProfile());
    mouseService->m_mouseAdaptor->m_adaptiveAccelProfile = true;
    ASSERT_EQ(true, mouseService->m_mouseAdaptor->AdaptiveAccelProfile());
    EXPECT_EQ(AccelerationProfile::Adaptive, deviceMouse->accelerationProfile());
}

TEST_F(TestDInputDeviceMouse, setAccelerationProfile)
{
    mouseService->m_mouseAdaptor->m_adaptiveAccelProfile = false;
    deviceMouse->setAccelerationProfile(AccelerationProfile::Adaptive);
    EXPECT_EQ(true, mouseService->m_mouseAdaptor->m_adaptiveAccelProfile);
}

TEST_F(TestDInputDeviceMouse, accelerationSpeed)
{
    mouseService->m_mouseAdaptor->m_motionAcceleration = 123;
    ASSERT_EQ(123, mouseService->m_mouseAdaptor->MotionAcceleration());
    EXPECT_EQ(123, deviceMouse->accelerationSpeed());
    mouseService->m_mouseAdaptor->m_motionAcceleration = 456;
    ASSERT_EQ(456, mouseService->m_mouseAdaptor->MotionAcceleration());
    EXPECT_EQ(456, deviceMouse->accelerationSpeed());
}

TEST_F(TestDInputDeviceMouse, setAccelerationSpeed)
{
    mouseService->m_mouseAdaptor->m_motionAcceleration = 123;
    deviceMouse->setAccelerationSpeed(456);
    EXPECT_EQ(456, mouseService->m_mouseAdaptor->m_motionAcceleration);
}

TEST_F(TestDInputDeviceMouse, reset)
{
    mouseService->m_mouseAdaptor->m_adaptiveAccelProfile = !MouseAdaptor::AdaptiveAccelProfileDefault;
    mouseService->m_mouseAdaptor->m_disableTpad = !MouseAdaptor::DisableTpadDefault;
    mouseService->m_mouseAdaptor->m_exist = !MouseAdaptor::ExistDefault;
    mouseService->m_mouseAdaptor->m_leftHanded = !MouseAdaptor::LeftHandedDefault;
    mouseService->m_mouseAdaptor->m_middleButtonEmulation = !MouseAdaptor::MiddleButtonEmulationDefault;
    mouseService->m_mouseAdaptor->m_naturalScroll = !MouseAdaptor::NaturalScrollDefault;
    mouseService->m_mouseAdaptor->m_motionAcceleration = MouseAdaptor::MotionAccelerationDefault + 2;
    mouseService->m_mouseAdaptor->m_doubleClick = MouseAdaptor::DoubleClickDefault + 100;
    mouseService->m_mouseAdaptor->m_dragThreshold = MouseAdaptor::DragThresholdDefault + 100;
    mouseService->m_mouseAdaptor->m_deviceList = "";
    auto result = deviceMouse->reset();
    EXPECT_TRUE(result.hasValue());
    EXPECT_EQ(mouseService->m_mouseAdaptor->m_adaptiveAccelProfile, MouseAdaptor::AdaptiveAccelProfileDefault);
    EXPECT_EQ(mouseService->m_mouseAdaptor->m_disableTpad, MouseAdaptor::DisableTpadDefault);
    EXPECT_EQ(mouseService->m_mouseAdaptor->m_exist, MouseAdaptor::ExistDefault);
    EXPECT_EQ(mouseService->m_mouseAdaptor->m_leftHanded, MouseAdaptor::LeftHandedDefault);
    EXPECT_EQ(mouseService->m_mouseAdaptor->m_middleButtonEmulation, MouseAdaptor::MiddleButtonEmulationDefault);
    EXPECT_EQ(mouseService->m_mouseAdaptor->m_naturalScroll, MouseAdaptor::NaturalScrollDefault);
    EXPECT_DOUBLE_EQ(mouseService->m_mouseAdaptor->m_motionAcceleration, MouseAdaptor::MotionAccelerationDefault);
    EXPECT_EQ(mouseService->m_mouseAdaptor->m_doubleClick, MouseAdaptor::DoubleClickDefault);
    EXPECT_EQ(mouseService->m_mouseAdaptor->m_dragThreshold, MouseAdaptor::DragThresholdDefault);
    EXPECT_EQ(mouseService->m_mouseAdaptor->m_deviceList, MouseAdaptor::DeviceListDefault);
}
