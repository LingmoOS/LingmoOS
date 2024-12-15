// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "animation_stackedwidget.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QPainter>

/***************************************STUB begin*********************************************/
QWidget* stub_animationFinished_widget(int index)
{
    static QWidget wid;
    return &wid;
}
bool stub_animationFinished_show()
{
    return false;
}

int stub_setCurrent_index_value0()
{
    //参数为0
    return 0;
}

int stub_setCurrent_index_valueN1()
{
    //参数为-1
    return -1;
}

AnimationStackedWidget::MoveOrientation stub_setCurrent_getMoveOrientation(void* obj, const int currIndex, const int nextIndex)
{
    return AnimationStackedWidget::LeftToRight;
}

int stub_setCurrent_setCurrentWidget(void* obj, int &index, int beginWidth)
{
    return 0;
}

int stub_setCurrentWidget_currentIndex()
{
    int index = 0;
    return index;
}

QStackedWidget* stub_setCurrentWidget_widget()
{
    static QStackedWidget *widget = new QStackedWidget;
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, widget, &QStackedWidget::deleteLater);
    return widget;
}

QStackedWidget* stub_paintEvent_widget()
{
    static QStackedWidget *widget = new QStackedWidget;
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, widget, &QStackedWidget::deleteLater);
    return widget;
}

/***************************************STUB end**********************************************/


class UT_AnimationStackedWidget : public ::testing::Test
{
public:
    UT_AnimationStackedWidget() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new AnimationStackedWidget();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    AnimationStackedWidget *m_tester;
};

ACCESS_PRIVATE_FUN(AnimationStackedWidget, AnimationStackedWidget::MoveOrientation(const int, const int), getMoveOrientation);
ACCESS_PRIVATE_FUN(AnimationStackedWidget, void(int&, int), setCurrentWidget);

TEST_F(UT_AnimationStackedWidget, initTest)
{

}

TEST_F(UT_AnimationStackedWidget, test_setDuration_01)
{
    m_tester->setDuration(500);

    EXPECT_EQ(m_tester->m_Duration, 500);
    EXPECT_EQ(m_tester->m_Animation->easingCurve(), QEasingCurve::InOutQuad);
}

TEST_F(UT_AnimationStackedWidget, test_paintEvent_01)
{
    Stub stub;
    stub.set(ADDR(QStackedWidget, widget), stub_paintEvent_widget);
    m_tester->m_IsAnimation = true;
    EXPECT_FALSE(m_tester->grab().isNull());
}

TEST_F(UT_AnimationStackedWidget, test_animationFinished_01)
{
    Stub stub;
    stub.set(ADDR(AnimationStackedWidget, widget), stub_animationFinished_widget);
    stub.set(ADDR(QWidget, show), stub_animationFinished_show);
    m_tester->animationFinished();

    EXPECT_FALSE(m_tester->m_IsAnimation);

    QSignalSpy signalSpy(m_tester, &AnimationStackedWidget::signalIsFinished);
    EXPECT_TRUE(signalSpy.count() == 0);

    emit m_tester->signalIsFinished();
    signalSpy.wait(50);
    EXPECT_TRUE(signalSpy.count() == 1);
}

TEST_F(UT_AnimationStackedWidget, test_setCurrent_01)
{
    int idx = 1;
    auto getMoveOrientationPrivate = get_private_fun::AnimationStackedWidgetgetMoveOrientation();
    auto setCurrentWidget = get_private_fun::AnimationStackedWidgetsetCurrentWidget();
    Stub stub;
    stub.set(ADDR(QStackedWidget, indexOf), stub_setCurrent_index_value0);
    stub.set(getMoveOrientationPrivate, stub_setCurrent_getMoveOrientation);
    stub.set(setCurrentWidget, stub_setCurrent_setCurrentWidget);


    call_private_fun::AnimationStackedWidgetgetMoveOrientation(*m_tester, 0, 1);
    int index = 0;
    call_private_fun::AnimationStackedWidgetsetCurrentWidget(*m_tester, index, 1);
    m_tester->setCurrent(idx);

    EXPECT_EQ(m_tester->m_moveOri, AnimationStackedWidget::LeftToRight);
}

TEST_F(UT_AnimationStackedWidget, test_setCurrent_02)
{
    int idx = 1;
    auto getMoveOrientationPrivate = get_private_fun::AnimationStackedWidgetgetMoveOrientation();
    auto setCurrentWidget = get_private_fun::AnimationStackedWidgetsetCurrentWidget();
    Stub stub;
    stub.set(ADDR(QStackedWidget, indexOf), stub_setCurrent_index_valueN1);
    stub.set(getMoveOrientationPrivate, stub_setCurrent_getMoveOrientation);
    stub.set(setCurrentWidget, stub_setCurrent_setCurrentWidget);


    call_private_fun::AnimationStackedWidgetgetMoveOrientation(*m_tester, 0, 1);
    int index = 0;
    call_private_fun::AnimationStackedWidgetsetCurrentWidget(*m_tester, index, 1);
    m_tester->setCurrent(idx);

    EXPECT_EQ(m_tester->m_moveOri, AnimationStackedWidget::LeftToRight);
}

TEST_F(UT_AnimationStackedWidget, test_setCurrent_03)
{
    QWidget *wid = new QWidget;

    auto setCurrentWidget = get_private_fun::AnimationStackedWidgetsetCurrentWidget();
    auto getMoveOrientationPrivate = get_private_fun::AnimationStackedWidgetgetMoveOrientation();
    Stub stub;
    stub.set(ADDR(QStackedWidget, indexOf), stub_setCurrent_index_value0);
    stub.set(getMoveOrientationPrivate, stub_setCurrent_getMoveOrientation);
    stub.set(setCurrentWidget, stub_setCurrent_setCurrentWidget);

    int index = 0;
    call_private_fun::AnimationStackedWidgetsetCurrentWidget(*m_tester, index, 1);
    m_tester->setCurrent(wid);

    EXPECT_EQ(m_tester->m_moveOri, AnimationStackedWidget::LeftToRight);

    wid->deleteLater();
}

TEST_F(UT_AnimationStackedWidget, test_setCurrent_04)
{
    QWidget *wid = new QWidget;
    Stub stub;
    stub.set(ADDR(QStackedWidget, indexOf), stub_setCurrent_index_valueN1);

    m_tester->setCurrent(wid);

    wid->deleteLater();
}

TEST_F(UT_AnimationStackedWidget, test_IsRunning_01)
{
    m_tester->m_IsAnimation = false;
    m_tester->IsRunning();

    EXPECT_FALSE(m_tester->m_IsAnimation);
}

TEST_F(UT_AnimationStackedWidget, test_IsRunning_02)
{
    m_tester->m_IsAnimation = true;
    m_tester->IsRunning();

    EXPECT_TRUE(m_tester->m_IsAnimation);
}

TEST_F(UT_AnimationStackedWidget, test_offset_01)
{
    double value = 1.0;
    m_tester->m_offset = value;
    m_tester->offset();

    EXPECT_EQ(m_tester->m_offset, value);
}

TEST_F(UT_AnimationStackedWidget, test_setOffset_01)
{
    double value = 1.0;
    m_tester->setOffset(value);

    EXPECT_EQ(m_tester->m_offset, value);
}

TEST_F(UT_AnimationStackedWidget, test_setCurrentWidget_01)
{
    int index = 0;
    m_tester->m_IsAnimation = true;
    m_tester->setCurrentWidget(index, 0);

    EXPECT_TRUE(m_tester->m_IsAnimation);
}

TEST_F(UT_AnimationStackedWidget, test_setCurrentWidget_02)
{
    int index = 0;
    Stub stub;
    stub.set(ADDR(QStackedWidget, currentIndex), stub_setCurrentWidget_currentIndex);

    m_tester->m_IsAnimation = false;
    m_tester->setCurrentWidget(index, 0);

    EXPECT_FALSE(m_tester->m_IsAnimation);
    QSignalSpy signalSpy(m_tester, &AnimationStackedWidget::signalIsFinished);
    EXPECT_TRUE(signalSpy.count() == 0);

    emit m_tester->signalIsFinished();
    signalSpy.wait(50);
    EXPECT_TRUE(signalSpy.count() == 1);
}

TEST_F(UT_AnimationStackedWidget, test_setCurrentWidget_03)
{
    int index = 1;
    Stub stub;
    stub.set(ADDR(QStackedWidget, currentIndex), stub_setCurrentWidget_currentIndex);

    stub.set(ADDR(QStackedWidget, widget), stub_setCurrentWidget_widget);

    m_tester->m_IsAnimation = false;
    m_tester->m_Duration = 0;
    m_tester->setCurrentWidget(index, 0);

    EXPECT_EQ(m_tester->m_NextIndex, index);
    EXPECT_EQ(m_tester->m_Animation->startValue(), 0);
    EXPECT_EQ(m_tester->m_Animation->endValue(), 0);
    EXPECT_EQ(m_tester->m_Animation->duration(), 0);
}

TEST_F(UT_AnimationStackedWidget, test_getBeginValue_01)
{
    m_tester->m_animationOri = AnimationStackedWidget::AnimationOri::LR;
    int value = m_tester->getBeginValue();
    EXPECT_EQ(value, m_tester->rect().width());
}

TEST_F(UT_AnimationStackedWidget, test_getBeginValue_02)
{
    m_tester->m_animationOri = AnimationStackedWidget::AnimationOri::TB;
    int value = m_tester->getBeginValue();
    EXPECT_EQ(value, m_tester->rect().height());
}

TEST_F(UT_AnimationStackedWidget, test_getMoveOrientation_01)
{
    m_tester->m_animationOri = AnimationStackedWidget::AnimationOri::LR;
    AnimationStackedWidget::MoveOrientation moveOri = m_tester->getMoveOrientation(1, 0);

    EXPECT_EQ(moveOri, AnimationStackedWidget::LeftToRight);
}

TEST_F(UT_AnimationStackedWidget, test_getMoveOrientation_02)
{
    m_tester->m_animationOri = AnimationStackedWidget::AnimationOri::TB;
    AnimationStackedWidget::MoveOrientation moveOri = m_tester->getMoveOrientation(1, 0);

    EXPECT_EQ(moveOri, AnimationStackedWidget::TopToBottom);
}

TEST_F(UT_AnimationStackedWidget, test_getMoveOrientation_03)
{
    m_tester->m_animationOri = AnimationStackedWidget::AnimationOri::LR;
    AnimationStackedWidget::MoveOrientation moveOri = m_tester->getMoveOrientation(0, 1);

    EXPECT_EQ(moveOri, AnimationStackedWidget::RightToLeft);
}

TEST_F(UT_AnimationStackedWidget, test_getMoveOrientation_04)
{
    m_tester->m_animationOri = AnimationStackedWidget::AnimationOri::TB;
    AnimationStackedWidget::MoveOrientation moveOri = m_tester->getMoveOrientation(0, 1);

    EXPECT_EQ(moveOri, AnimationStackedWidget::BottomToTop);
}

TEST_F(UT_AnimationStackedWidget, test_paintCurrentWidget_01)
{
    m_tester->m_moveOri = AnimationStackedWidget::MoveOrientation::LeftToRight;
    Stub stub;
    stub.set(ADDR(QStackedWidget, widget), stub_paintEvent_widget);
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->paintCurrentWidget(painter, 0);
}


TEST_F(UT_AnimationStackedWidget, test_paintCurrentWidget_02)
{
    m_tester->m_moveOri = AnimationStackedWidget::MoveOrientation::RightToLeft;
    Stub stub;
    stub.set(ADDR(QStackedWidget, widget), stub_paintEvent_widget);
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->paintCurrentWidget(painter, 0);
}

TEST_F(UT_AnimationStackedWidget, test_paintCurrentWidget_03)
{
    m_tester->m_moveOri = AnimationStackedWidget::MoveOrientation::TopToBottom;
    Stub stub;
    stub.set(ADDR(QStackedWidget, widget), stub_paintEvent_widget);
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->paintCurrentWidget(painter, 0);
}

TEST_F(UT_AnimationStackedWidget, test_paintCurrentWidget_04)
{
    m_tester->m_moveOri = AnimationStackedWidget::MoveOrientation::BottomToTop;
    Stub stub;
    stub.set(ADDR(QStackedWidget, widget), stub_paintEvent_widget);
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->paintCurrentWidget(painter, 0);
}

TEST_F(UT_AnimationStackedWidget, test_paintNextWidget_01)
{
    m_tester->m_moveOri = AnimationStackedWidget::MoveOrientation::LeftToRight;
    Stub stub;
    stub.set(ADDR(QStackedWidget, widget), stub_paintEvent_widget);
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->paintNextWidget(painter, 0);
}


TEST_F(UT_AnimationStackedWidget, test_paintNextWidget_02)
{
    m_tester->m_moveOri = AnimationStackedWidget::MoveOrientation::RightToLeft;
    Stub stub;
    stub.set(ADDR(QStackedWidget, widget), stub_paintEvent_widget);
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->paintNextWidget(painter, 0);
}

TEST_F(UT_AnimationStackedWidget, test_paintNextWidget_03)
{
    m_tester->m_moveOri = AnimationStackedWidget::MoveOrientation::TopToBottom;
    Stub stub;
    stub.set(ADDR(QStackedWidget, widget), stub_paintEvent_widget);
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->paintNextWidget(painter, 0);
}

TEST_F(UT_AnimationStackedWidget, test_paintNextWidget_04)
{
    m_tester->m_moveOri = AnimationStackedWidget::MoveOrientation::BottomToTop;
    Stub stub;
    stub.set(ADDR(QStackedWidget, widget), stub_paintEvent_widget);
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->paintNextWidget(painter, 0);
}

