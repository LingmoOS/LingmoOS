// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QWindow>
#include <QGuiApplication>


#include "dinputselectionhandle.h"
#include "dapplicationeventmonitor.h"

#define private public
#include "ddesktopinputselectioncontrol.h"
#undef private

DPP_USE_NAMESPACE

class TDDesktopInputSelectionControl : public testing::Test
{
protected:
    void SetUp();
    void TearDown();

    DDesktopInputSelectionControl *control = nullptr;
    DApplicationEventMonitor *monitor = nullptr;
};

void TDDesktopInputSelectionControl::SetUp()
{
    control = new DDesktopInputSelectionControl(nullptr, qApp->inputMethod());
    monitor = new DApplicationEventMonitor;
}

void TDDesktopInputSelectionControl::TearDown()
{
    delete control;
    delete monitor;
}

TEST_F(TDDesktopInputSelectionControl, createHandles)
{
    ASSERT_TRUE(control->m_anchorSelectionHandle.isNull());
    ASSERT_TRUE(control->m_cursorSelectionHandle.isNull());
    control->createHandles();
    ASSERT_FALSE(control->m_anchorSelectionHandle.isNull());
    ASSERT_FALSE(control->m_cursorSelectionHandle.isNull());
}

TEST_F(TDDesktopInputSelectionControl, setApplicationEventMonitor)
{
    ASSERT_TRUE(control->m_pApplicationEventMonitor.isNull());
    control->setApplicationEventMonitor(monitor);
    ASSERT_FALSE(control->m_pApplicationEventMonitor.isNull());
}


