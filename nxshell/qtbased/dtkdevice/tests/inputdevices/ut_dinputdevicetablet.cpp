// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicetablet.h"
#include "wacomservice.h"
#include <gtest/gtest.h>

DDEVICE_USE_NAMESPACE

class TestDInputDeviceTablet : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        deviceTablet = new DInputDeviceTablet;
        wacomService = new WacomService;
    }
    static void TearDownTestCase()
    {
        delete deviceTablet;
        delete wacomService;
    }

    static DInputDeviceTablet *deviceTablet;
    static WacomService *wacomService;
};

DInputDeviceTablet *TestDInputDeviceTablet::deviceTablet = nullptr;
WacomService *TestDInputDeviceTablet::wacomService = nullptr;

TEST_F(TestDInputDeviceTablet, cursorMode)
{
    wacomService->m_cursorMode = false;
    ASSERT_EQ(false, wacomService->CursorMode());
    EXPECT_EQ(false, deviceTablet->cursorMode());
    wacomService->m_cursorMode = true;
    ASSERT_EQ(true, wacomService->CursorMode());
    EXPECT_EQ(true, deviceTablet->cursorMode());
}

TEST_F(TestDInputDeviceTablet, setCursorMode)
{
    wacomService->m_cursorMode = false;
    deviceTablet->setCursorMode(true);
    EXPECT_EQ(true, wacomService->m_cursorMode);
}

TEST_F(TestDInputDeviceTablet, leftHanded)
{
    wacomService->m_leftHanded = false;
    ASSERT_EQ(false, wacomService->LeftHanded());
    EXPECT_EQ(false, deviceTablet->leftHanded());
    wacomService->m_leftHanded = true;
    ASSERT_EQ(true, wacomService->LeftHanded());
    EXPECT_EQ(true, deviceTablet->leftHanded());
}

TEST_F(TestDInputDeviceTablet, setLeftHanded)
{
    wacomService->m_leftHanded = false;
    deviceTablet->setLeftHanded(true);
    EXPECT_EQ(true, wacomService->m_leftHanded);
}

TEST_F(TestDInputDeviceTablet, scrollMethod)
{
    EXPECT_EQ(ScrollMethod::ScrollOnButtonDown, deviceTablet->scrollMethod());
}

TEST_F(TestDInputDeviceTablet, setScrollMethod)
{
    deviceTablet->setScrollMethod(ScrollMethod::NoScroll);
    EXPECT_EQ(ScrollMethod::ScrollOnButtonDown, deviceTablet->scrollMethod());
}

TEST_F(TestDInputDeviceTablet, accelerationProfile)
{
    EXPECT_EQ(AccelerationProfile::Na, deviceTablet->accelerationProfile());
}

TEST_F(TestDInputDeviceTablet, setAccelerationProfile)
{
    deviceTablet->setAccelerationProfile(AccelerationProfile::Adaptive);
    EXPECT_EQ(AccelerationProfile::Na, deviceTablet->accelerationProfile());
}

TEST_F(TestDInputDeviceTablet, accelerationSpeed)
{
    EXPECT_EQ(0, deviceTablet->accelerationSpeed());
}

TEST_F(TestDInputDeviceTablet, setAccelerationSpeed)
{
    deviceTablet->setAccelerationSpeed(456);
    EXPECT_EQ(0, deviceTablet->accelerationSpeed());
}

TEST_F(TestDInputDeviceTablet, reset)
{
    wacomService->m_cursorMode = !WacomService::CursorModeDefault;
    wacomService->m_exist = !WacomService::ExistDefault;
    wacomService->m_forceProportions = !WacomService::ForceProportionsDefault;
    wacomService->m_leftHanded = !WacomService::LeftHandedDefault;
    wacomService->m_mouseEnterRemap = !WacomService::MouseEnterRemapDefault;
    wacomService->m_deviceList = "";
    wacomService->m_keyDownAction = "";
    wacomService->m_keyUpAction = "";
    wacomService->m_mapOutput = "";
    wacomService->m_eraserPressureSensitive = WacomService::EraserPressureSensitiveDefault + 10;
    wacomService->m_eraserRawSample = WacomService::EraserRawSampleDefault + 10;
    wacomService->m_eraserThreshold = WacomService::EraserThresholdDefault + 10;
    wacomService->m_stylusPressureSensitive = WacomService::StylusPressureSensitiveDefault + 10;
    wacomService->m_stylusRawSample = WacomService::StylusRawSampleDefault + 10;
    wacomService->m_stylusThreshold = WacomService::StylusThresholdDefault + 10;
    wacomService->m_suppress = WacomService::SuppressDefault + 10;
    auto result = deviceTablet->reset();
    EXPECT_TRUE(result.hasValue());
    EXPECT_EQ(wacomService->m_cursorMode, WacomService::CursorModeDefault);
    EXPECT_EQ(wacomService->m_exist, WacomService::ExistDefault);
    EXPECT_EQ(wacomService->m_forceProportions, WacomService::ForceProportionsDefault);
    EXPECT_EQ(wacomService->m_leftHanded, WacomService::LeftHandedDefault);
    EXPECT_EQ(wacomService->m_mouseEnterRemap, WacomService::MouseEnterRemapDefault);
    EXPECT_EQ(wacomService->m_deviceList, WacomService::DeviceListDefault);
    EXPECT_EQ(wacomService->m_keyDownAction, WacomService::KeyDownActionDefault);
    EXPECT_EQ(wacomService->m_keyUpAction, WacomService::KeyUpActionDefault);
    EXPECT_EQ(wacomService->m_mapOutput, WacomService::MapOutputDefault);
    EXPECT_EQ(wacomService->m_eraserPressureSensitive, WacomService::EraserPressureSensitiveDefault);
    EXPECT_EQ(wacomService->m_eraserRawSample, WacomService::EraserRawSampleDefault);
    EXPECT_EQ(wacomService->m_eraserThreshold, WacomService::EraserThresholdDefault);
    EXPECT_EQ(wacomService->m_stylusPressureSensitive, WacomService::StylusPressureSensitiveDefault);
    EXPECT_EQ(wacomService->m_stylusRawSample, WacomService::StylusRawSampleDefault);
    EXPECT_EQ(wacomService->m_stylusThreshold, WacomService::StylusThresholdDefault);
    EXPECT_EQ(wacomService->m_suppress, WacomService::SuppressDefault);
}
