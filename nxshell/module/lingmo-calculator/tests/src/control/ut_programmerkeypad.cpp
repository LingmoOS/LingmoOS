// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_programmerkeypad.h"
#include "../../src/control/programmerkeypad.h"


Ut_ProgrammerKeypad::Ut_ProgrammerKeypad()
{

}

TEST_F(Ut_ProgrammerKeypad, button)
{
    ProgrammerKeypad *m_programmerkeypad = new ProgrammerKeypad;
    EXPECT_EQ(m_programmerkeypad->button(ProgrammerKeypad::Buttons::Key_AND)->text(), "AND");
    delete m_programmerkeypad;
}

TEST_F(Ut_ProgrammerKeypad, animate)
{
    ProgrammerKeypad *m_programmerkeypad = new ProgrammerKeypad;
    m_programmerkeypad->animate(ProgrammerKeypad::Buttons::Key_AND, false);
    m_programmerkeypad->animate(ProgrammerKeypad::Buttons::Key_equal, false);
    m_programmerkeypad->animate(ProgrammerKeypad::Buttons::Key_moveL, false);
    EXPECT_FALSE(static_cast<TextButton *>(m_programmerkeypad->button(ProgrammerKeypad::Buttons::Key_AND))->m_isHover);
    EXPECT_FALSE(static_cast<EqualButton *>(m_programmerkeypad->button(ProgrammerKeypad::Buttons::Key_equal))->m_isHover);
    EXPECT_FALSE(static_cast<TextButton *>(m_programmerkeypad->button(ProgrammerKeypad::Buttons::Key_moveL))->m_isHover);
    delete m_programmerkeypad;
}

TEST_F(Ut_ProgrammerKeypad, buttonHasFocus)
{
    ProgrammerKeypad *m_programmerkeypad = new ProgrammerKeypad;
    EXPECT_FALSE(m_programmerkeypad->buttonHasFocus());
    delete m_programmerkeypad;
}

TEST_F(Ut_ProgrammerKeypad, initButtons)
{
    ProgrammerKeypad *m_programmerkeypad = new ProgrammerKeypad;
    m_programmerkeypad->initButtons();
    EXPECT_EQ(m_programmerkeypad->m_keys.size(), 36);
    delete m_programmerkeypad;
}

TEST_F(Ut_ProgrammerKeypad, buttonThemeChanged)
{
    ProgrammerKeypad *m_programmerkeypad = new ProgrammerKeypad;
    m_programmerkeypad->buttonThemeChanged(1);
    EXPECT_EQ(static_cast<IconButton *>(m_programmerkeypad->button(ProgrammerKeypad::Key_Div))->m_currentUrl,
              ":/assets/images/light/divide_normal.svg");
    m_programmerkeypad->buttonThemeChanged(2);
    EXPECT_EQ(static_cast<IconButton *>(m_programmerkeypad->button(ProgrammerKeypad::Key_Mult))->m_currentUrl,
              ":/assets/images/dark/x_normal.svg");
    delete m_programmerkeypad;
}

TEST_F(Ut_ProgrammerKeypad, getFocus)
{
    ProgrammerKeypad *m_programmerkeypad = new ProgrammerKeypad;
    m_programmerkeypad->getFocus(1);
    m_programmerkeypad->getFocus(2);
    m_programmerkeypad->getFocus(3);
    //焦点函数无assert
    delete m_programmerkeypad;
}

TEST_F(Ut_ProgrammerKeypad, radixChanged)
{
    ProgrammerKeypad *m_programmerkeypad = new ProgrammerKeypad;
    m_programmerkeypad->radixChanged(0);
    EXPECT_EQ(m_programmerkeypad->m_currentBase, 16);
    m_programmerkeypad->radixChanged(1);
    EXPECT_EQ(m_programmerkeypad->m_currentBase, 10);
    m_programmerkeypad->radixChanged(2);
    EXPECT_EQ(m_programmerkeypad->m_currentBase, 8);
    m_programmerkeypad->radixChanged(3);
    EXPECT_EQ(m_programmerkeypad->m_currentBase, 2);
    m_programmerkeypad->radixChanged(5);
    EXPECT_EQ(m_programmerkeypad->m_currentBase, 10);
    delete m_programmerkeypad;
}

TEST_F(Ut_ProgrammerKeypad, bracketsNum)
{
    ProgrammerKeypad *m_programmerkeypad = new ProgrammerKeypad;
    m_programmerkeypad->bracketsNum(0, "0");
    EXPECT_EQ(m_programmerkeypad->m_leftBracket->text(), "");
    m_programmerkeypad->bracketsNum(1, "1");
    EXPECT_EQ(m_programmerkeypad->m_rightBracket->text(), "1");
    delete m_programmerkeypad;
}

TEST_F(Ut_ProgrammerKeypad, connects)
{
    ProgrammerKeypad *m_programmerkeypad = new ProgrammerKeypad;
    QString str1 = QString("font-family:Noto Sans;color:%1;font-size:14px;")
                   .arg(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name());
    QString str2 = QString("font-family:Noto Sans;color:black;font-size:14px;");
    QString str3 = QString("font-family:Noto Sans;color:white;font-size:14px;");

    m_programmerkeypad->m_themetype = 1;
    m_programmerkeypad->button(ProgrammerKeypad::Key_leftBracket)->pressed();
    EXPECT_EQ(m_programmerkeypad->m_leftBracket->styleSheet(), str1);
    static_cast<TextButton *>(m_programmerkeypad->button(ProgrammerKeypad::Key_leftBracket))->mouseRelease();
    EXPECT_EQ(m_programmerkeypad->m_leftBracket->styleSheet(), str2);

    m_programmerkeypad->m_themetype = 2;
    m_programmerkeypad->button(ProgrammerKeypad::Key_rightBracket)->pressed();
    EXPECT_EQ(m_programmerkeypad->m_rightBracket->styleSheet(), str1);
    static_cast<TextButton *>(m_programmerkeypad->button(ProgrammerKeypad::Key_rightBracket))->mouseRelease();
    EXPECT_EQ(m_programmerkeypad->m_rightBracket->styleSheet(), str3);
    delete m_programmerkeypad;
}

//TEST_F(Ut_ProgrammerKeypad, initUI)
//{
//    ProgrammerKeypad *m_programmerkeypad = new ProgrammerKeypad;
//    m_programmerkeypad->bracketsNum(0, "0");
//    m_programmerkeypad->bracketsNum(1, "1");
//    ASSERT_EQ(m_programmerkeypad->m_rightBracket->text(), "1");
//    DSettingsAlt::deleteInstance();
//}

