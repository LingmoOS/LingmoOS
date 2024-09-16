// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicemanager.h"
#include "dinputdevicemanager_p.h"
#include "mouseservice.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dinputdevicekeyboard.h"
#include "dinputdevicemanager_p.h"
#include "dinputdevicemouse.h"
#include "dinputdevicetablet.h"
#include "dinputdevicetouchpad.h"
#include "dinputdevicetrackpoint.h"
#include "mouseadaptor.h"
#include "touchpadservice.h"
#include "trackpointadaptor.h"
#include "wacomservice.h"

DDEVICE_USE_NAMESPACE

class TestDInputDeviceManager : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        touchPadService = new TouchPadService;
        wacomService = new WacomService;
        mouseService = new MouseService;
        manager = new DInputDeviceManager;
        d_ptr = dynamic_cast<DInputDeviceManagerPrivate *>(qGetPtrHelper(manager->d_d_ptr));
    }
    static void TearDownTestCase()
    {
        d_ptr = nullptr;
        delete manager;
        delete mouseService;
        delete touchPadService;
        delete wacomService;
    }

    static void Reset()
    {
        mouseService->m_mouseAdaptor->Reset();
        mouseService->m_trackPointAdaptor->Reset();
        touchPadService->Reset();
        wacomService->Reset();
        d_ptr->initializeDeviceInfos();
    }
    static DInputDeviceManager *manager;
    static MouseService *mouseService;
    static TouchPadService *touchPadService;
    static WacomService *wacomService;
    static DInputDeviceManagerPrivate *d_ptr;
};

DInputDeviceManager *TestDInputDeviceManager::manager = nullptr;
MouseService *TestDInputDeviceManager::mouseService = nullptr;
TouchPadService *TestDInputDeviceManager::touchPadService = nullptr;
WacomService *TestDInputDeviceManager::wacomService = nullptr;
DInputDeviceManagerPrivate *TestDInputDeviceManager::d_ptr = nullptr;

TEST_F(TestDInputDeviceManager, deviceInfos)
{
    ASSERT_NE(nullptr, d_ptr);
    ASSERT_THAT(d_ptr->m_deviceInfos, testing::SizeIs(4));
    auto infos = manager->deviceInfos();
    EXPECT_THAT(infos, testing::SizeIs(4));
    const auto &info = infos[0];
    EXPECT_EQ(10, info.id);
    EXPECT_EQ("Test Mouse", info.name);
    EXPECT_EQ(DeviceType::Mouse, info.type);
}

class TestCreateDevice : public TestDInputDeviceManager, public testing::WithParamInterface<DeviceInfo>
{
public:
    static void SetUpTestCase() { TestDInputDeviceManager::SetUpTestCase(); }
    static void TearDownTestCase() { TestDInputDeviceManager::TearDownTestCase(); }
};

TEST_P(TestCreateDevice, createDevice)
{
    Reset();
    auto info = GetParam();
    auto expected = manager->createDevice(info);
    ASSERT_TRUE(expected.hasValue());
    auto device = expected.value().data();
    ASSERT_NE(nullptr, device);
    switch (info.type) {
        case DeviceType::Mouse:
            EXPECT_EQ(typeid(DInputDeviceMouse).name(), typeid(*device).name());
            break;
        case DeviceType::TouchPad:
            EXPECT_EQ(typeid(DInputDeviceTouchPad).name(), typeid(*device).name());
            break;
        case DeviceType::Keyboard:
            EXPECT_EQ(typeid(DInputDeviceKeyboard).name(), typeid(*device).name());
            break;
        case DeviceType::TrackPoint:
            EXPECT_EQ(typeid(DInputDeviceTrackPoint).name(), typeid(*device).name());
            break;
        case DeviceType::Tablet:
            EXPECT_EQ(typeid(DInputDeviceTablet).name(), typeid(*device).name());
            break;
        case DeviceType::Generic:
            EXPECT_EQ(typeid(DInputDeviceGeneric).name(), typeid(*device).name());
            break;
        default:
            Q_UNREACHABLE();
    }
}

INSTANTIATE_TEST_CASE_P(Default,
                        TestCreateDevice,
                        testing::Values(DeviceInfo{10, "mouse", DeviceType::Mouse},
                                        DeviceInfo{10, "keyboard", DeviceType::Keyboard},
                                        DeviceInfo{10, "touchpad", DeviceType::TouchPad},
                                        DeviceInfo{10, "trackpoint", DeviceType::TrackPoint},
                                        DeviceInfo{10, "tablet", DeviceType::Tablet},
                                        DeviceInfo{10, "generic", DeviceType::Generic}));

TEST_F(TestDInputDeviceManager, setting)
{
    auto expected = manager->setting();
    ASSERT_TRUE(expected.hasValue());
    auto setting = expected.value();
    EXPECT_NE(nullptr, setting.data());
}
