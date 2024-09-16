// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_jumplinebar.h"
#include "../../src/controls/jumplinebar.h"

test_jumplinebar::test_jumplinebar()
{

}

TEST_F(test_jumplinebar, JumpLineBar)
{
    JumpLineBar jumpLineBar(nullptr);
    
}

//void focus();
TEST_F(test_jumplinebar, focus)
{
    JumpLineBar *jumpLineBar = new JumpLineBar();
    jumpLineBar->focus();
    
    EXPECT_NE(jumpLineBar,nullptr);

    delete jumpLineBar;jumpLineBar=nullptr;
}

//bool isFocus();
TEST_F(test_jumplinebar, isFocus)
{
    JumpLineBar *jumpLineBar = new JumpLineBar();
    jumpLineBar->isFocus();
    
    EXPECT_NE(jumpLineBar,nullptr);
    EXPECT_NE(jumpLineBar->isFocus(),true);

    delete jumpLineBar;jumpLineBar=nullptr;
}

//void activeInput(QString file, int row, int column, int lineCount, int scrollOffset);
TEST_F(test_jumplinebar, activeInput)
{
    JumpLineBar *jumpLineBar = new JumpLineBar();
    jumpLineBar->m_pSpinBoxInput->lineEdit()->setText("50");
    jumpLineBar->activeInput("aa",1,1,1,1);
    
    EXPECT_EQ(jumpLineBar->m_pSpinBoxInput->lineEdit()->text(),"1");
    EXPECT_NE(jumpLineBar,nullptr);

    delete jumpLineBar;jumpLineBar=nullptr;
}

//void handleFocusOut();
TEST_F(test_jumplinebar, handleFocusOut)
{
    JumpLineBar *jumpLineBar = new JumpLineBar();
    jumpLineBar->handleFocusOut();
    

    EXPECT_NE(jumpLineBar,nullptr);
    EXPECT_EQ(jumpLineBar->isVisible(),false);

    delete jumpLineBar;jumpLineBar=nullptr;
}

//void handleLineChanged();
TEST_F(test_jumplinebar, handleLineChanged)
{
    JumpLineBar *jumpLineBar = new JumpLineBar();
    jumpLineBar->m_pSpinBoxInput->lineEdit()->setText("123");
    jumpLineBar->handleLineChanged();
    
    EXPECT_NE(jumpLineBar,nullptr);


    delete jumpLineBar;jumpLineBar=nullptr;
}

//void jumpCancel();
TEST_F(test_jumplinebar, jumpCancel)
{
    JumpLineBar *jumpLineBar = new JumpLineBar();
    jumpLineBar->jumpCancel();
    
    EXPECT_NE(jumpLineBar,nullptr);
    EXPECT_EQ(jumpLineBar->isVisible(),false);

    delete jumpLineBar;jumpLineBar=nullptr;
}

//void jumpConfirm();
TEST_F(test_jumplinebar, jumpConfirm)
{
    JumpLineBar *jumpLineBar = new JumpLineBar();
    jumpLineBar->jumpConfirm();
    jumpLineBar->m_pSpinBoxInput->lineEdit()->setText("123");

    EXPECT_NE(jumpLineBar,nullptr);

    delete jumpLineBar;jumpLineBar=nullptr;
}

//void slotFocusChanged(bool bFocus);
TEST_F(test_jumplinebar, slotFocusChanged)
{
    JumpLineBar *jumpLineBar = new JumpLineBar();
    jumpLineBar->slotFocusChanged(false);
    
    EXPECT_NE(jumpLineBar,nullptr);

    delete jumpLineBar;jumpLineBar=nullptr;
}
