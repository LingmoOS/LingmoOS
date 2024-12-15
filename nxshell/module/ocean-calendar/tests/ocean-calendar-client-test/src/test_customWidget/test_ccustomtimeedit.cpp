// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_ccustomtimeedit.h"

#include <QTest>
#include <QFocusEvent>

void test_CCustomTimeEdit::SetUp()
{
    m_timeEdit = new CCustomTimeEdit();
}

void test_CCustomTimeEdit::TearDown()
{
    delete m_timeEdit;
    m_timeEdit = nullptr;
}

//mouseEventTest
TEST_F(test_CCustomTimeEdit, mouseEventTest)
{
    QWidget *widget = new QWidget();
    m_timeEdit->setParent(widget);
    QTest::mousePress(m_timeEdit, Qt::LeftButton);
    QTest::mouseRelease(m_timeEdit, Qt::LeftButton);
    QTest::mouseDClick(m_timeEdit, Qt::LeftButton);
    QTest::mouseRelease(m_timeEdit, Qt::LeftButton);

    m_timeEdit->setParent(nullptr);
    delete  widget;
}

TEST_F(test_CCustomTimeEdit,focusInEvent)
{
    QFocusEvent focusEvent_in(QEvent::FocusIn,Qt::FocusReason::ActiveWindowFocusReason);
    QApplication::sendEvent(m_timeEdit,&focusEvent_in);
}

TEST_F(test_CCustomTimeEdit,focusOutEvent)
{
    QFocusEvent focusEvent_out( QEvent::FocusOut,Qt::FocusReason::TabFocusReason);
    QApplication::sendEvent(m_timeEdit,&focusEvent_out);
}

TEST_F(test_CCustomTimeEdit,keyPressEvent)
{
    QWidget *widget = new QWidget();
    m_timeEdit->setParent(widget);
    QKeyEvent keyevent(QEvent::KeyPress,Qt::Key_Left,Qt::NoModifier);
    QApplication::sendEvent(m_timeEdit,&keyevent);
    m_timeEdit->setParent(nullptr);
    delete  widget;
}
