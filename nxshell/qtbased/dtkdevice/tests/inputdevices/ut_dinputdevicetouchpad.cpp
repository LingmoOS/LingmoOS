// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicetouchpad.h"
#include "touchpadservice.h"
#include <gtest/gtest.h>

DDEVICE_USE_NAMESPACE

class TestDInputDeviceTouchPad : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        deviceTouchPad = new DInputDeviceTouchPad;
        touchPadService = new TouchPadService;
    }
    static void TearDownTestCase()
    {
        delete deviceTouchPad;
        delete touchPadService;
    }

    static DInputDeviceTouchPad *deviceTouchPad;
    static TouchPadService *touchPadService;
};

DInputDeviceTouchPad *TestDInputDeviceTouchPad::deviceTouchPad = nullptr;
TouchPadService *TestDInputDeviceTouchPad::touchPadService = nullptr;

TEST_F(TestDInputDeviceTouchPad, leftHanded)
{
    touchPadService->m_leftHanded = false;
    ASSERT_EQ(false, touchPadService->LeftHanded());
    EXPECT_EQ(false, deviceTouchPad->leftHanded());
    touchPadService->m_leftHanded = true;
    ASSERT_EQ(true, touchPadService->LeftHanded());
    EXPECT_EQ(true, deviceTouchPad->leftHanded());
}

TEST_F(TestDInputDeviceTouchPad, setLeftHanded)
{
    touchPadService->m_leftHanded = false;
    deviceTouchPad->setLeftHanded(true);
    EXPECT_EQ(true, touchPadService->m_leftHanded);
}

TEST_F(TestDInputDeviceTouchPad, scrollMethod)
{
    touchPadService->m_edgeScroll = false;
    touchPadService->m_vertScroll = true;
    EXPECT_EQ(ScrollMethod::ScrollTwoFinger, deviceTouchPad->scrollMethod());
    touchPadService->m_edgeScroll = true;
    touchPadService->m_vertScroll = false;
    EXPECT_EQ(ScrollMethod::ScrollEdge, deviceTouchPad->scrollMethod());
    touchPadService->m_edgeScroll = false;
    touchPadService->m_vertScroll = false;
    EXPECT_EQ(ScrollMethod::NoScroll, deviceTouchPad->scrollMethod());
}

TEST_F(TestDInputDeviceTouchPad, setScrollMethod)
{
    touchPadService->m_edgeScroll = false;
    touchPadService->m_vertScroll = true;
    deviceTouchPad->setScrollMethod(ScrollMethod::ScrollEdge);
    EXPECT_EQ(true, touchPadService->m_edgeScroll);
    EXPECT_EQ(false, touchPadService->m_vertScroll);
    touchPadService->m_edgeScroll = true;
    touchPadService->m_vertScroll = false;
    deviceTouchPad->setScrollMethod(ScrollMethod::NoScroll);
    EXPECT_EQ(false, touchPadService->m_edgeScroll);
    EXPECT_EQ(false, touchPadService->m_vertScroll);
    touchPadService->m_edgeScroll = false;
    touchPadService->m_vertScroll = false;
    deviceTouchPad->setScrollMethod(ScrollMethod::ScrollTwoFinger);
    EXPECT_EQ(false, touchPadService->m_edgeScroll);
    EXPECT_EQ(true, touchPadService->m_vertScroll);
    deviceTouchPad->setScrollMethod(ScrollMethod::ScrollOnButtonDown);
    EXPECT_EQ(false, touchPadService->m_edgeScroll);
    EXPECT_EQ(true, touchPadService->m_vertScroll);
}

TEST_F(TestDInputDeviceTouchPad, accelerationProfile)
{
    EXPECT_EQ(AccelerationProfile::Na, deviceTouchPad->accelerationProfile());
}

TEST_F(TestDInputDeviceTouchPad, setAccelerationProfile)
{
    deviceTouchPad->setAccelerationProfile(AccelerationProfile::Adaptive);
    EXPECT_EQ(AccelerationProfile::Na, deviceTouchPad->accelerationProfile());
}

TEST_F(TestDInputDeviceTouchPad, accelerationSpeed)
{
    EXPECT_EQ(0, deviceTouchPad->accelerationSpeed());
}

TEST_F(TestDInputDeviceTouchPad, setAccelerationSpeed)
{
    deviceTouchPad->setAccelerationSpeed(123456);
    EXPECT_EQ(0, deviceTouchPad->accelerationSpeed());
}

TEST_F(TestDInputDeviceTouchPad, disableWhileTyping)
{
    touchPadService->m_disableIfTyping = false;
    ASSERT_EQ(false, touchPadService->DisableIfTyping());
    EXPECT_EQ(false, deviceTouchPad->disableWhileTyping());
    touchPadService->m_disableIfTyping = true;
    ASSERT_EQ(true, touchPadService->DisableIfTyping());
    EXPECT_EQ(true, deviceTouchPad->disableWhileTyping());
}

TEST_F(TestDInputDeviceTouchPad, setDisableWhileTyping)
{
    touchPadService->m_disableIfTyping = false;
    deviceTouchPad->setDisableWhileTyping(true);
    EXPECT_EQ(true, touchPadService->m_disableIfTyping);
}

TEST_F(TestDInputDeviceTouchPad, naturalScroll)
{
    touchPadService->m_naturalScroll = false;
    ASSERT_EQ(false, touchPadService->NaturalScroll());
    EXPECT_EQ(false, deviceTouchPad->naturalScroll());
    touchPadService->m_naturalScroll = true;
    ASSERT_EQ(true, touchPadService->NaturalScroll());
    EXPECT_EQ(true, deviceTouchPad->naturalScroll());
}

TEST_F(TestDInputDeviceTouchPad, setNaturalScroll)
{
    touchPadService->m_naturalScroll = false;
    deviceTouchPad->setNaturalScroll(true);
    EXPECT_EQ(true, touchPadService->m_naturalScroll);
}

TEST_F(TestDInputDeviceTouchPad, tapToClick)
{
    touchPadService->m_tapClick = false;
    ASSERT_EQ(false, touchPadService->TapClick());
    EXPECT_EQ(false, deviceTouchPad->tapToClick());
    touchPadService->m_tapClick = true;
    ASSERT_EQ(true, touchPadService->TapClick());
    EXPECT_EQ(true, deviceTouchPad->tapToClick());
}

TEST_F(TestDInputDeviceTouchPad, setTapToClick)
{
    touchPadService->m_tapClick = false;
    deviceTouchPad->setTapToClick(true);
    EXPECT_EQ(true, touchPadService->m_tapClick);
}

TEST_F(TestDInputDeviceTouchPad, reset)
{
    touchPadService->m_disableIfTyping = !TouchPadService::DisableIfTypingDefault;
    touchPadService->m_edgeScroll = !TouchPadService::EdgeScrollDefault;
    touchPadService->m_exist = !TouchPadService::ExistDefault;
    touchPadService->m_leftHanded = !TouchPadService::LeftHandedDefault;
    touchPadService->m_naturalScroll = !TouchPadService::NaturalScrollDefault;
    touchPadService->m_tPadEnable = !TouchPadService::TPadEnableDefault;
    touchPadService->m_tapClick = !TouchPadService::TapClickDefault;
    touchPadService->m_vertScroll = !TouchPadService::VertScrollDefault;
    touchPadService->m_motionAcceleration = TouchPadService::MotionAccelerationDefault + 1.0;
    touchPadService->m_deltaScroll = TouchPadService::DeltaScrollDefault + 100;
    touchPadService->m_doubleClick = TouchPadService::DoubleClickDefault + 100;
    touchPadService->m_dragThreshold = TouchPadService::DragThresholdDefault + 100;
    touchPadService->m_deviceList = "";
    auto result = deviceTouchPad->reset();
    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(touchPadService->m_disableIfTyping, TouchPadService::DisableIfTypingDefault);
    EXPECT_EQ(touchPadService->m_edgeScroll, TouchPadService::EdgeScrollDefault);
    EXPECT_EQ(touchPadService->m_exist, TouchPadService::ExistDefault);
    EXPECT_EQ(touchPadService->m_leftHanded, TouchPadService::LeftHandedDefault);
    EXPECT_EQ(touchPadService->m_naturalScroll, TouchPadService::NaturalScrollDefault);
    EXPECT_EQ(touchPadService->m_tPadEnable, TouchPadService::TPadEnableDefault);
    EXPECT_EQ(touchPadService->m_tapClick, TouchPadService::TapClickDefault);
    EXPECT_EQ(touchPadService->m_vertScroll, TouchPadService::VertScrollDefault);
    EXPECT_EQ(touchPadService->m_motionAcceleration, TouchPadService::MotionAccelerationDefault);
    EXPECT_EQ(touchPadService->m_deltaScroll, TouchPadService::DeltaScrollDefault);
    EXPECT_EQ(touchPadService->m_doubleClick, TouchPadService::DoubleClickDefault);
    EXPECT_EQ(touchPadService->m_dragThreshold, TouchPadService::DragThresholdDefault);
    EXPECT_EQ(touchPadService->m_deviceList, TouchPadService::DeviceListDefault);
}

TEST_F(TestDInputDeviceTouchPad, enable)
{
    touchPadService->m_tPadEnable = false;
    deviceTouchPad->enable();
    EXPECT_EQ(true, touchPadService->m_tPadEnable);
}
