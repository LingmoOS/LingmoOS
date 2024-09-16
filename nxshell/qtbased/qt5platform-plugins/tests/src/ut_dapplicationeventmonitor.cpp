// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QWindow>
#include <QGuiApplication>

#include "dapplicationeventmonitor.h"

DPP_USE_NAMESPACE

class TDApplicationEventMonitor : public testing::Test
{
protected:
    void SetUp();
    void TearDown();

    DApplicationEventMonitor *monitor = nullptr;
};

void TDApplicationEventMonitor::SetUp()
{
    monitor = new DApplicationEventMonitor;
}

void TDApplicationEventMonitor::TearDown()
{
    delete monitor;
}

TEST_F(TDApplicationEventMonitor, lastInputDeviceType)
{
    ASSERT_EQ(monitor->lastInputDeviceType(), DApplicationEventMonitor::None);
}
