// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_procheckbtnkeypad.h"
#include "../../src/control/procheckbtnkeypad.h"

Ut_ProCheckBtnKeypad::Ut_ProCheckBtnKeypad()
{

}

TEST_F(Ut_ProCheckBtnKeypad, button)
{
    ProCheckBtnKeypad *m_procheckbtnkeypad = new ProCheckBtnKeypad;
    EXPECT_EQ(m_procheckbtnkeypad->button(ProCheckBtnKeypad::Buttons::Key_MS)->text(), "MS");
    delete m_procheckbtnkeypad;
}

TEST_F(Ut_ProCheckBtnKeypad, animate)
{
    ProCheckBtnKeypad *m_procheckbtnkeypad = new ProCheckBtnKeypad;
    m_procheckbtnkeypad->animate(ProCheckBtnKeypad::Buttons::Key_MS, false);
    EXPECT_FALSE(static_cast<MemoryButton *>(m_procheckbtnkeypad->button(ProCheckBtnKeypad::Buttons::Key_MS))->m_isHover);
    delete m_procheckbtnkeypad;
}

TEST_F(Ut_ProCheckBtnKeypad, buttonHasFocus)
{
    ProCheckBtnKeypad *m_procheckbtnkeypad = new ProCheckBtnKeypad;
    EXPECT_FALSE(m_procheckbtnkeypad->buttonHasFocus());
    delete m_procheckbtnkeypad;
}

TEST_F(Ut_ProCheckBtnKeypad, initButtons)
{
    ProCheckBtnKeypad *m_procheckbtnkeypad = new ProCheckBtnKeypad;
    m_procheckbtnkeypad->initButtons();
    EXPECT_EQ(m_procheckbtnkeypad->m_keys.size(), 6);
    delete m_procheckbtnkeypad;
}

TEST_F(Ut_ProCheckBtnKeypad, buttonThemeChanged)
{
    ProCheckBtnKeypad *m_procheckbtnkeypad = new ProCheckBtnKeypad;
    m_procheckbtnkeypad->buttonThemeChanged(1);
    EXPECT_EQ(static_cast<IconButton *>(m_procheckbtnkeypad->button(ProCheckBtnKeypad::Key_GeneralKeypad))->m_currentUrl,
              ":/assets/images/light/icon_generalkeyboard_normal.svg");
    m_procheckbtnkeypad->buttonThemeChanged(2);
    EXPECT_EQ(static_cast<IconButton *>(m_procheckbtnkeypad->button(ProCheckBtnKeypad::Key_BinaryKeypad))->m_currentUrl,
              ":/assets/images/dark/icon_binarysystem_normal.svg");
    delete m_procheckbtnkeypad;
}

TEST_F(Ut_ProCheckBtnKeypad, getFocus)
{
    ProCheckBtnKeypad *m_procheckbtnkeypad = new ProCheckBtnKeypad;
    m_procheckbtnkeypad->getFocus(1);
    m_procheckbtnkeypad->getFocus(2);
    m_procheckbtnkeypad->getFocus(3);
    //焦点函数，无assert
    delete m_procheckbtnkeypad;
}
