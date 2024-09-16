// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicekeyboard.h"
#include <gtest/gtest.h>
DDEVICE_USE_NAMESPACE

class TestDInputDeviceKeyboard : public testing::Test
{
public:
    static void SetUpTestCase() { deviceKeyboard = new DInputDeviceKeyboard; }
    static void TearDownTestCase() { delete deviceKeyboard; }
    static DInputDeviceKeyboard *deviceKeyboard;
};

DInputDeviceKeyboard *TestDInputDeviceKeyboard::deviceKeyboard = nullptr;

TEST_F(TestDInputDeviceKeyboard, reset)
{
    auto expected = deviceKeyboard->reset();
    ASSERT_TRUE(expected.hasValue());
}
