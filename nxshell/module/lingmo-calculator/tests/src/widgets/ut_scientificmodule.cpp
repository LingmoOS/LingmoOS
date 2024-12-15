// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_scientificmodule.h"

#include <qrect.h>

#include "../../src/widgets/scientificmodule.h"
#include "../../src/views/memoryitemwidget.h"

#include "../../src/memorypublic.h"

Ut_ScientificModule::Ut_ScientificModule()
{
}

TEST_F(Ut_ScientificModule, signals1)
{
    scientificModule *m_scientificModule = new scientificModule;
    m_scientificModule->m_memhiswidget->hisIsFilled(true);
    EXPECT_TRUE(m_scientificModule->m_havail);

    m_scientificModule->m_memoryPublic->memoryclean();
    m_scientificModule->m_memoryPublic->generateData(Quantity(1));
    EXPECT_TRUE(m_scientificModule->m_memRCbtn);
    EXPECT_TRUE(m_scientificModule->m_avail);

    m_scientificModule->m_memhiswidget->m_memoryWidget->widgetminus(0);
    EXPECT_FALSE(m_scientificModule->m_memoryPublic->isEmpty());

    m_scientificModule->m_memhiswidget->m_memoryWidget->widgetplus(0);
    EXPECT_FALSE(m_scientificModule->m_memoryPublic->isEmpty());

    m_scientificModule->m_memhiswidget->m_memoryWidget->itemclick({"1", Quantity(1)});
    EXPECT_EQ(m_scientificModule->m_sciexpressionBar->getInputEdit()->text(), "1");
    m_scientificModule->deleteLater();
}

bool stub_geometry_contains(const QPoint &p, bool proper = false)
{
    Q_UNUSED(p);
    proper = false;
    return false;
}

TEST_F(Ut_ScientificModule, mousePressEvent)
{
    scientificModule *m_scientificModule = new scientificModule;
    m_scientificModule->m_stackWidget->setCurrentWidget(m_scientificModule->m_memhiswidget);
    Stub stub;
    stub.set((bool (QRect::*)(const QPoint &, bool) const)ADDR(QRect, contains), stub_geometry_contains);
    QMouseEvent *m = new QMouseEvent(QEvent::MouseButtonPress, m_scientificModule->findChild<InputEdit *>()->pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_scientificModule->mousePressEvent(m);
    EXPECT_EQ(m_scientificModule->m_stackWidget->currentWidget(), m_scientificModule->m_scikeypadwidget);
    delete m;
    m_scientificModule->deleteLater();
}

TEST_F(Ut_ScientificModule, setKeyPress)
{
    scientificModule *m_scientificModule = new scientificModule;
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_1, Qt::NoModifier);
    m_scientificModule->setKeyPress(k);
    EXPECT_EQ(m_scientificModule->findChild<InputEdit *>()->text(), "1");
    delete k;
    m_scientificModule->deleteLater();
}

TEST_F(Ut_ScientificModule, checkLineEmpty)
{
    scientificModule *m_scientificModule = new scientificModule;
    m_scientificModule->checkLineEmpty();
    EXPECT_FALSE(m_scientificModule->m_memCalbtn);
    m_scientificModule->findChild<InputEdit *>()->setText("1");
    m_scientificModule->checkLineEmpty();
    EXPECT_TRUE(m_scientificModule->m_memCalbtn);
    m_scientificModule->deleteLater();
}

TEST_F(Ut_ScientificModule, mAvailableEvent)
{
    scientificModule *m_scientificModule = new scientificModule;
    m_scientificModule->mAvailableEvent();
    EXPECT_TRUE(m_scientificModule->m_memRCbtn);
    EXPECT_TRUE(m_scientificModule->m_avail);
    EXPECT_TRUE(static_cast<MemoryButton *>(m_scientificModule->m_memhiskeypad->button(MemHisKeypad::Key_MC))->isEnabled());
    EXPECT_TRUE(static_cast<MemoryButton *>(m_scientificModule->m_memhiskeypad->button(MemHisKeypad::Key_MR))->isEnabled());
    EXPECT_TRUE(static_cast<MemoryButton *>(m_scientificModule->m_memhiskeypad->button(MemHisKeypad::Key_MHlist))->isEnabled());
    m_scientificModule->deleteLater();
}

TEST_F(Ut_ScientificModule, mUnAvailableEvent)
{
    scientificModule *m_scientificModule = new scientificModule;
    m_scientificModule->mUnAvailableEvent();
    EXPECT_FALSE(m_scientificModule->m_memRCbtn);
    EXPECT_FALSE(m_scientificModule->m_avail);
    EXPECT_FALSE(static_cast<MemoryButton *>(m_scientificModule->m_memhiskeypad->button(MemHisKeypad::Key_MC))->isEnabled());
    EXPECT_FALSE(static_cast<MemoryButton *>(m_scientificModule->m_memhiskeypad->button(MemHisKeypad::Key_MR))->isEnabled());
    m_scientificModule->deleteLater();
}

TEST_F(Ut_ScientificModule, initTheme)
{
    scientificModule *m_scientificModule = new scientificModule;
    m_scientificModule->initTheme(1);
    EXPECT_EQ(m_scientificModule->findChild<InputEdit *>()->palette().color(QPalette::ColorGroup::Active, QPalette::ColorRole::Text), "#303030");
    m_scientificModule->initTheme(2);
    EXPECT_EQ(m_scientificModule->findChild<InputEdit *>()->palette().color(QPalette::ColorGroup::Active, QPalette::ColorRole::Text), "#b4b4b4");
    m_scientificModule->deleteLater();
}

TEST_F(Ut_ScientificModule, handleEditKeyPress)
{
    scientificModule *m_scientificModule = new scientificModule;
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_0, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_1, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_2, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_3, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_4, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_5, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_6, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_7, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_8, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_9, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Plus, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Minus, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Underscore, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Asterisk, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Slash, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Enter, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Return, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Equal, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Backspace, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Period, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Escape, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_ParenLeft, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_ParenRight, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Percent, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_AsciiCircum, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_C, Qt::ControlModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_V, Qt::ControlModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_V, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_A, Qt::ControlModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_X, Qt::ControlModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_X, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Delete, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Left, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Right, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Z, Qt::ControlModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Y, Qt::ControlModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Y, Qt::NoModifier);
    m_scientificModule->m_memRCbtn = true;
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_L, Qt::ControlModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_L, Qt::ShiftModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_L, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_R, Qt::ControlModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_R, Qt::ShiftModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_R, Qt::NoModifier);
    m_scientificModule->m_memCalbtn = true;
    m_scientificModule->findChild<InputEdit *>()->setText("1");
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_P, Qt::ControlModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_P, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Q, Qt::ControlModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Q, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_M, Qt::ControlModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_M, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_NumberSign, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_N, Qt::ControlModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_N, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_At, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_B, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Bar, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_E, Qt::ShiftModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_E, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_S, Qt::ShiftModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_S, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_O, Qt::ShiftModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_O, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_T, Qt::ShiftModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_T, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_J, Qt::ShiftModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_J, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Exclam, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Escape, Qt::NoModifier);
    //函数中输入
    QTest::mouseClick(m_scientificModule->findChild<ScientificKeyPad *>()->button(ScientificKeyPad::Key_sin), Qt::LeftButton);
    m_scientificModule->findChild<InputEdit *>()->setCursorPosition(1);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_0, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_1, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_2, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_3, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_4, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_5, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_6, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_7, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_8, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_9, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Plus, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Minus, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Underscore, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Asterisk, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Slash, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_ParenLeft, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_ParenRight, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Slash, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Percent, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_AsciiCircum, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_X, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Y, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_L, Qt::ShiftModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_L, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_R, Qt::ShiftModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_R, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_P, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Q, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_M, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_NumberSign, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_N, Qt::ControlModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_N, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_At, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_B, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Bar, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_E, Qt::ShiftModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_E, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_S, Qt::ShiftModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_S, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_O, Qt::ShiftModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_O, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_T, Qt::ShiftModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_T, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_J, Qt::ShiftModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_J, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Exclam, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_W, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Escape, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_1, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Plus, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_1, Qt::NoModifier);
    QTest::keyClick(m_scientificModule->findChild<InputEdit *>(), Qt::Key_Equal, Qt::NoModifier);
    EXPECT_EQ(m_scientificModule->findChild<InputEdit *>()->text(), "2");
    m_scientificModule->deleteLater();
}

TEST_F(Ut_ScientificModule, handleKeypadButtonPress)
{
    scientificModule *m_scientificModule = new scientificModule;
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_0);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_1);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_2);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_3);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_4);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_5);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_6);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_7);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_8);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_9);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Plus);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Min);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Mult);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Div);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Percent);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Equals);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Clear);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Backspace);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Point);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_left);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_right);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Clear);
    m_scientificModule->findChild<InputEdit *>()->setText("1");
    m_scientificModule->handleKeypadButtonPress(MemHisKeypad::Key_MS);
    m_scientificModule->handleKeypadButtonPress(MemHisKeypad::Key_MC);
    m_scientificModule->handleKeypadButtonPress(MemHisKeypad::Key_Mplus);
    m_scientificModule->handleKeypadButtonPress(MemHisKeypad::Key_Mmin);
    m_scientificModule->handleKeypadButtonPress(MemHisKeypad::Key_MR);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_deg);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_deg);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_deg);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_sin);
    m_scientificModule->handleKeypadButtonPress(MemHisKeypad::Key_FE);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_page);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_PI);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_e);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Mod);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_x2);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Derivative);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Factorials);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_exp);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_cos);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_x3);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_tan);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_xy);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_cot);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_10x);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Modulus);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_log);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Rand);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_ln);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_arcsin);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_sqrt2);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_arccos);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_sqrt3);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_arctan);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_ysqrtx);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_arccot);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_2x);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_logyx);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_ex);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_opposite);
    m_scientificModule->handleKeypadButtonPress(84);
    //函数中输入
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Clear);
    QTest::mouseClick(m_scientificModule->findChild<ScientificKeyPad *>()->button(ScientificKeyPad::Key_sin), Qt::LeftButton);
    m_scientificModule->findChild<InputEdit *>()->setCursorPosition(1);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_0);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_1);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_2);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_3);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_4);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_5);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_6);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_7);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_8);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_9);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Plus);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Min);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Mult);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Div);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Percent);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Point);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_left);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_right);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_sin);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_PI);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_e);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Mod);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_x2);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Derivative);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Factorials);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_exp);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_cos);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_x3);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_tan);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_xy);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_cot);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_10x);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Modulus);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_log);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_ln);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_arcsin);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_sqrt2);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_arccos);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_sqrt3);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_arctan);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_ysqrtx);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_arccot);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_2x);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_logyx);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_ex);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_opposite);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Clear);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_1);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Plus);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_1);
    m_scientificModule->handleKeypadButtonPress(ScientificKeyPad::Key_Equals);
    EXPECT_EQ(m_scientificModule->findChild<InputEdit *>()->text(), "2");
    m_scientificModule->deleteLater();
}

TEST_F(Ut_ScientificModule, handleKeypadButtonPressByspace)
{
    scientificModule *m_scientificModule = new scientificModule;
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_0);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_1);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_2);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_3);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_4);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_5);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_6);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_7);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_8);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_9);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Plus);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Min);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Mult);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Div);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Percent);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Equals);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Clear);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Backspace);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Point);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_left);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_right);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Clear);
    m_scientificModule->findChild<InputEdit *>()->setText("1");
    m_scientificModule->handleKeypadButtonPressByspace(MemHisKeypad::Key_MS);
    m_scientificModule->handleKeypadButtonPressByspace(MemHisKeypad::Key_MC);
    m_scientificModule->handleKeypadButtonPressByspace(MemHisKeypad::Key_Mplus);
    m_scientificModule->handleKeypadButtonPressByspace(MemHisKeypad::Key_Mmin);
    m_scientificModule->handleKeypadButtonPressByspace(MemHisKeypad::Key_MR);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_deg);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_deg);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_deg);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_sin);
    m_scientificModule->handleKeypadButtonPressByspace(MemHisKeypad::Key_FE);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_page);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_page);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_PI);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_e);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Mod);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_x2);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Derivative);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Factorials);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_exp);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_cos);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_x3);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_tan);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_xy);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_cot);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_10x);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Modulus);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_log);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Rand);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_ln);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_arcsin);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_sqrt2);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_arccos);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_sqrt3);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_arctan);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_ysqrtx);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_arccot);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_2x);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_logyx);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_ex);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_opposite);
    //函数中输入
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Clear);
    QTest::mouseClick(m_scientificModule->findChild<ScientificKeyPad *>()->button(ScientificKeyPad::Key_sin), Qt::LeftButton);
    m_scientificModule->findChild<InputEdit *>()->setCursorPosition(1);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_0);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_1);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_2);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_3);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_4);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_5);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_6);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_7);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_8);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_9);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Plus);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Min);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Mult);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Div);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Percent);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Point);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_left);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_right);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_sin);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_PI);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_e);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Mod);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_x2);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Derivative);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Factorials);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_exp);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_cos);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_x3);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_tan);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_xy);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_cot);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_10x);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Modulus);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_log);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_ln);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_arcsin);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_sqrt2);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_arccos);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_sqrt3);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_arctan);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_ysqrtx);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_arccot);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_2x);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_logyx);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_ex);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_opposite);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Clear);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_1);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Plus);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_1);
    m_scientificModule->handleKeypadButtonPressByspace(ScientificKeyPad::Key_Equals);
    EXPECT_EQ(m_scientificModule->findChild<InputEdit *>()->text(), "2");
    m_scientificModule->deleteLater();
}

TEST_F(Ut_ScientificModule, hideMemHisWidget)
{
    scientificModule *m_scientificModule = new scientificModule;
    m_scientificModule->m_avail = true;
    m_scientificModule->hideMemHisWidget();
    EXPECT_FALSE(m_scientificModule-> m_isallgray);
    EXPECT_TRUE(m_scientificModule-> m_memRCbtn);
    m_scientificModule->m_avail = false;
    m_scientificModule->hideMemHisWidget();
    EXPECT_EQ(m_scientificModule->m_stackWidget->currentWidget(), m_scientificModule->m_scikeypadwidget);
    EXPECT_FALSE(m_scientificModule-> m_memRCbtn);
    m_scientificModule->deleteLater();
}

int stub_model_sci()
{
    return 0;
}

TEST_F(Ut_ScientificModule, clickListView)
{
    scientificModule *m_scientificModule = new scientificModule;
    m_scientificModule->m_sciexpressionBar->getInputEdit()->setText("1+1");
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    m_scientificModule->handleEditKeyPress(k);
    Stub stub;
    stub.set(ADDR(QModelIndex, row), stub_model_sci);
    m_scientificModule->clickListView(QModelIndex());
    EXPECT_EQ(m_scientificModule->m_sciexpressionBar->getInputEdit()->text(), "2");
    delete k;
    m_scientificModule->deleteLater();
}
