// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicetrackpoint.h"
#include "mouseservice.h"
#include "trackpointadaptor.h"
#include <gtest/gtest.h>
DDEVICE_USE_NAMESPACE

class TestDInputDeviceTrackPoint : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        deviceTrackPoint = new DInputDeviceTrackPoint;
        mouseService = new MouseService;
    }
    static void TearDownTestCase()
    {
        delete deviceTrackPoint;
        delete mouseService;
    }
    static DInputDeviceTrackPoint *deviceTrackPoint;
    static MouseService *mouseService;
};

DInputDeviceTrackPoint *TestDInputDeviceTrackPoint::deviceTrackPoint = nullptr;
MouseService *TestDInputDeviceTrackPoint::mouseService = nullptr;

TEST_F(TestDInputDeviceTrackPoint, setMiddleButtonEnabled)
{
    mouseService->m_trackPointAdaptor->m_middleButtonEnabled = false;
    deviceTrackPoint->setMiddleButtonEnabled(true);
    EXPECT_EQ(true, mouseService->m_trackPointAdaptor->m_middleButtonEnabled);
}

TEST_F(TestDInputDeviceTrackPoint, middleButtonEnabled)
{
    mouseService->m_trackPointAdaptor->m_middleButtonEnabled = false;
    ASSERT_EQ(false, mouseService->m_trackPointAdaptor->MiddleButtonEnabled());
    EXPECT_EQ(false, deviceTrackPoint->middleButtonEnabled());
    mouseService->m_trackPointAdaptor->m_middleButtonEnabled = true;
    ASSERT_EQ(true, mouseService->m_trackPointAdaptor->MiddleButtonEnabled());
    EXPECT_EQ(true, deviceTrackPoint->middleButtonEnabled());
}

TEST_F(TestDInputDeviceTrackPoint, wheelEmulation)
{
    mouseService->m_trackPointAdaptor->m_wheelEmulation = false;
    ASSERT_EQ(false, mouseService->m_trackPointAdaptor->WheelEmulation());
    EXPECT_EQ(false, deviceTrackPoint->wheelEmulation());
    mouseService->m_trackPointAdaptor->m_wheelEmulation = true;
    ASSERT_EQ(true, mouseService->m_trackPointAdaptor->WheelEmulation());
    EXPECT_EQ(true, deviceTrackPoint->wheelEmulation());
}

TEST_F(TestDInputDeviceTrackPoint, setWheelEmulation)
{
    mouseService->m_trackPointAdaptor->m_wheelEmulation = false;
    deviceTrackPoint->setWheelEmulation(true);
    EXPECT_EQ(true, mouseService->m_trackPointAdaptor->m_wheelEmulation);
}

TEST_F(TestDInputDeviceTrackPoint, wheelHorizontalScroll)
{
    mouseService->m_trackPointAdaptor->m_wheelHorizScroll = false;
    ASSERT_EQ(false, mouseService->m_trackPointAdaptor->WheelHorizScroll());
    EXPECT_EQ(false, deviceTrackPoint->wheelHorizontalScroll());
    mouseService->m_trackPointAdaptor->m_wheelHorizScroll = true;
    ASSERT_EQ(true, mouseService->m_trackPointAdaptor->WheelHorizScroll());
    EXPECT_EQ(true, deviceTrackPoint->wheelHorizontalScroll());
}

TEST_F(TestDInputDeviceTrackPoint, setWheelHorizScroll)
{
    mouseService->m_trackPointAdaptor->m_wheelHorizScroll = false;
    deviceTrackPoint->setWheelHorizontalScroll(true);
    EXPECT_EQ(true, mouseService->m_trackPointAdaptor->m_wheelHorizScroll);
}

TEST_F(TestDInputDeviceTrackPoint, middleButtonTimeout)
{
    auto adaptor = mouseService->m_trackPointAdaptor;
    adaptor->m_middleButtonTimeout = 0;
    ASSERT_EQ(0, adaptor->MiddleButtonTimeout());
    EXPECT_EQ(0, deviceTrackPoint->middleButtonTimeout());
    adaptor->m_middleButtonTimeout = 100;
    ASSERT_EQ(100, adaptor->MiddleButtonTimeout());
    EXPECT_EQ(100, deviceTrackPoint->middleButtonTimeout());
}

TEST_F(TestDInputDeviceTrackPoint, setMiddleButtonTimeout)
{
    auto adaptor = mouseService->m_trackPointAdaptor;
    adaptor->m_middleButtonTimeout = 0;
    deviceTrackPoint->setMiddleButtonTimeout(100);
    EXPECT_EQ(100, adaptor->m_middleButtonTimeout);
}

TEST_F(TestDInputDeviceTrackPoint, wheelEmulationButton)
{
    auto adaptor = mouseService->m_trackPointAdaptor;
    adaptor->m_wheelEmulationButton = 2;
    ASSERT_EQ(2, adaptor->WheelEmulationButton());
    EXPECT_EQ(2, deviceTrackPoint->wheelEmulationButton());
    adaptor->m_wheelEmulationButton = 4;
    ASSERT_EQ(4, adaptor->WheelEmulationButton());
    EXPECT_EQ(4, deviceTrackPoint->wheelEmulationButton());
}

TEST_F(TestDInputDeviceTrackPoint, setWheelEmulationButton)
{
    auto adaptor = mouseService->m_trackPointAdaptor;
    adaptor->m_wheelEmulationButton = 0;
    deviceTrackPoint->setWheelEmulationButton(3);
    EXPECT_EQ(3, adaptor->m_wheelEmulationButton);
}

TEST_F(TestDInputDeviceTrackPoint, wheelEmulationTimeout)
{
    auto adaptor = mouseService->m_trackPointAdaptor;
    adaptor->m_wheelEmulationTimeout = 100;
    ASSERT_EQ(100, adaptor->WheelEmulationTimeout());
    EXPECT_EQ(100, deviceTrackPoint->wheelEmulationTimeout());
    adaptor->m_wheelEmulationTimeout = 300;
    ASSERT_EQ(300, adaptor->WheelEmulationTimeout());
    EXPECT_EQ(300, deviceTrackPoint->wheelEmulationTimeout());
}

TEST_F(TestDInputDeviceTrackPoint, setWheelEmulationTimeout)
{
    auto adaptor = mouseService->m_trackPointAdaptor;
    adaptor->m_wheelEmulationTimeout = 0;
    deviceTrackPoint->setWheelEmulationTimeout(500);
    EXPECT_EQ(500, adaptor->m_wheelEmulationTimeout);
}

TEST_F(TestDInputDeviceTrackPoint, leftHanded)
{
    mouseService->m_trackPointAdaptor->m_leftHanded = false;
    ASSERT_EQ(false, mouseService->m_trackPointAdaptor->LeftHanded());
    EXPECT_EQ(false, deviceTrackPoint->leftHanded());
    mouseService->m_trackPointAdaptor->m_leftHanded = true;
    ASSERT_EQ(true, mouseService->m_trackPointAdaptor->LeftHanded());
    EXPECT_EQ(true, deviceTrackPoint->leftHanded());
}

TEST_F(TestDInputDeviceTrackPoint, setLeftHanded)
{
    mouseService->m_trackPointAdaptor->m_leftHanded = false;
    deviceTrackPoint->setLeftHanded(true);
    EXPECT_EQ(true, mouseService->m_trackPointAdaptor->m_leftHanded);
}

TEST_F(TestDInputDeviceTrackPoint, scrollMethod)
{
    EXPECT_EQ(ScrollMethod::ScrollOnButtonDown, deviceTrackPoint->scrollMethod());
}

TEST_F(TestDInputDeviceTrackPoint, setScrollMethod)
{
    deviceTrackPoint->setScrollMethod(ScrollMethod::NoScroll);
    EXPECT_EQ(ScrollMethod::ScrollOnButtonDown, deviceTrackPoint->scrollMethod());
}

TEST_F(TestDInputDeviceTrackPoint, accelerationProfile)
{
    EXPECT_EQ(AccelerationProfile::Adaptive, deviceTrackPoint->accelerationProfile());
}

TEST_F(TestDInputDeviceTrackPoint, setAccelerationProfile)
{
    deviceTrackPoint->setAccelerationProfile(AccelerationProfile::Flat);
    EXPECT_EQ(AccelerationProfile::Adaptive, deviceTrackPoint->accelerationProfile());
}

TEST_F(TestDInputDeviceTrackPoint, accelerationSpeed)
{
    mouseService->m_trackPointAdaptor->m_motionAcceleration = 123;
    ASSERT_EQ(123, mouseService->m_trackPointAdaptor->MotionAcceleration());
    EXPECT_EQ(123, deviceTrackPoint->accelerationSpeed());
    mouseService->m_trackPointAdaptor->m_motionAcceleration = 456;
    ASSERT_EQ(456, mouseService->m_trackPointAdaptor->MotionAcceleration());
    EXPECT_EQ(456, deviceTrackPoint->accelerationSpeed());
}

TEST_F(TestDInputDeviceTrackPoint, setAccelerationSpeed)
{
    mouseService->m_trackPointAdaptor->m_motionAcceleration = 123;
    deviceTrackPoint->setAccelerationSpeed(456);
    EXPECT_EQ(456, mouseService->m_trackPointAdaptor->m_motionAcceleration);
}

TEST_F(TestDInputDeviceTrackPoint, reset)
{
    auto adaptor = mouseService->m_trackPointAdaptor;
    adaptor->m_leftHanded = !TrackPointAdaptor::LeftHandedDefault;
    adaptor->m_middleButtonEnabled = !TrackPointAdaptor::MiddleButtonEnabledDefault;
    adaptor->m_wheelEmulation = !TrackPointAdaptor::WheelEmulationDefault;
    adaptor->m_wheelHorizScroll = !TrackPointAdaptor::WheelHorizScrollDefault;
    adaptor->m_motionAcceleration = TrackPointAdaptor::MotionAccelerationDefault + 100;
    adaptor->m_middleButtonTimeout = TrackPointAdaptor::MiddleButtonTimeoutDefault + 100;
    adaptor->m_wheelEmulationButton = TrackPointAdaptor::WheelEmulationButtonDefault + 5;
    adaptor->m_wheelEmulationTimeout = TrackPointAdaptor::WheelEmulationTimeoutDefault + 100;
    adaptor->m_deviceList = "";
    auto result = deviceTrackPoint->reset();
    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(adaptor->m_exist, TrackPointAdaptor::ExistDefault);
    EXPECT_EQ(adaptor->m_leftHanded, TrackPointAdaptor::LeftHandedDefault);
    EXPECT_EQ(adaptor->m_middleButtonEnabled, TrackPointAdaptor::MiddleButtonEnabledDefault);
    EXPECT_EQ(adaptor->m_wheelEmulation, TrackPointAdaptor::WheelEmulationDefault);
    EXPECT_EQ(adaptor->m_wheelHorizScroll, TrackPointAdaptor::WheelHorizScrollDefault);
    EXPECT_EQ(adaptor->m_motionAcceleration, TrackPointAdaptor::MotionAccelerationDefault);
    EXPECT_EQ(adaptor->m_middleButtonTimeout, TrackPointAdaptor::MiddleButtonTimeoutDefault);
    EXPECT_EQ(adaptor->m_wheelEmulationButton, TrackPointAdaptor::WheelEmulationButtonDefault);
    EXPECT_EQ(adaptor->m_wheelEmulationTimeout, TrackPointAdaptor::WheelEmulationTimeoutDefault);
    EXPECT_EQ(adaptor->m_deviceList, TrackPointAdaptor::DeviceListDefault);
}
