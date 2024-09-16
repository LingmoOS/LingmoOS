// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_todaybutton.h"

#include <QTest>
#include <QFocusEvent>
#include <QEnterEvent>
//#include <QLeaveEvent>

test_todaybutton::test_todaybutton()
{

}

void test_todaybutton::SetUp()
{
    mTodayButton = new CTodayButton();
}

void test_todaybutton::TearDown()
{
    delete mTodayButton;
    mTodayButton = nullptr;
}

//void CTodayButton::setBColor(QColor normalC, QColor hoverC, QColor pressc, QColor normalC1, QColor hoverC1, QColor pressc1)
TEST_F(test_todaybutton, setBColor)
{
    mTodayButton->setBColor("#FFFFFF", "#000000", "#000000", "#FFFFFF", "#000000", "#000000");
}

//void CTodayButton::setTColor(QColor normalC, QColor hoverC, QColor pressc)
TEST_F(test_todaybutton, setTColor)
{
    mTodayButton->setTColor(Qt::red, "#001A2E", "#0081FF");
}

//void CTodayButton::setshadowColor(QColor sc)
TEST_F(test_todaybutton, setshadowColor)
{
    mTodayButton->setshadowColor("#FFFFFF");
}

//test mouse event
TEST_F(test_todaybutton, mouseEventTest)
{
    QTest::mousePress(mTodayButton, Qt::LeftButton);
    QTest::mouseRelease(mTodayButton, Qt::LeftButton);

    QWidget *testWidget = new QWidget();
    CTodayButton *toDayButton = new CTodayButton(testWidget);
    toDayButton->setGeometry(10, 10, 20, 20);
    testWidget->setFixedSize(50, 50);
    toDayButton->setFocus(Qt::TabFocusReason);
    QTest::mouseMove(testWidget, QPoint(2, 2));
    QTest::mouseMove(testWidget, QPoint(15, 15));
    QTest::mouseMove(testWidget, QPoint(45, 45));
    QTest::keyClick(testWidget, Qt::Key_Tab);
    QTest::keyClick(testWidget->focusWidget(), Qt::Key_Tab);
    QTest::keyClick(testWidget->focusWidget(), Qt::Key_Tab);
    delete testWidget;
}

//test key event
TEST_F(test_todaybutton, keyEventTest)
{
    QTest::keyClick(mTodayButton, Qt::Key_Enter);
    QTest::keyClick(mTodayButton, Qt::Key_Tab);
}

//QTEST_MAIN(testGUI_toDayButton)
TEST_F(test_todaybutton,focusOutEvent)
{
    mTodayButton->setFocus();
    QFocusEvent focusEvent_out( QEvent::FocusOut,Qt::FocusReason::TabFocusReason);
    QApplication::sendEvent(mTodayButton,&focusEvent_out);
}

TEST_F(test_todaybutton,enterEvent)
{
    QEnterEvent enterEvent(QPointF(10,2),QPointF(11,3),QPointF(12,4));
    QApplication::sendEvent(mTodayButton,&enterEvent);
    QEvent event(QEvent::Leave);
    QApplication::sendEvent(mTodayButton,&event);
}

TEST_F(test_todaybutton,keypressEvent)
{
    QKeyEvent keyevent(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);
    QApplication::sendEvent(mTodayButton,&keyevent);
}
