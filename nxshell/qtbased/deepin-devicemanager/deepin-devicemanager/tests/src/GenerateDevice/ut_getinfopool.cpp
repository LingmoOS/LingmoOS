// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceWidget.h"
#include "LoadInfoThread.h"
#include "GetInfoPool.h"
#include "GenerateDevicePool.h"
#include "DeviceManager.h"
#include "CmdTool.h"
#include "stub.h"
#include "ut_Head.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_GetInfoPool : public UT_HEAD
{
public:
    void SetUp()
    {
        m_readFilePool = new GetInfoPool;
    }
    void TearDown()
    {
        delete m_readFilePool;
    }
    GetInfoPool *m_readFilePool;
};

class UT_CmdTask : public UT_HEAD
{
public:
    void SetUp()
    {
        readFilePool = new GetInfoPool;
        m_cmdTask = new CmdTask("", "", "", readFilePool);
    }
    void TearDown()
    {
        delete readFilePool;
        delete m_cmdTask;
    }
    CmdTask *m_cmdTask;
    GetInfoPool *readFilePool;
};

TEST_F(UT_GetInfoPool, UT_GetInfoPool_initcmd)
{
    m_readFilePool->m_CmdList.clear();
    m_readFilePool->initCmd();
    EXPECT_EQ(m_readFilePool->m_CmdList.size(), 29);
}

bool ut_getDeviceInfo_getAllInfo(void *obj, QString &deviceInfo, const QString &file)
{
    deviceInfo = "test info";
    return true;
}
TEST_F(UT_GetInfoPool, UT_GetInfoPool_getAllInfo)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_getDeviceInfo_getAllInfo);
    m_readFilePool->getAllInfo();
    EXPECT_TRUE(m_readFilePool->waitForDone(-1));
}

TEST_F(UT_GetInfoPool, UT_GetInfoPool_setFramework)
{
    m_readFilePool->setFramework("x86");
    EXPECT_STREQ("x86", m_readFilePool->m_Arch.toStdString().c_str());
}

