// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QPalette>
#include "ut_basicmodul.h"

#include "../../src/widgets/basicmodule.h"

Ut_BasicModul::Ut_BasicModul()
{

}

TEST_F(Ut_BasicModul, Signals)
{
    BasicModule *m_basicmodul = new BasicModule;

    m_basicmodul->m_memorylistwidget->widgetplus(0);
    EXPECT_EQ(m_basicmodul->findChild<InputEdit *>()->text(), "");
    m_basicmodul->m_memorylistwidget->widgetminus(0);
    EXPECT_EQ(m_basicmodul->findChild<InputEdit *>()->text(), "");
    m_basicmodul->m_memoryPublic->generateData(Quantity(1));
    m_basicmodul->m_memoryPublic->memoryclean();
    m_basicmodul->m_keypadLayout->setCurrentIndex(1);
    QPair<QString, Quantity> pair2 = {"1", Quantity(1)};
    m_basicmodul->m_memorylistwidget->itemclick(pair2);
    EXPECT_EQ(m_basicmodul->findChild<InputEdit *>()->text(), "1");
    m_basicmodul->m_memorylistwidget->hideWidget();
    EXPECT_EQ(m_basicmodul->m_keypadLayout->currentIndex(), 0);
    m_basicmodul->m_memorylistwidget->insidewidget();
    m_basicmodul->deleteLater();
}

TEST_F(Ut_BasicModul, setKeyPress)
{
    BasicModule *m_basicmodul = new BasicModule;
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_1, Qt::NoModifier);
    m_basicmodul->setKeyPress(k);
    EXPECT_EQ(m_basicmodul->findChild<InputEdit *>()->text(), "1");
    delete k;
    m_basicmodul->deleteLater();
}

TEST_F(Ut_BasicModul, mAvailableEvent)
{
    BasicModule *m_basicmodul = new BasicModule;
    m_basicmodul->mAvailableEvent();
    EXPECT_TRUE(m_basicmodul->m_memRCbtn);
    EXPECT_TRUE(m_basicmodul->m_avail);
    m_basicmodul->deleteLater();
}

TEST_F(Ut_BasicModul, mUnAvailableEvent)
{
    BasicModule *m_basicmodul = new BasicModule;
    m_basicmodul->mUnAvailableEvent();
    EXPECT_FALSE(m_basicmodul->m_memRCbtn);
    EXPECT_FALSE(m_basicmodul->m_avail);
    m_basicmodul->deleteLater();
}

TEST_F(Ut_BasicModul, showListWidget)
{
    BasicModule *m_basicmodul = new BasicModule;
    m_basicmodul->showListWidget();
    EXPECT_EQ(m_basicmodul->m_keypadLayout->currentIndex(), 1);
    EXPECT_TRUE(m_basicmodul->m_isallgray);
    m_basicmodul->deleteLater();
}

TEST_F(Ut_BasicModul, initTheme)
{
    BasicModule *m_basicmodul = new BasicModule;
    m_basicmodul->initTheme(1);
    EXPECT_EQ(m_basicmodul->findChild<InputEdit *>()->palette().color(QPalette::ColorGroup::Active, QPalette::ColorRole::Text), "#303030");
    m_basicmodul->deleteLater();
}

TEST_F(Ut_BasicModul, handleEditKeyPress)
{
    BasicModule *m_basicmodul = new BasicModule;
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_0, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_1, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_2, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_3, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_4, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_5, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_6, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_7, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_8, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_9, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Plus, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Minus, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Underscore, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Asterisk, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Slash, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Enter, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Return, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Backspace, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Period, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Escape, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_ParenLeft, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_ParenRight, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Percent, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_C, Qt::ControlModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_V, Qt::ControlModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_A, Qt::ControlModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_X, Qt::ControlModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_X, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Delete, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Left, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Right, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Z, Qt::ControlModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Y, Qt::ControlModifier);
    m_basicmodul->m_memRCbtn = true;
    m_basicmodul->m_isallgray = false;
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_L, Qt::ControlModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_R, Qt::ControlModifier);
    m_basicmodul->m_memCalbtn = true;
    m_basicmodul->findChild<InputEdit *>()->setText("1");
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_P, Qt::ControlModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Q, Qt::ControlModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_M, Qt::ControlModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_Escape, Qt::NoModifier);
    QTest::keyClick(m_basicmodul->findChild<InputEdit *>(), Qt::Key_1, Qt::NoModifier);
    EXPECT_EQ(m_basicmodul->findChild<InputEdit *>()->text(), "1");
    m_basicmodul->deleteLater();
}

TEST_F(Ut_BasicModul, handleKeypadButtonPress)
{
    BasicModule *m_basicmodul = new BasicModule;
//    QTest::mouseClick(m_basicmodul->findChild<BasicKeypad *>()->button(BasicKeypad::Key_Clear), Qt::LeftButton);
//    QTest::mouseClick(m_basicmodul->findChild<BasicKeypad *>()->button(BasicKeypad::Key_1), Qt::LeftButton);
//    QTest::mouseClick(m_basicmodul->findChild<BasicKeypad *>()->button(BasicKeypad::Key_Plus), Qt::LeftButton);
//    QTest::mouseClick(m_basicmodul->findChild<BasicKeypad *>()->button(BasicKeypad::Key_3), Qt::LeftButton);
//    QTest::mouseClick(m_basicmodul->findChild<BasicKeypad *>()->button(BasicKeypad::Key_Equals), Qt::LeftButton);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_0);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_1);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_2);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_3);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_4);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_5);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_6);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_7);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_8);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_9);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_Plus);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_Min);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_Mult);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_Div);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_Percent);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_Equals);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_Clear);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_Backspace);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_Point);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_Brackets);
    m_basicmodul->handleKeypadButtonPress(MemoryKeypad::Key_MS);
    m_basicmodul->handleKeypadButtonPress(MemoryKeypad::Key_MC);
    m_basicmodul->handleKeypadButtonPress(MemoryKeypad::Key_Mlist);
    m_basicmodul->handleKeypadButtonPress(MemoryKeypad::Key_Mplus);
    m_basicmodul->handleKeypadButtonPress(MemoryKeypad::Key_Mminus);
    m_basicmodul->handleKeypadButtonPress(MemoryKeypad::Key_MR);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_Clear);
    m_basicmodul->handleKeypadButtonPress(BasicKeypad::Key_1);
    EXPECT_EQ(m_basicmodul->findChild<InputEdit *>()->text(), "1");
    m_basicmodul->deleteLater();
}

TEST_F(Ut_BasicModul, handleKeypadButtonPressByspace)
{
    BasicModule *m_basicmodul = new BasicModule;
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_0);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_1);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_2);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_3);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_4);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_5);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_6);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_7);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_8);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_9);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_Plus);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_Min);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_Mult);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_Div);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_Percent);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_Equals);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_Clear);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_Backspace);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_Point);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_Brackets);
    m_basicmodul->handleKeypadButtonPressByspace(MemoryKeypad::Key_MS);
    m_basicmodul->handleKeypadButtonPressByspace(MemoryKeypad::Key_MC);
    m_basicmodul->handleKeypadButtonPressByspace(MemoryKeypad::Key_Mlist);
    m_basicmodul->handleKeypadButtonPressByspace(MemoryKeypad::Key_Mplus);
    m_basicmodul->handleKeypadButtonPressByspace(MemoryKeypad::Key_Mminus);
    m_basicmodul->handleKeypadButtonPressByspace(MemoryKeypad::Key_MR);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_Clear);
    m_basicmodul->handleKeypadButtonPressByspace(BasicKeypad::Key_1);
    EXPECT_EQ(m_basicmodul->findChild<InputEdit *>()->text(), "1");
    m_basicmodul->deleteLater();
}

TEST_F(Ut_BasicModul, mousePressEvent)
{
    BasicModule *m_basicmodul = new BasicModule;
    QTest::mouseClick(m_basicmodul->findChild<BasicKeypad *>()->button(BasicKeypad::Key_1), Qt::LeftButton);
    QTest::mouseClick(m_basicmodul->findChild<MemoryKeypad *>()->button(MemoryKeypad::Key_MS), Qt::LeftButton);
    QTest::mouseClick(m_basicmodul->findChild<MemoryKeypad *>()->button(MemoryKeypad::Key_Mlist), Qt::LeftButton);
    QMouseEvent *m = new QMouseEvent(QEvent::MouseButtonPress, m_basicmodul->findChild<InputEdit *>()->pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_basicmodul->mousePressEvent(m);
    EXPECT_EQ(m_basicmodul->m_keypadLayout->currentIndex(), 0);
    QTest::mouseClick(m_basicmodul->findChild<MemoryKeypad *>()->button(MemoryKeypad::Key_Mlist), Qt::LeftButton);
    QTest::mouseClick(m_basicmodul->findChild<MemoryWidget *>()->findChild<IconButton *>(), Qt::LeftButton);
    m_basicmodul->mousePressEvent(m);
    EXPECT_EQ(m_basicmodul->m_keypadLayout->currentIndex(), 0);
    delete m;
    m_basicmodul->deleteLater();
}

TEST_F(Ut_BasicModul, itemclick)
{
    BasicModule *m_basicmodul = new BasicModule;
    QTest::mouseClick(m_basicmodul->findChild<BasicKeypad *>()->button(BasicKeypad::Key_1), Qt::LeftButton);
    QTest::mouseClick(m_basicmodul->findChild<MemoryKeypad *>()->button(MemoryKeypad::Key_MS), Qt::LeftButton);
    QTest::mouseClick(m_basicmodul->findChild<MemoryKeypad *>()->button(MemoryKeypad::Key_Mlist), Qt::LeftButton);
    QTest::mouseClick(m_basicmodul->findChild<MemoryItemWidget *>(), Qt::LeftButton);
    EXPECT_EQ(m_basicmodul->m_keypadLayout->currentIndex(), 0);
    m_basicmodul->deleteLater();
}
