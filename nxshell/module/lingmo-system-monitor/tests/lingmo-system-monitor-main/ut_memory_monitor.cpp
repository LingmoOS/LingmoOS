// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "memory_monitor.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

//qt
#include <DApplication>
#include <QMouseEvent>
#include <DApplicationHelper>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
using namespace core::system;

/***************************************STUB begin*********************************************/

//double stub_cpuAllPercent_noNum()
//{
//    return 1;
//}

/***************************************STUB end**********************************************/

class UT_MemoryMonitor: public ::testing::Test
{
public:
    UT_MemoryMonitor() : m_tester(nullptr) {}
    QWidget* m_widget = nullptr;

public:
    virtual void SetUp()
    {
        m_widget = new QWidget();
        m_tester = new MemoryMonitor(m_widget);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
        if (m_widget) {
            delete m_widget;
            m_widget = nullptr;
        }
    }

protected:
    MemoryMonitor *m_tester;
};

TEST_F(UT_MemoryMonitor, initTest)
{
}

TEST_F(UT_MemoryMonitor, test_onStatInfoUpdated)
{
    m_tester->onStatInfoUpdated();
}

TEST_F(UT_MemoryMonitor, test_onAnimationFinished)
{
    m_tester->onAnimationFinished();
}

TEST_F(UT_MemoryMonitor, test_onValueChanged)
{
    m_tester->onValueChanged();
}

TEST_F(UT_MemoryMonitor, test_changeTheme_01)
{
    m_tester->changeTheme(DApplicationHelper::LightType);
}

TEST_F(UT_MemoryMonitor, test_changeTheme_02)
{
    m_tester->changeTheme(DApplicationHelper::DarkType);
}

TEST_F(UT_MemoryMonitor, test_changeTheme_03)
{
    m_tester->changeTheme(DApplicationHelper::UnknownType);
}

TEST_F(UT_MemoryMonitor, test_changeFont)
{
    QFont font;
    font.setWeight(QFont::Medium);
    m_tester->changeFont(font);
}

TEST_F(UT_MemoryMonitor, test_paintEvent)
{
    EXPECT_FALSE(m_tester->grab().isNull());
}

TEST_F(UT_MemoryMonitor, test_mouseReleaseEvent)
{
    QMouseEvent *event = new QMouseEvent(QEvent::Type::Move, QPointF(0, 0), Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier);
    m_tester->mouseReleaseEvent(event);
    delete event;
}

TEST_F(UT_MemoryMonitor, test_mouseMoveEvent)
{
    QMouseEvent *event = new QMouseEvent(QEvent::Type::Move, QPointF(0, 0), Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier);
    m_tester->mouseMoveEvent(event);
    delete event;
}
