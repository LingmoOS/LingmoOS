// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detectthread.h"
#include "monitorusb.h"
#include "../ut_Head.h"
#include <gtest/gtest.h>
#include "../stub.h"

class DetectThread_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_thread = new DetectThread(nullptr);
    }
    void TearDown()
    {
        delete m_thread;
    }
    DetectThread *m_thread = nullptr;
};

void ut_monitor1()
{
    return;
}

TEST_F(DetectThread_UT, DetectThread_UT_monitor)
{
    Stub stub;
    stub.set(ADDR(MonitorUsb, monitor), ut_monitor1);
    m_thread->run();
}
