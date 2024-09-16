// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicemouse.h"
#include "dinputdevicepointer.h"
#include "mouseadaptor.h"
#include "mouseservice.h"
#include <gtest/gtest.h>

DDEVICE_USE_NAMESPACE

class TestDInputDevicePointer : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        devicePointer = new DInputDeviceMouse;
        mouseService = new MouseService;
    }
    static void TearDownTestCase()
    {
        delete devicePointer;
        delete mouseService;
    }

    static DInputDevicePointer *devicePointer;
    static MouseService *mouseService;
};

DInputDevicePointer *TestDInputDevicePointer::devicePointer = nullptr;
MouseService *TestDInputDevicePointer::mouseService = nullptr;

TEST_F(TestDInputDevicePointer, leftHanded)
{
    mouseService->m_mouseAdaptor->m_leftHanded = false;
    ASSERT_EQ(false, mouseService->m_mouseAdaptor->LeftHanded());
    EXPECT_EQ(false, devicePointer->leftHanded());
    mouseService->m_mouseAdaptor->m_leftHanded = true;
    ASSERT_EQ(true, mouseService->m_mouseAdaptor->LeftHanded());
    EXPECT_EQ(true, devicePointer->leftHanded());
}

TEST_F(TestDInputDevicePointer, setLeftHanded)
{
    mouseService->m_mouseAdaptor->m_leftHanded = false;
    devicePointer->setLeftHanded(true);
    EXPECT_EQ(true, mouseService->m_mouseAdaptor->m_leftHanded);
}

TEST_F(TestDInputDevicePointer, scrollMethod)
{
    EXPECT_EQ(ScrollMethod::ScrollOnButtonDown, devicePointer->scrollMethod());
}

TEST_F(TestDInputDevicePointer, setScrollMethod)
{
    devicePointer->setScrollMethod(ScrollMethod::NoScroll);
    EXPECT_EQ(ScrollMethod::ScrollOnButtonDown, devicePointer->scrollMethod());
}

TEST_F(TestDInputDevicePointer, accelerationProfile)
{
    mouseService->m_mouseAdaptor->m_adaptiveAccelProfile = false;
    ASSERT_EQ(false, mouseService->m_mouseAdaptor->AdaptiveAccelProfile());
    EXPECT_EQ(AccelerationProfile::Flat, devicePointer->accelerationProfile());
    mouseService->m_mouseAdaptor->m_adaptiveAccelProfile = true;
    ASSERT_EQ(true, mouseService->m_mouseAdaptor->AdaptiveAccelProfile());
    EXPECT_EQ(AccelerationProfile::Adaptive, devicePointer->accelerationProfile());
}

TEST_F(TestDInputDevicePointer, setAccelerationProfile)
{
    mouseService->m_mouseAdaptor->m_adaptiveAccelProfile = false;
    devicePointer->setAccelerationProfile(AccelerationProfile::Adaptive);
    EXPECT_EQ(true, mouseService->m_mouseAdaptor->m_adaptiveAccelProfile);
}

TEST_F(TestDInputDevicePointer, accelerationSpeed)
{
    mouseService->m_mouseAdaptor->m_motionAcceleration = 123;
    ASSERT_EQ(123, mouseService->m_mouseAdaptor->MotionAcceleration());
    EXPECT_EQ(123, devicePointer->accelerationSpeed());
    mouseService->m_mouseAdaptor->m_motionAcceleration = 456;
    ASSERT_EQ(456, mouseService->m_mouseAdaptor->MotionAcceleration());
    EXPECT_EQ(456, devicePointer->accelerationSpeed());
}

TEST_F(TestDInputDevicePointer, setAccelerationSpeed)
{
    mouseService->m_mouseAdaptor->m_motionAcceleration = 123;
    devicePointer->setAccelerationSpeed(456);
    EXPECT_EQ(456, mouseService->m_mouseAdaptor->m_motionAcceleration);
}

TEST_F(TestDInputDevicePointer, reset)
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
    auto result = devicePointer->reset();
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
