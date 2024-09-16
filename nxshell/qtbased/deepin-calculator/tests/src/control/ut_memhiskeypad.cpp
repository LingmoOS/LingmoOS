// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_memhiskeypad.h"
#include "../../src/control/memhiskeypad.h"

Ut_MemHisKeypad::Ut_MemHisKeypad()
{

}

TEST_F(Ut_MemHisKeypad, button)
{
    MemHisKeypad *m_memhiskeypad = new MemHisKeypad;
    EXPECT_EQ(m_memhiskeypad->button(MemHisKeypad::Buttons::Key_MC)->text(), "MC");
    delete m_memhiskeypad;
}

TEST_F(Ut_MemHisKeypad, animate)
{
    MemHisKeypad *m_memhiskeypad = new MemHisKeypad;
    m_memhiskeypad->animate(MemHisKeypad::Buttons::Key_MC, false);
    EXPECT_FALSE(static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Buttons::Key_MC))->m_isHover);
    delete m_memhiskeypad;
}

TEST_F(Ut_MemHisKeypad, buttonHasFocus)
{
    MemHisKeypad *m_memhiskeypad = new MemHisKeypad;
    EXPECT_FALSE(m_memhiskeypad->buttonHasFocus());
    delete m_memhiskeypad;
}

TEST_F(Ut_MemHisKeypad, initButtons)
{
    MemHisKeypad *m_memhiskeypad = new MemHisKeypad;
    m_memhiskeypad->initButtons();
    EXPECT_EQ(m_memhiskeypad->m_keys.size(), 7);
    delete m_memhiskeypad;
}

TEST_F(Ut_MemHisKeypad, getFocus)
{
    MemHisKeypad *m_memhiskeypad = new MemHisKeypad;
    m_memhiskeypad->getFocus(1);
    m_memhiskeypad->getFocus(2);
    m_memhiskeypad->getFocus(3);
    //焦点函数，无assert
    delete m_memhiskeypad;
}
