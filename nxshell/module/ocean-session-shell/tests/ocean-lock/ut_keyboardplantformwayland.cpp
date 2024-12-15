// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "keyboardplantform_wayland.h"

#include <gtest/gtest.h>

class UT_KeyboardPlantformWayland : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    KeyboardPlantformWayland *m_keyboard;
};

void UT_KeyboardPlantformWayland::SetUp()
{
    m_keyboard = new KeyboardPlantformWayland;
}

void UT_KeyboardPlantformWayland::TearDown()
{
    delete m_keyboard;
}

TEST_F(UT_KeyboardPlantformWayland, basic)
{
    m_keyboard->isCapslockOn();
    m_keyboard->isNumlockOn();
    m_keyboard->setNumlockStatus(false);
}
