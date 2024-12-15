// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_basickeypad.h"
#include "../../src/control/basickeypad.h"
#include "../../src/memorypublic.h"

Ut_BasicKeypad::Ut_BasicKeypad()
{

}

TEST_F(Ut_BasicKeypad, buttonThemeChanged)
{
    BasicKeypad *m_basickeypad = new BasicKeypad;
    m_basickeypad->buttonThemeChanged(1);
    EXPECT_EQ(static_cast<IconButton *>(m_basickeypad->button(BasicKeypad::Key_Mult))->m_currentUrl, ":/assets/images/light/x_normal.svg");
    m_basickeypad->buttonThemeChanged(2);
    EXPECT_EQ(static_cast<IconButton *>(m_basickeypad->button(BasicKeypad::Key_Div))->m_currentUrl, ":/assets/images/dark/divide_normal.svg");
    delete m_basickeypad;
}

TEST_F(Ut_BasicKeypad, getFocus)
{
    BasicKeypad *m_basickeypad = new BasicKeypad;
    m_basickeypad->getFocus(0);
    m_basickeypad->getFocus(1);
    m_basickeypad->getFocus(2);
    m_basickeypad->getFocus(3);
    m_basickeypad->getFocus(4);
    //焦点函数，无assert
    delete m_basickeypad;
}
