// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_replacebar.h"
#include <QKeyEvent>
test_replacebar::test_replacebar()
{

}
//bool isFocus();
TEST_F(test_replacebar, isFocus)
{
    ReplaceBar * rep = new ReplaceBar();
    EXPECT_EQ(rep->isFocus(),false);
    rep->handleSkip();

    EXPECT_NE(rep,nullptr);
    rep->deleteLater();
}
//void focus();
TEST_F(test_replacebar, focus)
{
    ReplaceBar * rep = new ReplaceBar();
    EXPECT_EQ(rep->isFocus(),false);
    rep->focus();

    EXPECT_NE(rep,nullptr);
    rep->deleteLater();

    
}

//void activeInput(QString text, QString file, int row, int column, int scrollOffset);
TEST_F(test_replacebar, activeInput)
{
    ReplaceBar * rep = new ReplaceBar();
    rep->activeInput("aa","bb",2,2,2);
    EXPECT_EQ(rep->isVisible(),true);

    EXPECT_NE(rep,nullptr);
    rep->deleteLater();

    
}
//void setMismatchAlert(bool isAlert);
TEST_F(test_replacebar, setMismatchAlert)
{
    ReplaceBar * rep = new ReplaceBar();
    rep->setMismatchAlert(true);
    EXPECT_EQ(rep->m_replaceLine->isAlert(),true);

    EXPECT_NE(rep,nullptr);
    rep->deleteLater();

    
}
//void setsearched(bool _);
TEST_F(test_replacebar, setsearched)
{
    ReplaceBar * rep = new ReplaceBar();
    rep->setsearched(true);
    EXPECT_EQ(rep->searched,true);

    EXPECT_NE(rep,nullptr);
    rep->deleteLater();

    
}
//    void change();
TEST_F(test_replacebar, change)
{
    ReplaceBar * rep = new ReplaceBar();
    rep->change();
    EXPECT_EQ(rep->searched,false);


    EXPECT_NE(rep,nullptr);
    rep->deleteLater();

    
}
//void replaceClose();
TEST_F(test_replacebar, replaceClose)
{
    ReplaceBar * rep = new ReplaceBar();
    rep->replaceClose();
    EXPECT_EQ(rep->isVisible(),false);

    EXPECT_NE(rep,nullptr);
    rep->deleteLater();

    
}
//void handleContentChanged();
TEST_F(test_replacebar, handleContentChanged)
{
    ReplaceBar * rep = new ReplaceBar();
    rep->handleContentChanged();

    EXPECT_NE(rep,nullptr);
    rep->deleteLater();

    
}
//void handleReplaceAll();
TEST_F(test_replacebar, handleReplaceAll)
{
    ReplaceBar * rep = new ReplaceBar();
    rep->handleReplaceAll();

    EXPECT_NE(rep,nullptr);
    rep->deleteLater();

    
}
//void handleReplaceNext();
TEST_F(test_replacebar, handleReplaceNext)
{
    ReplaceBar * rep = new ReplaceBar();
    rep->handleReplaceNext();
    EXPECT_EQ(rep->searched,true);

    EXPECT_NE(rep,nullptr);
    rep->deleteLater();

    
}
//void handleReplaceRest();
TEST_F(test_replacebar, handleReplaceRest)
{
    ReplaceBar * rep = new ReplaceBar();
    rep->handleReplaceRest();

    EXPECT_NE(rep,nullptr);
    rep->deleteLater();

    
}

//protected:
//void hideEvent(QHideEvent *event);
TEST_F(test_replacebar, hideEvent)
{
    ReplaceBar * rep = new ReplaceBar();
    QHideEvent*e;
    rep->hideEvent(e);
    EXPECT_EQ(rep->searched,false);

    EXPECT_NE(rep,nullptr);
    rep->deleteLater();

    
}
//bool focusNextPrevChild(bool next);
//void keyPressEvent(QKeyEvent *e);

TEST_F(test_replacebar, focusNextPrevChild)
{
    ReplaceBar * rep = new ReplaceBar();
    EXPECT_NE(rep->focusNextPrevChild(true),true);

    EXPECT_NE(rep,nullptr);
    rep->deleteLater();

    
}

TEST_F(test_replacebar, keyPressEvent)
{
    ReplaceBar * rep = new ReplaceBar();
    QKeyEvent * e3 = new QKeyEvent(QEvent::KeyPress,Qt::Key_Excel,Qt::NoModifier);
    rep->keyPressEvent(e3);
    delete e3;e3=nullptr;

    QKeyEvent *e = new QKeyEvent(QEvent::KeyPress,Qt::Key_Tab,Qt::NoModifier);
    rep->m_closeButton->setFocus();
    rep->keyPressEvent(e);
    delete e;e=nullptr;

    QKeyEvent *e1 = new QKeyEvent(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);
    rep->keyPressEvent(e1);
    EXPECT_EQ(rep->m_replaceLine->lineEdit()->hasFocus(),false);
    delete e1;e1=nullptr;

    QKeyEvent *e2 = new QKeyEvent(QEvent::KeyPress,Qt::Key_Enter,Qt::NoModifier);
    rep->m_replaceSkipButton->setFocus();
    rep->m_replaceButton->setFocus();
    rep->m_replaceAllButton->setFocus();
    rep->m_replaceRestButton->setFocus();
    rep->keyPressEvent(e2);

    EXPECT_EQ(rep->m_replaceRestButton->hasFocus(),false);
    delete e2; e2=nullptr;


    EXPECT_NE(rep,nullptr);
    rep->deleteLater();

    
}
