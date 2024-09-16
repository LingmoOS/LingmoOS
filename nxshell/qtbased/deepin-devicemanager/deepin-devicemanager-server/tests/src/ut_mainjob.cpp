// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainjob.h"
#include "ut_Head.h"
#include <gtest/gtest.h>
#include "stub.h"

#include <QProcess>

class MainJob_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_mainJob = new MainJob("org.deepin.DeviceInfo");
    }
    void TearDown()
    {
        delete m_mainJob;
    }
    MainJob *m_mainJob = nullptr;
};

void ut_process_start()
{
    return;
}

bool ut_initDBus()
{
    return true;
}

TEST_F(MainJob_UT, MainJob_UT_executeClientInstruction)
{
    m_mainJob->m_firstUpdate = true;
    m_mainJob->executeClientInstruction("UPDATE_UI");
}

TEST_F(MainJob_UT, MainJob_UT_slotUsbChanged)
{
    m_mainJob->slotUsbChanged();
}

TEST_F(MainJob_UT, MainJob_UT_clientIsRunning)
{
    m_mainJob->clientIsRunning();
}

TEST_F(MainJob_UT, MainJob_UT_serverIsRunning)
{
    m_mainJob->serverIsRunning();
}
