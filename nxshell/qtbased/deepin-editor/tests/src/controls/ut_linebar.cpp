// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_linebar.h"
#include "../../src/controls/linebar.h"
#include <QFocusEvent>
#include <QEvent>

test_linebar::test_linebar()
{

}

TEST_F(test_linebar, LineBar)
{
    LineBar lineBar(nullptr);
    
}

//public slots:
//    void handleTextChangeTimer();
TEST_F(test_linebar, handleTextChangeTimer)
{
    LineBar *lineBar = new LineBar();
    lineBar->handleTextChangeTimer();


     EXPECT_NE(lineBar,nullptr);

    lineBar->deleteLater();
    
}

//    void handleTextChanged();
TEST_F(test_linebar, handleTextChanged)
{
    LineBar *lineBar = new LineBar();
    lineBar->m_autoSaveTimer->start();

    lineBar->handleTextChanged();

    lineBar->m_autoSaveTimer->stop();

    EXPECT_EQ(lineBar->m_autoSaveTimer->isActive(),false);
    EXPECT_NE(lineBar,nullptr);

   lineBar->deleteLater();
}

//    void sendText(QString t);
TEST_F(test_linebar, sendText)
{
    LineBar *lineBar = new LineBar();
    lineBar->sendText("aa");
    
    EXPECT_NE(lineBar,nullptr);

    lineBar->deleteLater();
}

//protected:
//    virtual void focusOutEvent(QFocusEvent *e);
TEST_F(test_linebar, focusOutEvent)
{
    LineBar *lineBar = new LineBar();
    QFocusEvent *e = new QFocusEvent(QEvent::FocusIn);
    lineBar->focusOutEvent(e);


    EXPECT_NE(lineBar,nullptr);

    lineBar->deleteLater();
    delete e;e=nullptr;
    
}

//    virtual void keyPressEvent(QKeyEvent *e);
TEST_F(test_linebar, keyPressEvent)
{
    LineBar *lineBar = new LineBar();
    Qt::KeyboardModifier modefiers[4] = {Qt::ControlModifier,Qt::AltModifier,Qt::MetaModifier,Qt::NoModifier};

    EXPECT_NE(lineBar,nullptr);

    QKeyEvent *e = new QKeyEvent(QEvent::KeyPress,1,modefiers[0],"\r");
    lineBar->keyPressEvent(e);
    delete e;e=nullptr;


    e = new QKeyEvent(QEvent::KeyPress,1,modefiers[1],"\r");
    lineBar->keyPressEvent(e);
    delete e;e=nullptr;

    e = new QKeyEvent(QEvent::KeyPress,1,modefiers[2],"\r");
    lineBar->keyPressEvent(e);
    delete e;e=nullptr;

    e = new QKeyEvent(QEvent::KeyPress,1,modefiers[3],"\r");
    lineBar->keyPressEvent(e);
    delete e;e=nullptr;


    lineBar->deleteLater();
}
