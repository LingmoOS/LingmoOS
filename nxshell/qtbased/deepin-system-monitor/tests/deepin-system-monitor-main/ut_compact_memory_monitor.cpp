// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "compact_memory_monitor.h"

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

class UT_CompactMemoryMonitor: public ::testing::Test
{
public:
    UT_CompactMemoryMonitor() : m_tester(nullptr) {}
    QWidget* m_widget = nullptr;

public:
    virtual void SetUp()
    {
        m_widget = new QWidget();
        m_tester = new CompactMemoryMonitor(m_widget);
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
    CompactMemoryMonitor *m_tester;
};

TEST_F(UT_CompactMemoryMonitor, initTest)
{
}

TEST_F(UT_CompactMemoryMonitor, test_progress)
{
    m_tester->progress();
}

TEST_F(UT_CompactMemoryMonitor, test_setProgress)
{
    qreal p = 10;
    m_tester->setProgress(p);
}

TEST_F(UT_CompactMemoryMonitor, test_onStatInfoUpdated)
{
    m_tester->onStatInfoUpdated();
}


TEST_F(UT_CompactMemoryMonitor, test_animationFinshed)
{
    m_tester->animationFinshed();
}

TEST_F(UT_CompactMemoryMonitor, test_onValueChanged)
{
    m_tester->onValueChanged();
}

TEST_F(UT_CompactMemoryMonitor, test_changeTheme_01)
{
    int themeType = DApplicationHelper::LightType;
    m_tester->changeTheme(themeType);
}

TEST_F(UT_CompactMemoryMonitor, test_changeTheme_02)
{
    int themeType = DApplicationHelper::DarkType;
    m_tester->changeTheme(themeType);
}

TEST_F(UT_CompactMemoryMonitor, test_changeTheme_03)
{
    int themeType = DApplicationHelper::UnknownType;
    m_tester->changeTheme(themeType);
}

TEST_F(UT_CompactMemoryMonitor, test_changeFont)
{
    QFont font;
    font.setWeight(QFont::Medium);
    m_tester->changeFont(font);
}

TEST_F(UT_CompactMemoryMonitor, test_paintEvent)
{
    EXPECT_TRUE(!m_tester->grab().isNull());
}

TEST_F(UT_CompactMemoryMonitor, test_mouseReleaseEvent)
{
    QMouseEvent *event = new QMouseEvent(QEvent::Type::Move, QPointF(0, 0), Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier);
    m_tester->mouseReleaseEvent(event);
    delete event;
}

TEST_F(UT_CompactMemoryMonitor, test_mouseMoveEvent)
{
    QMouseEvent *event = new QMouseEvent(QEvent::Type::Move, QPointF(0, 0), Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier);
    m_tester->mouseMoveEvent(event);
    delete event;
}
