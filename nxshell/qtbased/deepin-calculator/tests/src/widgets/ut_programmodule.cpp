// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QPalette>
#include "ut_programmodule.h"

#include "../../src/widgets/programmodule.h"
#include "../../3rdparty/core/settings.h"
#include <QPair>

Ut_ProgramModule::Ut_ProgramModule()
{
}

TEST_F(Ut_ProgramModule, Signals)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->m_proExpressionBar->getInputEdit()->cursorPositionNumberChanged("1");
    EXPECT_EQ(m_programmodule->m_proSystemKeypad->m_binaryValue.back(), "1");

    QPair<bool, Quantity> pair = {false, Quantity(1)};
    m_programmodule->m_proExpressionBar->getInputEdit()->prolistAns(pair);
    EXPECT_EQ(m_programmodule->m_proListModel->m_expressionList.at(0), "");

    QPair<bool, Quantity> pair1 = {true, Quantity(1)};
    m_programmodule->m_proExpressionBar->getInputEdit()->prolistAns(pair1);
    EXPECT_EQ(m_programmodule->m_proListModel->m_expressionList.at(0), "1");

    m_programmodule->m_proExpressionBar->getInputEdit()->emptyExpression(true);
    EXPECT_FALSE(m_programmodule->m_memCalbtn);

    m_programmodule->m_proExpressionBar->getInputEdit()->emptyExpression(false);
    EXPECT_TRUE(m_programmodule->m_memCalbtn);

    m_programmodule->m_memorylistwidget->widgetplus(0);
    EXPECT_TRUE(m_programmodule->m_memoryPublic->isEmpty());
    m_programmodule->m_memorylistwidget->widgetminus(0);
    EXPECT_TRUE(m_programmodule->m_memoryPublic->isEmpty());

    m_programmodule->m_memoryPublic->generateData(Quantity(1));
    m_programmodule->m_memoryPublic->memoryclean();
    EXPECT_TRUE(m_programmodule->m_memoryPublic->isEmpty());

    m_programmodule->m_stackWidget->setCurrentIndex(2);
    QPair<QString, Quantity> pair2 = {"1", Quantity(1)};
    m_programmodule->m_memorylistwidget->itemclick(pair2);
    EXPECT_TRUE(m_programmodule->m_memCalbtn);
    EXPECT_FALSE(m_programmodule->m_isallgray);
    EXPECT_EQ(m_programmodule->m_proExpressionBar->getInputEdit()->text(), "1");

    m_programmodule->m_memorylistwidget->hideWidget();
    EXPECT_FALSE(m_programmodule->m_isallgray);
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, Signals1)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->m_checkBtnKeypad->buttonPressed(38);
    m_programmodule->m_checkBtnKeypad->buttonPressedbySpace(38);
    m_programmodule->m_programmerKeypad->buttonPressed(26);
    m_programmodule->m_programmerKeypad->buttonPressedbySpace(26);
    m_programmodule->m_proSystemKeypad->bitbuttonclicked();
    EXPECT_EQ(m_programmodule->findChild<InputEdit *>()->text(), "11");
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, Signals2)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->m_proSystemKeypad->longbitcut(Quantity(1));
    EXPECT_TRUE(m_programmodule->m_proExpressionBar->findChild<SimpleListModel *>()->m_expressionList.isEmpty());

    m_programmodule->m_byteArrowListWidget->focus(1);
    EXPECT_EQ(m_programmodule->m_byteArrowListWidget->currentRow(), 1);
    m_programmodule->m_byteArrowListWidget->focus(0);
    EXPECT_EQ(m_programmodule->m_byteArrowListWidget->currentRow(), 0);

    m_programmodule->m_shiftArrowListWidget->focus(1);
    EXPECT_EQ(m_programmodule->m_shiftArrowListWidget->currentRow(), 1);
    m_programmodule->m_shiftArrowListWidget->focus(0);
    EXPECT_EQ(m_programmodule->m_shiftArrowListWidget->currentRow(), 0);

    m_programmodule->m_byteArrowRectangle->hidearrowrectangle();
    EXPECT_TRUE(m_programmodule->m_byteArrowRectangle->isHidden());
    m_programmodule->m_shiftArrowRectangle->hidearrowrectangle();
    EXPECT_TRUE(m_programmodule->m_shiftArrowRectangle->isHidden());

    Settings::instance()->programmerBase = 0;
    Settings::instance()->proBitLength = 64;
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, setKeyPress)
{
    ProgramModule *m_programmodule = new ProgramModule;
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_1, Qt::NoModifier);
    m_programmodule->setKeyPress(k);
    EXPECT_EQ(m_programmodule->findChild<InputEdit *>()->text(), "1");
    delete k;
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, mAvailableEvent)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->mAvailableEvent();
    EXPECT_TRUE(m_programmodule->m_avail);
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, mUnAvailableEvent)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->mUnAvailableEvent();
    EXPECT_FALSE(m_programmodule->m_avail);
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, showListWidget)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->showListWidget();
    EXPECT_TRUE(m_programmodule->m_isallgray);
    EXPECT_EQ(m_programmodule->m_stackWidget->currentIndex(), 2);
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, closeListWidget)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->m_stackWidget->setCurrentIndex(2);
    m_programmodule->m_insidewidget = false;
    m_programmodule->closeListWidget();
    EXPECT_EQ(m_programmodule->m_stackWidget->currentIndex(), 0);
    EXPECT_FALSE(m_programmodule->m_isallgray);
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, handleEditKeyPress)
{
    ProgramModule *m_programmodule = new ProgramModule;
    Settings::instance()->programmerBase = 16;
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_0, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_1, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_2, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_3, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_4, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_5, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_6, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_7, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_8, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_9, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_A, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_B, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_C, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_D, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_E, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_F, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Plus, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Minus, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Underscore, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Asterisk, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Slash, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Enter, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Return, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Equal, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Backspace, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Period, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Escape, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_ParenLeft, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_ParenRight, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Percent, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_AsciiCircum, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Ampersand, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Backslash, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Bar, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_AsciiTilde, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_C, Qt::ControlModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_V, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_V, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_A, Qt::ControlModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_X, Qt::ControlModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_X, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Delete, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Left, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Right, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Z, Qt::ControlModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Y, Qt::ControlModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Y, Qt::NoModifier);
    m_programmodule->m_avail = true;
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_L, Qt::ControlModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_L, Qt::ShiftModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_L, Qt::NoModifier);
    m_programmodule->m_avail = true;
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_R, Qt::ControlModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_R, Qt::ShiftModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_R, Qt::NoModifier);
    m_programmodule->m_memCalbtn = true;
    m_programmodule->findChild<InputEdit *>()->setText("1");
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_P, Qt::ControlModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_P, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Q, Qt::ControlModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Q, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_M, Qt::ControlModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_M, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_NumberSign, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_N, Qt::ControlModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_N, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_At, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_B, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Bar, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_E, Qt::ShiftModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_E, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_S, Qt::ShiftModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_S, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_O, Qt::ShiftModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_O, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_T, Qt::ShiftModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_T, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_J, Qt::ShiftModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_J, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Exclam, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Escape, Qt::NoModifier);
    //函数中输入
    QTest::mouseClick(m_programmodule->findChild<ProgrammerKeypad *>()->button(ProgrammerKeypad::Key_AND), Qt::LeftButton);
    m_programmodule->findChild<InputEdit *>()->setCursorPosition(1);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_0, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_1, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_2, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_3, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_4, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_5, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_6, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_7, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_8, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_9, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_A, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_B, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_C, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_D, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_E, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_F, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Plus, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Minus, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Underscore, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Asterisk, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Slash, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Period, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_ParenLeft, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_ParenRight, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Percent, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_AsciiCircum, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_X, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Y, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_L, Qt::ShiftModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_L, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_R, Qt::ShiftModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_R, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_P, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Q, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_M, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_NumberSign, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_N, Qt::ControlModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_N, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_At, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_B, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Bar, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_E, Qt::ShiftModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_E, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_S, Qt::ShiftModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_S, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_O, Qt::ShiftModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_O, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_T, Qt::ShiftModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_T, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_J, Qt::ShiftModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_J, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Exclam, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_W, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Escape, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_1, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Plus, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_1, Qt::NoModifier);
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Equal, Qt::NoModifier);
    EXPECT_EQ(m_programmodule->findChild<InputEdit *>()->text(), "2");
    Settings::instance()->programmerBase = 0;
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, handleKeypadButtonPress)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->handleKeypadButtonPress(ProCheckBtnKeypad::Key_BinaryKeypad);
    m_programmodule->handleKeypadButtonPress(ProCheckBtnKeypad::Key_GeneralKeypad);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_0);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_1);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_2);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_3);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_4);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_5);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_6);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_7);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_8);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_9);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_Plus);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_Min);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_Mult);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_Div);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_percent);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_moveL);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_moveR);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_Clear);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_Backspace);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_opposite);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_leftBracket);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_rightBracket);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_OR);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_XOR);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_NAND);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_NOR);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_NOT);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_Clear);
    m_programmodule->findChild<InputEdit *>()->setText("1");
    m_programmodule->handleKeypadButtonPress(ProCheckBtnKeypad::Key_BinaryKeypad);
    m_programmodule->handleKeypadButtonPress(ProCheckBtnKeypad::Key_GeneralKeypad);
    m_programmodule->handleKeypadButtonPress(ProCheckBtnKeypad::Key_System);
    m_programmodule->handleKeypadButtonPress(ProCheckBtnKeypad::Key_Option);
    m_programmodule->handleKeypadButtonPress(ProCheckBtnKeypad::Key_MS);
    //函数中输入
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_Clear);
    QTest::mouseClick(m_programmodule->findChild<ProgrammerKeypad *>()->button(ProgrammerKeypad::Key_AND), Qt::LeftButton);
    m_programmodule->findChild<InputEdit *>()->setCursorPosition(1);
    Settings::instance()->programmerBase = 16;
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_0);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_1);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_2);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_3);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_4);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_5);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_6);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_7);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_8);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_9);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_A);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_B);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_C);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_D);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_E);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_F);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_Plus);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_Min);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_Mult);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_Div);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_percent);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_moveL);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_moveR);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_Clear);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_Backspace);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_opposite);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_leftBracket);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_rightBracket);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_Clear);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_1);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_Plus);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_3);
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_equal);
    m_programmodule->handleKeypadButtonPress(ProCheckBtnKeypad::Key_MS);
    m_programmodule->handleKeypadButtonPress(ProCheckBtnKeypad::Key_Mlist);
    EXPECT_EQ(m_programmodule->findChild<InputEdit *>()->text(), "4");
    Settings::instance()->programmerBase = 0;
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, handleKeypadButtonPressByspace)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_0);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_1);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_2);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_3);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_4);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_5);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_6);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_7);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_8);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_9);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_Plus);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_Min);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_Mult);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_Div);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_percent);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_moveL);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_moveR);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_Clear);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_Backspace);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_opposite);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_leftBracket);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_rightBracket);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_OR);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_XOR);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_NAND);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_NOR);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_NOT);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_Clear);
    m_programmodule->findChild<InputEdit *>()->setText("1");
    m_programmodule->handleKeypadButtonPressByspace(ProCheckBtnKeypad::Key_BinaryKeypad);
    m_programmodule->handleKeypadButtonPressByspace(ProCheckBtnKeypad::Key_GeneralKeypad);
    m_programmodule->handleKeypadButtonPressByspace(ProCheckBtnKeypad::Key_System);
    m_programmodule->handleKeypadButtonPressByspace(ProCheckBtnKeypad::Key_Option);
    m_programmodule->handleKeypadButtonPressByspace(ProCheckBtnKeypad::Key_MS);
    //函数中输入
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_Clear);
    QTest::mouseClick(m_programmodule->findChild<ProgrammerKeypad *>()->button(ProgrammerKeypad::Key_AND), Qt::LeftButton);
    m_programmodule->findChild<InputEdit *>()->setCursorPosition(1);
    Settings::instance()->programmerBase = 16;
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_0);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_1);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_2);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_3);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_4);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_5);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_6);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_7);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_8);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_9);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_A);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_B);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_C);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_D);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_E);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_F);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_Plus);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_Min);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_Mult);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_Div);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_percent);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_moveL);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_moveR);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_Clear);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_Backspace);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_opposite);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_leftBracket);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_rightBracket);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_Clear);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_1);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_Plus);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_3);
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_equal);
    m_programmodule->handleKeypadButtonPressByspace(ProCheckBtnKeypad::Key_MS);
    m_programmodule->handleKeypadButtonPressByspace(ProCheckBtnKeypad::Key_Mlist);
    EXPECT_EQ(m_programmodule->findChild<InputEdit *>()->text(), "4");
    Settings::instance()->programmerBase = 0;
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, moveL)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->m_shiftArrowCurrentRow = 0;
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_moveL);
    m_programmodule->m_shiftArrowCurrentRow = 1;
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_moveL);
    m_programmodule->m_shiftArrowCurrentRow = 2;
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_moveL);
    m_programmodule->m_shiftArrowCurrentRow = 3;
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_moveL);
    EXPECT_EQ(m_programmodule->m_proExpressionBar->getInputEdit()->text(), "0rcl");
    m_programmodule->m_proExpressionBar->enterClearEvent();
    m_programmodule->m_shiftArrowCurrentRow = 0;
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_moveL);
    m_programmodule->m_shiftArrowCurrentRow = 1;
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_moveL);
    m_programmodule->m_shiftArrowCurrentRow = 2;
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_moveL);
    m_programmodule->m_shiftArrowCurrentRow = 3;
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_moveL);
    EXPECT_EQ(m_programmodule->m_proExpressionBar->getInputEdit()->text(), "0rcl");
    m_programmodule->m_proExpressionBar->enterClearEvent();
    m_programmodule->m_shiftArrowCurrentRow = 0;
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Less, Qt::NoModifier);
    m_programmodule->m_shiftArrowCurrentRow = 1;
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Less, Qt::NoModifier);
    m_programmodule->m_shiftArrowCurrentRow = 2;
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Less, Qt::NoModifier);
    m_programmodule->m_shiftArrowCurrentRow = 3;
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Less, Qt::NoModifier);
    EXPECT_EQ(m_programmodule->m_proExpressionBar->getInputEdit()->text(), "0rcl");
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, moveR)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->m_shiftArrowCurrentRow = 0;
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_moveR);
    m_programmodule->m_shiftArrowCurrentRow = 1;
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_moveR);
    m_programmodule->m_shiftArrowCurrentRow = 2;
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_moveR);
    m_programmodule->m_shiftArrowCurrentRow = 3;
    m_programmodule->handleKeypadButtonPress(ProgrammerKeypad::Key_moveR);
    EXPECT_EQ(m_programmodule->m_proExpressionBar->getInputEdit()->text(), "0rcr");
    m_programmodule->m_proExpressionBar->enterClearEvent();
    m_programmodule->m_shiftArrowCurrentRow = 0;
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_moveR);
    m_programmodule->m_shiftArrowCurrentRow = 1;
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_moveR);
    m_programmodule->m_shiftArrowCurrentRow = 2;
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_moveR);
    m_programmodule->m_shiftArrowCurrentRow = 3;
    m_programmodule->handleKeypadButtonPressByspace(ProgrammerKeypad::Key_moveR);
    EXPECT_EQ(m_programmodule->m_proExpressionBar->getInputEdit()->text(), "0rcr");
    m_programmodule->m_proExpressionBar->enterClearEvent();
    m_programmodule->m_shiftArrowCurrentRow = 0;
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Greater, Qt::NoModifier);
    m_programmodule->m_shiftArrowCurrentRow = 1;
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Greater, Qt::NoModifier);
    m_programmodule->m_shiftArrowCurrentRow = 2;
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Greater, Qt::NoModifier);
    m_programmodule->m_shiftArrowCurrentRow = 3;
    QTest::keyClick(m_programmodule->findChild<InputEdit *>(), Qt::Key_Greater, Qt::NoModifier);
    EXPECT_EQ(m_programmodule->m_proExpressionBar->getInputEdit()->text(), "0rcr");
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, shiftArrowListWidgetItemClicked)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->shiftArrowListWidgetItemSpace();
    m_programmodule->shiftArrowListWidgetItemClicked(3, false);
    EXPECT_EQ(m_programmodule->m_shiftArrowCurrentRow, 3);
    m_programmodule->m_proListView->testAttribute(Qt::WA_TransparentForMouseEvents);
    m_programmodule->shiftArrowListWidgetItemClicked(0, false);
    EXPECT_EQ(m_programmodule->m_shiftArrowCurrentRow, 0);
    m_programmodule->m_proListView->testAttribute(Qt::WA_TransparentForMouseEvents);
    m_programmodule->shiftArrowListWidgetItemClicked(1, false);
    EXPECT_EQ(m_programmodule->m_shiftArrowCurrentRow, 1);
    m_programmodule->m_proListView->testAttribute(Qt::WA_TransparentForMouseEvents);
    m_programmodule->shiftArrowListWidgetItemClicked(2, false);
    m_programmodule->m_proListView->testAttribute(Qt::WA_TransparentForMouseEvents);
    EXPECT_EQ(m_programmodule->m_shiftArrowCurrentRow, 2);
    m_programmodule->shiftArrowListWidgetItemPressed(0);
    EXPECT_EQ(m_programmodule->m_shiftArrowPressRow, 0);
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, byteArrowListWidgetItemClicked)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->byteArrowListWidgetItemSpace();
    m_programmodule->byteArrowListWidgetItemClicked(3, false);
    EXPECT_EQ(Settings::instance()->proBitLength, 8);
    m_programmodule->byteArrowListWidgetItemClicked(0, false);
    EXPECT_EQ(Settings::instance()->proBitLength, 64);
    m_programmodule->byteArrowListWidgetItemClicked(1, false);
    EXPECT_EQ(Settings::instance()->proBitLength, 32);
    m_programmodule->byteArrowListWidgetItemClicked(2, false);
    m_programmodule->m_proListView->testAttribute(Qt::WA_TransparentForMouseEvents);
    EXPECT_EQ(Settings::instance()->proBitLength, 16);
    m_programmodule->byteArrowListWidgetItemPressed(0);
    EXPECT_EQ(m_programmodule->m_byteArrowPressRow, 0);
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, handleClearStateChanged)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->handleClearStateChanged(true);
    EXPECT_EQ(m_programmodule->findChild<ProgrammerKeypad *>()->button(ProgrammerKeypad::Key_Clear)->text(), "AC");
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, mousePressEvent)
{
    ProgramModule *m_programmodule = new ProgramModule;
    QMouseEvent *m = new QMouseEvent(QMouseEvent::Type::MouseButtonPress,
                                     m_programmodule->pos(), Qt::MouseButton::LeftButton,
                                     Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
    m_programmodule->mousePressEvent(m);
    EXPECT_FALSE(m_programmodule->m_insidewidget);
    delete m;
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, setwidgetAttribute)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->setwidgetAttribute(false);
    EXPECT_FALSE(m_programmodule->m_proExpressionBar->testAttribute(Qt::WA_TransparentForMouseEvents));
    m_programmodule->deleteLater();
}

TEST_F(Ut_ProgramModule, checkBtnKeypadThemeChange)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->checkBtnKeypadThemeChange(1);
    EXPECT_EQ(static_cast<IconButton *>(m_programmodule->m_checkBtnKeypad->button(ProCheckBtnKeypad::Key_GeneralKeypad))->m_currentUrl,
              ":/assets/images/light/icon_generalkeyboard_normal.svg");
    m_programmodule->checkBtnKeypadThemeChange(2);
    EXPECT_EQ(static_cast<IconButton *>(m_programmodule->m_checkBtnKeypad->button(ProCheckBtnKeypad::Key_GeneralKeypad))->m_currentUrl,
              ":/assets/images/dark/icon_generalkeyboard_normal.svg");
    m_programmodule->deleteLater();
}

int stub_model_pro()
{
    return 1;
}

TEST_F(Ut_ProgramModule, radixListChange)
{
    ProgramModule *m_programmodule = new ProgramModule;
    m_programmodule->m_proExpressionBar->getInputEdit()->setText("12");
    Stub stub;
    stub.set(ADDR(QModelIndex, row), stub_model_pro);
    m_programmodule->radixListChange(QModelIndex(), false);
    EXPECT_EQ(Settings::instance()->programmerBase, 10);
    Settings::instance()->programmerBase = 0;
    m_programmodule->deleteLater();
}
