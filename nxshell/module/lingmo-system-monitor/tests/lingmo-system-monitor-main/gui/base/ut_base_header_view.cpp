// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "base/base_header_view.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QResizeEvent>
#include <QKeyEvent>


/***************************************STUB begin*********************************************/
bool stub_BaseHeaderView_hasFocus()
{
    return true;
}

QWidget* stub_BaseHeaderView_viewport()
{
    static QWidget widgetViewport;
    return &widgetViewport;
}
/***************************************STUB end**********************************************/


class UT_BaseHeaderView : public ::testing::Test
{
public:
    UT_BaseHeaderView() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget obj;
        m_tester = new BaseHeaderView(Qt::Horizontal, &obj);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    BaseHeaderView *m_tester;
};

TEST_F(UT_BaseHeaderView, initTest)
{

}

TEST_F(UT_BaseHeaderView, test_sizeHint_01)
{
    EXPECT_EQ(m_tester->sizeHint().height(), 37);
}

TEST_F(UT_BaseHeaderView, test_paintEvent_01)
{
    EXPECT_TRUE(!m_tester->grab().isNull());
}

//TEST_F(UT_BaseHeaderView, test_paintEvent_02)
//{
//    m_tester->m_focusReason = Qt::TabFocusReason;
//    Stub stub;
//    stub.set(ADDR(QWidget, hasFocus), stub_BaseHeaderView_hasFocus);
//    stub.set(ADDR(QAbstractScrollArea, viewport), stub_BaseHeaderView_viewport);
//    EXPECT_TRUE(!m_tester->grab().isNull());
//}

TEST_F(UT_BaseHeaderView, test_eventFilter_01)
{
    static QKeyEvent ev(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);

    EXPECT_TRUE(m_tester->eventFilter(m_tester, &ev));
}

TEST_F(UT_BaseHeaderView, test_eventFilter_02)
{
    static QKeyEvent ev(QEvent::KeyPress, Qt::Key_M, Qt::NoModifier);

    EXPECT_FALSE(m_tester->eventFilter(m_tester, &ev));
}

TEST_F(UT_BaseHeaderView, test_focusInEvent_01)
{
    static QFocusEvent ev(QEvent::FocusIn, Qt::OtherFocusReason);
    m_tester->focusInEvent(&ev);

    EXPECT_EQ(m_tester->m_focusReason, Qt::OtherFocusReason);
}

