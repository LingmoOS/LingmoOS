// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicesetting.h"

#include <gtest/gtest.h>

#include "dtkinputdevices_converters.h"
#include "inputdevicesservice.h"
#include "keyboardservice.h"
#include "mouseadaptor.h"
#include "mouseservice.h"
#include "wacomservice.h"

DDEVICE_USE_NAMESPACE

class TestDInputDeviceSetting : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        setting = new DInputDeviceSetting;
        wacomService = new WacomService;
        mouseService = new MouseService;
        keyboardService = new KeyboardService;
        inputDevicesService = new InputDevicesService;
    }
    static void TearDownTestCase()
    {
        delete setting;
        delete wacomService;
        delete mouseService;
        delete keyboardService;
        delete inputDevicesService;
    }

    static DInputDeviceSetting *setting;
    static WacomService *wacomService;
    static MouseService *mouseService;
    static KeyboardService *keyboardService;
    static InputDevicesService *inputDevicesService;
};

DInputDeviceSetting *TestDInputDeviceSetting::setting = nullptr;
WacomService *TestDInputDeviceSetting::wacomService = nullptr;
MouseService *TestDInputDeviceSetting::mouseService = nullptr;
KeyboardService *TestDInputDeviceSetting::keyboardService = nullptr;
InputDevicesService *TestDInputDeviceSetting::inputDevicesService = nullptr;

TEST_F(TestDInputDeviceSetting, disableTouchPadWhileMouse)
{
    auto adaptor = mouseService->m_mouseAdaptor;
    adaptor->m_disableTpad = false;
    ASSERT_EQ(false, adaptor->DisableTpad());
    EXPECT_EQ(false, setting->disableTouchPadWhileMouse());
    adaptor->m_disableTpad = true;
    ASSERT_EQ(true, adaptor->DisableTpad());
    EXPECT_EQ(true, setting->disableTouchPadWhileMouse());
}

TEST_F(TestDInputDeviceSetting, setDisableTouchPadWhileMouse)
{
    auto adaptor = mouseService->m_mouseAdaptor;
    adaptor->m_disableTpad = false;
    setting->setDisableTouchPadWhileMouse(true);
    EXPECT_EQ(true, adaptor->m_disableTpad);
}

TEST_F(TestDInputDeviceSetting, doubleClickInterval)
{
    auto adaptor = mouseService->m_mouseAdaptor;
    adaptor->m_doubleClick = 100;
    ASSERT_EQ(100, adaptor->DoubleClick());
    EXPECT_EQ(100, setting->doubleClickInterval());
    adaptor->m_doubleClick = 300;
    ASSERT_EQ(300, adaptor->DoubleClick());
    EXPECT_EQ(300, setting->doubleClickInterval());
}

TEST_F(TestDInputDeviceSetting, setDoubleClickInterval)
{
    auto adaptor = mouseService->m_mouseAdaptor;
    adaptor->m_doubleClick = 0;
    setting->setDoubleClickInterval(100);
    EXPECT_EQ(100, adaptor->m_doubleClick);
}

TEST_F(TestDInputDeviceSetting, dragThreshold)
{
    auto adaptor = mouseService->m_mouseAdaptor;
    adaptor->m_dragThreshold = 10;
    ASSERT_EQ(10, adaptor->DragThreshold());
    EXPECT_EQ(10, setting->dragThreshold());
    adaptor->m_dragThreshold = 20;
    ASSERT_EQ(20, adaptor->DragThreshold());
    EXPECT_EQ(20, setting->dragThreshold());
}

TEST_F(TestDInputDeviceSetting, setDragThreshold)
{
    auto adaptor = mouseService->m_mouseAdaptor;
    adaptor->m_dragThreshold = 0;
    setting->setDragThreshold(100);
    EXPECT_EQ(100, adaptor->m_dragThreshold);
}

TEST_F(TestDInputDeviceSetting, repeatEnabled)
{
    keyboardService->m_repeatEnabled = false;
    ASSERT_EQ(false, keyboardService->RepeatEnabled());
    EXPECT_EQ(false, setting->repeatEnabled());
    keyboardService->m_repeatEnabled = true;
    ASSERT_EQ(true, keyboardService->RepeatEnabled());
    EXPECT_EQ(true, setting->repeatEnabled());
}

TEST_F(TestDInputDeviceSetting, setRepeatEnabled)
{
    keyboardService->m_repeatEnabled = false;
    setting->setRepeatEnabled(true);
    EXPECT_EQ(true, keyboardService->m_repeatEnabled);
}

TEST_F(TestDInputDeviceSetting, repeatDelay)
{
    keyboardService->m_repeatDelay = 100;
    ASSERT_EQ(100, keyboardService->RepeatDelay());
    EXPECT_EQ(100, setting->repeatDelay());
    keyboardService->m_repeatDelay = 500;
    ASSERT_EQ(500, keyboardService->RepeatDelay());
    EXPECT_EQ(500, setting->repeatDelay());
}

TEST_F(TestDInputDeviceSetting, setRepeatDelay)
{
    keyboardService->m_repeatDelay = 0;
    setting->setRepeatDelay(100);
    EXPECT_EQ(100, keyboardService->m_repeatDelay);
}

TEST_F(TestDInputDeviceSetting, repeatInterval)
{
    keyboardService->m_repeatInterval = 100;
    ASSERT_EQ(100, keyboardService->RepeatInterval());
    EXPECT_EQ(100, setting->repeatInterval());
    keyboardService->m_repeatInterval = 300;
    ASSERT_EQ(300, keyboardService->RepeatInterval());
    EXPECT_EQ(300, setting->repeatInterval());
}

TEST_F(TestDInputDeviceSetting, setRepeatInterval)
{
    keyboardService->m_repeatInterval = 0;
    setting->setRepeatInterval(100);
    EXPECT_EQ(100, keyboardService->m_repeatInterval);
}

TEST_F(TestDInputDeviceSetting, cursorBlinkInterval)
{
    keyboardService->m_cursorBlink = 100;
    ASSERT_EQ(100, keyboardService->CursorBlink());
    EXPECT_EQ(100, setting->cursorBlinkInterval());
    keyboardService->m_cursorBlink = 300;
    ASSERT_EQ(300, keyboardService->CursorBlink());
    EXPECT_EQ(300, setting->cursorBlinkInterval());
}

TEST_F(TestDInputDeviceSetting, setCursorBlinkInterval)
{
    keyboardService->m_cursorBlink = 0;
    setting->setCursorBlinkInterval(100);
    EXPECT_EQ(100, keyboardService->m_cursorBlink);
}

TEST_F(TestDInputDeviceSetting, wheelSpeed)
{
    inputDevicesService->m_wheelSpeed = 10;
    ASSERT_EQ(10, inputDevicesService->WheelSpeed());
    EXPECT_EQ(10, setting->wheelSpeed());
    inputDevicesService->m_wheelSpeed = 20;
    ASSERT_EQ(20, inputDevicesService->WheelSpeed());
    EXPECT_EQ(20, setting->wheelSpeed());
}

TEST_F(TestDInputDeviceSetting, setWheelSpeed)
{
    inputDevicesService->m_wheelSpeed = 0;
    setting->setWheelSpeed(20);
    EXPECT_EQ(20, inputDevicesService->m_wheelSpeed);
}

TEST_F(TestDInputDeviceSetting, pressureSensitivity)
{
    wacomService->m_eraserPressureSensitive = 10;
    ASSERT_EQ(10, wacomService->EraserPressureSensitive());
    EXPECT_EQ(10, setting->pressureSensitivity(10));
    wacomService->m_eraserPressureSensitive = 20;
    ASSERT_EQ(20, wacomService->EraserPressureSensitive());
    EXPECT_EQ(20, setting->pressureSensitivity(10));
}

TEST_F(TestDInputDeviceSetting, rawSampleSize)
{
    wacomService->m_eraserRawSample = 10;
    ASSERT_EQ(10, wacomService->EraserRawSample());
    EXPECT_EQ(10, setting->rawSampleSize(10));
    wacomService->m_eraserRawSample = 20;
    ASSERT_EQ(20, wacomService->EraserRawSample());
    EXPECT_EQ(20, setting->rawSampleSize(10));
}

TEST_F(TestDInputDeviceSetting, pressureThreshold)
{
    wacomService->m_eraserThreshold = 10;
    ASSERT_EQ(10, wacomService->EraserThreshold());
    EXPECT_EQ(10, setting->pressureThreshold(10));
    wacomService->m_eraserThreshold = 20;
    ASSERT_EQ(20, wacomService->EraserThreshold());
    EXPECT_EQ(20, setting->pressureThreshold(10));
}

TEST_F(TestDInputDeviceSetting, forceProportions)
{
    wacomService->m_forceProportions = false;
    ASSERT_EQ(false, wacomService->ForceProportions());
    EXPECT_EQ(false, setting->forceProportions(10));
    wacomService->m_forceProportions = true;
    ASSERT_EQ(true, wacomService->ForceProportions());
    EXPECT_EQ(true, setting->forceProportions(10));
}

TEST_F(TestDInputDeviceSetting, mapOutput)
{
    wacomService->m_mapOutput = "";
    ASSERT_EQ("", wacomService->MapOutput());
    EXPECT_EQ("", setting->mapOutput(10));
    wacomService->m_mapOutput = "test";
    ASSERT_EQ("test", wacomService->MapOutput());
    EXPECT_EQ("test", setting->mapOutput(10));
}

TEST_F(TestDInputDeviceSetting, mouseEnterRemap)
{
    wacomService->m_mouseEnterRemap = false;
    ASSERT_EQ(false, wacomService->MouseEnterRemap());
    EXPECT_EQ(false, setting->mouseEnterRemap(10));
    wacomService->m_mouseEnterRemap = true;
    ASSERT_EQ(true, wacomService->MouseEnterRemap());
    EXPECT_EQ(true, setting->mouseEnterRemap(10));
}

TEST_F(TestDInputDeviceSetting, suppress)
{
    wacomService->m_suppress = 10;
    ASSERT_EQ(10, wacomService->Suppress());
    EXPECT_EQ(10, setting->suppress(10));
    wacomService->m_suppress = 20;
    ASSERT_EQ(20, wacomService->Suppress());
    EXPECT_EQ(20, setting->suppress(10));
}

TEST_F(TestDInputDeviceSetting, setPressureSensitivity)
{
    wacomService->m_eraserPressureSensitive = 0;
    setting->setPressureSensitivity(10, 100);
    EXPECT_EQ(100, wacomService->m_eraserPressureSensitive);
}

TEST_F(TestDInputDeviceSetting, setRawSampleSize)
{
    wacomService->m_eraserRawSample = 0;
    setting->setRawSampleSize(10, 100);
    EXPECT_EQ(100, wacomService->m_eraserRawSample);
}

TEST_F(TestDInputDeviceSetting, setPressureThreshold)
{
    wacomService->m_eraserThreshold = 0;
    setting->setPressureThreshold(10, 100);
    EXPECT_EQ(100, wacomService->m_eraserThreshold);
}

TEST_F(TestDInputDeviceSetting, setForceProportions)
{
    wacomService->m_forceProportions = false;
    setting->setForceProportions(10, true);
    EXPECT_EQ(true, wacomService->m_forceProportions);
}

TEST_F(TestDInputDeviceSetting, setMouseEnterRemap)
{
    wacomService->m_mouseEnterRemap = false;
    setting->setMouseEnterRemap(10, true);
    EXPECT_EQ(true, wacomService->m_mouseEnterRemap);
}

TEST_F(TestDInputDeviceSetting, setSuppress)
{
    wacomService->m_suppress = 0;
    setting->setSuppress(10, 100);
    EXPECT_EQ(100, wacomService->m_suppress);
}

struct KeyParam
{
    Key key;
    KeyAction action;
};

class TestKey : public TestDInputDeviceSetting, public testing::WithParamInterface<KeyParam>
{
public:
    static void SetUpTestCase() { TestDInputDeviceSetting::SetUpTestCase(); }
    static void TearDownTestCase() { TestDInputDeviceSetting::TearDownTestCase(); }
};

TEST_P(TestKey, keymap)
{
    auto param = GetParam();
    if (param.key == Key::KeyUp) {
        wacomService->m_keyUpAction = keyActionToString(param.action);
    } else {
        wacomService->m_keyDownAction = keyActionToString(param.action);
    }
    EXPECT_EQ(param.action, setting->keymap(10, param.key));
}

TEST_P(TestKey, setKeymap)
{
    auto param = GetParam();
    setting->setKeymap(10, param.key, param.action);
    if (param.key == Key::KeyUp) {
        EXPECT_EQ(keyActionToString(param.action), wacomService->m_keyUpAction);
    } else {
        EXPECT_EQ(keyActionToString(param.action), wacomService->m_keyDownAction);
    }
}

INSTANTIATE_TEST_CASE_P(Default,
                        TestKey,
                        testing::Values(KeyParam{Key::KeyUp, KeyAction::LeftClick},
                                        KeyParam{Key::KeyUp, KeyAction::RightClick},
                                        KeyParam{Key::KeyUp, KeyAction::MiddleClick},
                                        KeyParam{Key::KeyUp, KeyAction::PageUp},
                                        KeyParam{Key::KeyUp, KeyAction::PageDown},
                                        KeyParam{Key::KeyDown, KeyAction::LeftClick},
                                        KeyParam{Key::KeyDown, KeyAction::RightClick},
                                        KeyParam{Key::KeyDown, KeyAction::MiddleClick},
                                        KeyParam{Key::KeyDown, KeyAction::PageUp},
                                        KeyParam{Key::KeyDown, KeyAction::PageDown}));
