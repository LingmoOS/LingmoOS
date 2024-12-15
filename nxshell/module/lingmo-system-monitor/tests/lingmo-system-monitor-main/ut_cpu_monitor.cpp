// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "cpu_monitor.h"
#include "model/cpu_info_model.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

//qt
#include <DApplication>
#include <QMouseEvent>

/***************************************STUB begin*********************************************/

double stub_cpuAllPercent()
{
    return 1;
}

/***************************************STUB end**********************************************/


class UT_CpuMonitor: public ::testing::Test
{
public:
    UT_CpuMonitor() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CpuMonitor();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    CpuMonitor *m_tester;
};

TEST_F(UT_CpuMonitor, initTest)
{
}

TEST_F(UT_CpuMonitor, test_onDetailInfoClicked)
{
    m_tester->onDetailInfoClicked();
}

TEST_F(UT_CpuMonitor, test_setDetailButtonVisible)
{
    m_tester->setDetailButtonVisible(true);
}

TEST_F(UT_CpuMonitor, test_changeTheme_01)
{
    m_tester->changeTheme(DApplicationHelper::LightType);
}

TEST_F(UT_CpuMonitor, test_changeTheme_02)
{
    m_tester->changeTheme(DApplicationHelper::DarkType);
}

TEST_F(UT_CpuMonitor, test_changeTheme_03)
{
    m_tester->changeTheme(DApplicationHelper::UnknownType);
}

TEST_F(UT_CpuMonitor, test_updateStatus_01)
{
    Stub stub;
    stub.set(ADDR(CPUInfoModel, cpuAllPercent), stub_cpuAllPercent);
    m_tester->updateStatus();
}

TEST_F(UT_CpuMonitor, test_updateStatus_02)
{
    m_tester->cpuPercents->append(0.1);
    m_tester->cpuPercents->append(0.2);
    m_tester->cpuPercents->append(0.3);
    m_tester->cpuPercents->append(0.4);
    m_tester->cpuPercents->append(0.5);
    m_tester->updateStatus();
}

TEST_F(UT_CpuMonitor, test_resizeEvent)
{
    QResizeEvent *event = nullptr;
    m_tester->resizeEvent(event);
}

TEST_F(UT_CpuMonitor, test_paintEvent)
{
    EXPECT_FALSE(m_tester->grab().isNull());
}

TEST_F(UT_CpuMonitor, test_changeFont)
{
    QFont font;
    font.setWeight(QFont::Medium);
    m_tester->changeFont(font);
}

TEST_F(UT_CpuMonitor, test_mouseReleaseEvent)
{
    QMouseEvent *event = new QMouseEvent(QEvent::Type::Move, QPointF(0, 0), Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier);
    m_tester->mouseReleaseEvent(event);
    delete event;
}

TEST_F(UT_CpuMonitor, test_mouseMoveEvent)
{
    QMouseEvent *event = new QMouseEvent(QEvent::Type::Move, QPointF(0, 0), Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier);
    m_tester->mouseMoveEvent(event);
    delete event;
}

