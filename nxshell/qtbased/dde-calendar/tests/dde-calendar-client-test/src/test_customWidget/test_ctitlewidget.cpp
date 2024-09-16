// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

//#include "test_ctitlewidget.h"

//test_ctitlewidget::test_ctitlewidget()
//{

//}

//TEST_F(test_ctitlewidget, setShowState_001)
//{
//    mWidget->setShowState(CTitleWidget::Title_State_Mini);
//    EXPECT_EQ(mWidget->m_showState, CTitleWidget::Title_State_Mini);
//}

//TEST_F(test_ctitlewidget, setShowState_002)
//{
//    mWidget->setShowState(CTitleWidget::Title_State_Normal);
//    EXPECT_EQ(mWidget->m_showState, CTitleWidget::Title_State_Normal);
//}

//TEST_F(test_ctitlewidget, buttonBox_001)
//{
//    EXPECT_EQ(mWidget->buttonBox(), mWidget->m_buttonBox);
//}

//TEST_F(test_ctitlewidget, searchEdit_001)
//{
//    EXPECT_EQ(mWidget->searchEdit(), mWidget->m_searchEdit);
//}

//TEST_F(test_ctitlewidget, stateUpdate_001)
//{
//    mWidget->setShowState(CTitleWidget::Title_State_Mini);
//    mWidget->stateUpdate();
//}

//TEST_F(test_ctitlewidget, stateUpdate_002)
//{
//    mWidget->setShowState(CTitleWidget::Title_State_Normal);
//    mWidget->stateUpdate();
//}

//TEST_F(test_ctitlewidget, miniStateShowSearchEdit_001)
//{
//    mWidget->miniStateShowSearchEdit();
//    EXPECT_EQ(mWidget->width(), mWidget->m_searchEdit->maximumWidth());
//}

//TEST_F(test_ctitlewidget, normalStateUpdateSearchEditWidth_001)
//{
//    mWidget->normalStateUpdateSearchEditWidth();
//    EXPECT_NE(mWidget->width(), mWidget->m_searchEdit->maximumWidth());
//}

//TEST_F(test_ctitlewidget, eventFilter_001)
//{
//    DButtonBoxButton btn("");
//    QKeyEvent e(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
//    mWidget->eventFilter(&btn, &e);
//    EXPECT_NE(mWidget->width(), mWidget->m_searchEdit->maximumWidth());
//}

//TEST_F(test_ctitlewidget, eventFilter_002)
//{
//    QFocusEvent e(QEvent::FocusOut, Qt::TabFocusReason);
//    mWidget->eventFilter(mWidget->m_searchEdit, &e);
//    EXPECT_NE(mWidget->width(), mWidget->m_searchEdit->maximumWidth());
//}

//TEST_F(test_ctitlewidget, slotShowSearchEdit_001)
//{
//    mWidget->slotShowSearchEdit();
//}

//TEST_F(test_ctitlewidget, slotSearchEditFocusChanged_001)
//{
//    mWidget->slotSearchEditFocusChanged(true);
//}

//TEST_F(test_ctitlewidget, slotSearchEditFocusChanged_002)
//{
//    mWidget->setShowState(CTitleWidget::Title_State_Mini);
//    mWidget->slotSearchEditFocusChanged(false);
//}
