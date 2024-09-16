// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "monitorusb.h"
#include "../ut_Head.h"
#include <gtest/gtest.h>
#include "../stub.h"

class MonitorUsb_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_monitor = new MonitorUsb;
    }
    void TearDown()
    {
        delete m_monitor;
    }
    MonitorUsb *m_monitor = nullptr;
};

void ut_monitor()
{
    return;
}

TEST_F(MonitorUsb_UT, MonitorUsb_UT_monitor)
{
    Stub stub;
    stub.set(ADDR(MonitorUsb, monitor), ut_monitor);
    m_monitor->monitor();
}
