// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_titlewidget.h"

#include <QTest>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QFocusEvent>

test_titleWidget::test_titleWidget()
{

}

void test_titleWidget::SetUp()
{
    m_titleWidget = new CTitleWidget();
}

void test_titleWidget::TearDown()
{
    delete m_titleWidget;
    m_titleWidget = nullptr;
}


TEST_F(test_titleWidget,resizeEvent)
{
    m_titleWidget->setFixedSize(600,30);
    QResizeEvent resizeEvent(QSize(500,30),QSize(600,30));
    QApplication::sendEvent(m_titleWidget, &resizeEvent);

    m_titleWidget->setFixedSize(1900,30);
    QResizeEvent resizeEvent1(QSize(1920,30),QSize(1900,30));
    QApplication::sendEvent(m_titleWidget, &resizeEvent1);
}

TEST_F(test_titleWidget,eventFilter)
{
    DButtonBoxButton *boxbtn = m_titleWidget->findChild<DButtonBoxButton *>();
    if(boxbtn){
        QKeyEvent keyevent(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);
        QApplication::sendEvent(boxbtn,&keyevent);
    }

    DSearchEdit *edit = m_titleWidget->findChild<DSearchEdit *>();
    if(edit){
        QFocusEvent focusEvent_in( QEvent::FocusIn,Qt::FocusReason::TabFocusReason);
        QApplication::sendEvent(edit->lineEdit(),&focusEvent_in);
        QFocusEvent focusEvent_out( QEvent::FocusOut,Qt::FocusReason::TabFocusReason);
        QApplication::sendEvent(edit->lineEdit(),&focusEvent_out);
    }

    DButtonBox *buttonBox = m_titleWidget->findChild<DButtonBox *>();
    if(buttonBox){
        buttonBox->button(0)->setFocus();
        buttonBox->button(0)->setChecked(true);
        QFocusEvent focusEvent_in(QEvent::FocusIn,Qt::FocusReason::ActiveWindowFocusReason);
        QApplication::sendEvent(buttonBox,&focusEvent_in);
    }
}