// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_scientifickeypad.h"
#include "../../src/control/scientifickeypad.h"
#include "../../src/memorypublic.h"

Ut_ScientificKeypad::Ut_ScientificKeypad()
{

}

TEST_F(Ut_ScientificKeypad, button1)
{
    ScientificKeyPad *m_scientifickeypad = new ScientificKeyPad;
    m_scientifickeypad->button(ScientificKeyPad::Key_arcsin);
    EXPECT_EQ(m_scientifickeypad->button(ScientificKeyPad::Key_sin)->text(), "sin");
    delete m_scientifickeypad;
}

TEST_F(Ut_ScientificKeypad, button2)
{
    ScientificKeyPad *m_scientifickeypad = new ScientificKeyPad;
    m_scientifickeypad->button(42);
    EXPECT_EQ(m_scientifickeypad->button(6)->text(), "sin");
    delete m_scientifickeypad;
}

TEST_F(Ut_ScientificKeypad, animate)
{
    ScientificKeyPad *m_scientifickeypad = new ScientificKeyPad;
    m_scientifickeypad->animate(ScientificKeyPad::Key_sin, false);
    EXPECT_FALSE(static_cast<TextButton *>(m_scientifickeypad->button(ScientificKeyPad::Buttons::Key_sin))->m_isHover);
    delete m_scientifickeypad;
}

TEST_F(Ut_ScientificKeypad, buttonHasFocus)
{
    ScientificKeyPad *m_scientifickeypad = new ScientificKeyPad;
    m_scientifickeypad->button(6)->setFocus();
    EXPECT_FALSE(m_scientifickeypad->buttonHasFocus());
    delete m_scientifickeypad;
}

TEST_F(Ut_ScientificKeypad, buttonThemeChanged)
{
    ScientificKeyPad *m_scientifickeypad = new ScientificKeyPad;
    m_scientifickeypad->m_deg = 3;
    m_scientifickeypad->buttonThemeChanged(1);
    EXPECT_EQ(static_cast<IconButton *>(m_scientifickeypad->button(ScientificKeyPad::Key_deg))->m_currentUrl,
              ":/assets/images/light/deg_normal.svg");
    m_scientifickeypad->buttonThemeChanged(2);
    EXPECT_EQ(static_cast<IconButton *>(m_scientifickeypad->button(ScientificKeyPad::Key_deg))->m_currentUrl,
              ":/assets/images/dark/deg_normal.svg");
    delete m_scientifickeypad;
}

TEST_F(Ut_ScientificKeypad, turnPage)
{
    ScientificKeyPad *m_scientifickeypad = new ScientificKeyPad;
    m_scientifickeypad->turnPage(ScientificKeyPad::Key_page);
    EXPECT_EQ(m_scientifickeypad->m_sqrtwidget->currentIndex(), 1);
    m_scientifickeypad->turnPage(ScientificKeyPad::Key_page);
    EXPECT_EQ(m_scientifickeypad->m_sqrtwidget->currentIndex(), 0);
    delete m_scientifickeypad;
}

TEST_F(Ut_ScientificKeypad, getdeg)
{
    ScientificKeyPad *m_scientifickeypad = new ScientificKeyPad;
    m_scientifickeypad->getdeg(1);
    EXPECT_NE(static_cast<IconButton *>(m_scientifickeypad->button(ScientificKeyPad::Key_deg))->m_currentUrl,
              ":/assets/images/light/grad_normal.svg");
    m_scientifickeypad->getdeg(2);
    EXPECT_NE(static_cast<IconButton *>(m_scientifickeypad->button(ScientificKeyPad::Key_deg))->m_currentUrl,
              ":/assets/images/light/deg_normal.svg");
    m_scientifickeypad->getdeg(3);
    EXPECT_NE(static_cast<IconButton *>(m_scientifickeypad->button(ScientificKeyPad::Key_deg))->m_currentUrl,
              ":/assets/images/light/rad_normal.svg");
    delete m_scientifickeypad;
}

TEST_F(Ut_ScientificKeypad, bracketsNum)
{
    ScientificKeyPad *m_scientifickeypad = new ScientificKeyPad;
    m_scientifickeypad->bracketsNum(0, "0");
    EXPECT_EQ(m_scientifickeypad->m_leftBracket->text(), "");
    m_scientifickeypad->bracketsNum(1, "1");
    EXPECT_EQ(m_scientifickeypad->m_rightBracket->text(), "1");
    delete m_scientifickeypad;
}

bool stub_focus_scikey()
{
    return true;
}

TEST_F(Ut_ScientificKeypad, getFocus)
{
    ScientificKeyPad *m_scientifickeypad = new ScientificKeyPad;
    Stub stub;
    stub.set(ADDR(QWidget, hasFocus), stub_focus_scikey);
    m_scientifickeypad->m_arcsinwidget->setCurrentIndex(0);
    m_scientifickeypad->getFocus(0);
    m_scientifickeypad->getFocus(1);
    m_scientifickeypad->getFocus(2);
    m_scientifickeypad->getFocus(3);
    m_scientifickeypad->getFocus(4);
    m_scientifickeypad->m_arcsinwidget->setCurrentIndex(1);
    m_scientifickeypad->getFocus(0);
    m_scientifickeypad->getFocus(1);
    m_scientifickeypad->getFocus(2);
    m_scientifickeypad->getFocus(3);
    m_scientifickeypad->getFocus(4);
    //无ASSERT
    delete m_scientifickeypad;
}

TEST_F(Ut_ScientificKeypad, connects)
{
    ScientificKeyPad *m_scientifickeypad = new ScientificKeyPad;
    QString str1 = QString("font-family:Noto Sans;color:%1;font-size:14px;")
                   .arg(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name());
    QString str2 = QString("font-family:Noto Sans;color:black;font-size:14px;");
    QString str3 = QString("font-family:Noto Sans;color:white;font-size:14px;");

    m_scientifickeypad->m_themetype = 1;
    m_scientifickeypad->button(ScientificKeyPad::Key_left)->pressed();
    EXPECT_EQ(m_scientifickeypad->m_leftBracket->styleSheet(), str1);
    static_cast<TextButton *>(m_scientifickeypad->button(ScientificKeyPad::Key_left))->mouseRelease();
    EXPECT_EQ(m_scientifickeypad->m_leftBracket->styleSheet(), str2);

    m_scientifickeypad->m_themetype = 2;
    m_scientifickeypad->button(ScientificKeyPad::Key_right)->pressed();
    EXPECT_EQ(m_scientifickeypad->m_rightBracket->styleSheet(), str1);
    static_cast<TextButton *>(m_scientifickeypad->button(ScientificKeyPad::Key_right))->mouseRelease();
    EXPECT_EQ(m_scientifickeypad->m_rightBracket->styleSheet(), str3);
    delete m_scientifickeypad;
}

