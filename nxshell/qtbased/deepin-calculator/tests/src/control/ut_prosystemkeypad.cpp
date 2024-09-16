// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_prosystemkeypad.h"
#include "../../src/control/prosystemkeypad.h"
#include "../3rdparty/core/settings.h"

Ut_ProSystemKeypad::Ut_ProSystemKeypad()
{

}

TEST_F(Ut_ProSystemKeypad, buttonHasFocus)
{
    ProSystemKeypad *m_prosystemkeypad = new ProSystemKeypad;
    EXPECT_FALSE(m_prosystemkeypad->buttonHasFocus());
    delete m_prosystemkeypad;
}

TEST_F(Ut_ProSystemKeypad, getFocus)
{
    ProSystemKeypad *m_prosystemkeypad = new ProSystemKeypad;
    m_prosystemkeypad->getFocus(0, 1);
    m_prosystemkeypad->getFocus(1, 1);
    m_prosystemkeypad->getFocus(2, 1);
    m_prosystemkeypad->getFocus(3, 1);
    //焦点函数，无assert
    delete m_prosystemkeypad;
}

TEST_F(Ut_ProSystemKeypad, setSystem)
{
    ProSystemKeypad *m_prosystemkeypad = new ProSystemKeypad;
    m_prosystemkeypad->setSystem(32, 64);
    EXPECT_FALSE(m_prosystemkeypad->m_buttons.value(33)->isEnabled());
    m_prosystemkeypad->m_binaryValue = "0000000000000000000000000000000000000000000000000000000011111111";
    m_prosystemkeypad->setSystem(64, 8);
    EXPECT_TRUE(m_prosystemkeypad->m_buttons.value(60)->isEnabled());
    m_prosystemkeypad->setSystem(8, 16);
    EXPECT_FALSE(m_prosystemkeypad->m_buttons.value(11)->isEnabled());
    m_prosystemkeypad->setSystem(16, 32);
    EXPECT_FALSE(m_prosystemkeypad->m_buttons.value(20)->isEnabled());
    m_prosystemkeypad->m_binaryValue = "0000000000000000000000000000000011111111111111111111111111111111";
    m_prosystemkeypad->setSystem(64, 32);
    EXPECT_EQ(m_prosystemkeypad->m_buttons.value(60)->text(), "1");
    m_prosystemkeypad->m_binaryValue = "0000000000000000000000000000000000000000000000001111111111111111";
    m_prosystemkeypad->setSystem(32, 16);
    EXPECT_EQ(m_prosystemkeypad->m_buttons.value(20)->text(), "1");
    m_prosystemkeypad->m_binaryValue = "0000000000000000000000000000000000000000000000000000011011111111";
    m_prosystemkeypad->setSystem(16, 8);
    EXPECT_EQ(m_prosystemkeypad->m_buttons.value(9)->text(), "1");
    m_prosystemkeypad->setSystem(8, 64);
    EXPECT_FALSE(m_prosystemkeypad->m_buttons.value(60)->isEnabled());
    delete m_prosystemkeypad;
}

TEST_F(Ut_ProSystemKeypad, setvalue)
{
    ProSystemKeypad *m_prosystemkeypad = new ProSystemKeypad;
    Settings::instance()->proBitLength = 8;
    m_prosystemkeypad->setvalue("101111111");
    EXPECT_EQ(m_prosystemkeypad->m_binaryValue, "0000000000000000000000000000000000000000000000000000000001111111");
    Settings::instance()->proBitLength = 64;
    delete m_prosystemkeypad;
}

TEST_F(Ut_ProSystemKeypad, longBitCut)
{
    ProSystemKeypad *m_prosystemkeypad = new ProSystemKeypad;
    m_prosystemkeypad->longBitCut("110000000000000000000000000000000000000000000000000000000101111111");
    EXPECT_EQ(m_prosystemkeypad->m_binaryValue, "0000000000000000000000000000000000000000000000000000000000000000");
    delete m_prosystemkeypad;
}

TEST_F(Ut_ProSystemKeypad, changeBinaryValue)
{
    ProSystemKeypad *m_prosystemkeypad = new ProSystemKeypad;
    m_prosystemkeypad->changeBinaryValue(5);
    EXPECT_EQ(m_prosystemkeypad->m_binaryValue, "0000000000000000000000000000000000000000000000000000000000100000");
    delete m_prosystemkeypad;
}

TEST_F(Ut_ProSystemKeypad, initconnects)
{
    ProSystemKeypad *m_prosystemkeypad = new ProSystemKeypad;
    m_prosystemkeypad->m_buttons.value(1)->clicked();
    EXPECT_EQ(m_prosystemkeypad->m_buttons.value(1)->text(), "0");
    m_prosystemkeypad->m_buttons.value(1)->space();
    EXPECT_EQ(m_prosystemkeypad->m_buttons.value(1)->text(), "1");
    delete m_prosystemkeypad;
}
