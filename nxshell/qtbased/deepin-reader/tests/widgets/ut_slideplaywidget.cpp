// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SlidePlayWidget.h"
#include "ut_common.h"

#include <QSignalSpy>
#include <QDebug>

#include <gtest/gtest.h>

class UT_SlidePlayWidget : public ::testing::Test
{
public:
    UT_SlidePlayWidget(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new SlidePlayWidget();
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    SlidePlayWidget *m_tester;
};

TEST_F(UT_SlidePlayWidget, initTest)
{

}

static QString g_funcname;
void qTimer_start_stub1()
{
    g_funcname = __FUNCTION__;
    return;
}

TEST_F(UT_SlidePlayWidget, UT_SlidePlayWidget_showControl)
{
    Stub stub;
    stub.set((void(QTimer::*)())ADDR(QTimer, start), qTimer_start_stub1);
    m_tester->showControl();
    EXPECT_TRUE(g_funcname == "qTimer_start_stub1");
}

TEST_F(UT_SlidePlayWidget, test_TestTextEditShadowWidget_onTimerout)
{
    m_tester->onTimerout();
    EXPECT_TRUE(m_tester->isVisible() == false);
}

TEST_F(UT_SlidePlayWidget, test_TestTextEditShadowWidget_enterEvent)
{
    QEvent *event = new QEvent(QEvent::Enter);
    m_tester->enterEvent(event);
    delete event;
    EXPECT_TRUE(m_tester->m_timer.isActive() == false);
}

TEST_F(UT_SlidePlayWidget, test_TestTextEditShadowWidget_leaveEvent)
{
    Stub stub;
    stub.set((void(QTimer::*)())ADDR(QTimer, start), qTimer_start_stub1);
    QEvent *event = new QEvent(QEvent::Leave);
    m_tester->leaveEvent(event);
    delete event;
    EXPECT_TRUE(g_funcname == "qTimer_start_stub1");
}

TEST_F(UT_SlidePlayWidget, test_TestTextEditShadowWidget_setPlayStatus)
{
    m_tester->setPlayStatus(false);
    EXPECT_TRUE(m_tester->m_autoPlay == false);
}

TEST_F(UT_SlidePlayWidget, test_TestTextEditShadowWidget_getPlayStatus)
{
    m_tester->m_autoPlay = false;
    EXPECT_TRUE(m_tester->getPlayStatus() == false);
}

TEST_F(UT_SlidePlayWidget, test_TestTextEditShadowWidget_onPreClicked)
{
    QSignalSpy spy(m_tester, SIGNAL(signalPreBtnClicked()));
    m_tester->onPreClicked();
    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(UT_SlidePlayWidget, test_TestTextEditShadowWidget_onPlayClicked)
{
    m_tester->m_autoPlay = false;
    m_tester->onPlayClicked();
    EXPECT_TRUE(m_tester->m_autoPlay == true);
}

TEST_F(UT_SlidePlayWidget, test_TestTextEditShadowWidget_onNextClicked)
{
    QSignalSpy spy(m_tester, SIGNAL(signalNextBtnClicked()));
    m_tester->onNextClicked();
    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(UT_SlidePlayWidget, test_TestTextEditShadowWidget_onExitClicked)
{
    QSignalSpy spy(m_tester, SIGNAL(signalExitBtnClicked()));
    m_tester->onExitClicked();
    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(UT_SlidePlayWidget, test_TestTextEditShadowWidget_playStatusChanged_001)
{
    m_tester->m_autoPlay = true;
    QSignalSpy spy(m_tester, SIGNAL(signalPlayBtnClicked()));
    m_tester->playStatusChanged();
    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(UT_SlidePlayWidget, test_TestTextEditShadowWidget_playStatusChanged_002)
{
    m_tester->m_autoPlay = false;
    QSignalSpy spy(m_tester, SIGNAL(signalPlayBtnClicked()));
    m_tester->playStatusChanged();
    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(UT_SlidePlayWidget, test_TestTextEditShadowWidget_mousePressEvent)
{
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonRelease, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->mousePressEvent(event);
    delete event;
    EXPECT_TRUE(m_tester != nullptr);
}

