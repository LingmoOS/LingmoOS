// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_memorykeypad.h"
#include "../../src/control/memorykeypad.h"

Ut_MemoryKeypad::Ut_MemoryKeypad()
{

}

TEST_F(Ut_MemoryKeypad, button)
{
    MemoryKeypad *m_memorykeypad = new MemoryKeypad;
    EXPECT_EQ(m_memorykeypad->button(MemoryKeypad::Buttons::Key_MC)->text(), "MC");
    delete m_memorykeypad;
}

TEST_F(Ut_MemoryKeypad, getFocus)
{
    MemoryKeypad *m_basickeypad = new MemoryKeypad;
    m_basickeypad->getFocus(1);
    m_basickeypad->getFocus(2);
    m_basickeypad->getFocus(3);
    //无ASSERT
    delete m_basickeypad;
}
