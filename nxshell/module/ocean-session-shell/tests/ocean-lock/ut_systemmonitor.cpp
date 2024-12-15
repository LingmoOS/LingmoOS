// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "systemmonitor.h"

#include <QTest>

#include <gtest/gtest.h>

class UT_SystemMonitor : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    SystemMonitor *m_systemMonitor;
};

void UT_SystemMonitor::SetUp()
{
    m_systemMonitor = new SystemMonitor;
}

void UT_SystemMonitor::TearDown()
{
    delete m_systemMonitor;
}

TEST_F(UT_SystemMonitor, BasicTest)
{
    m_systemMonitor->state();
    m_systemMonitor->setState(SystemMonitor::Enter);
}

TEST_F(UT_SystemMonitor, EventTest)
{
    //m_systemMonitor->enterEvent(new QEvent(QEvent::Type::Enter));
    //m_systemMonitor->leaveEvent(new QEvent(QEvent::Type::Leave));
    //m_systemMonitor->paintEvent(new QPaintEvent(m_systemMonitor->rect()));
    QTest::mousePress(m_systemMonitor, Qt::LeftButton, Qt::KeyboardModifier::NoModifier);
    QTest::mouseRelease(m_systemMonitor, Qt::LeftButton, Qt::KeyboardModifier::NoModifier);
}
