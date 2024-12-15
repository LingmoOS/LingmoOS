// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceWidget.h"
#include "LoadInfoThread.h"
#include "ThreadExecXrandr.h"
#include "GenerateDevicePool.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_LoadInfoThread : public UT_HEAD
{
public:
    void SetUp()
    {
        m_loadInfoThread = new LoadInfoThread;
    }
    void TearDown()
    {
        delete m_loadInfoThread;
    }
    LoadInfoThread *m_loadInfoThread;
};

class UT_ThreadExecXrandr : public UT_HEAD
{
public:
    void SetUp()
    {
        m_threadExecXrandr = new ThreadExecXrandr(true, true);
    }
    void TearDown()
    {
        delete m_threadExecXrandr;
    }
    ThreadExecXrandr *m_threadExecXrandr;
};

//TEST_F(UT_LoadInfoThread,UT_LoadInfoThread_start){
//    m_loadInfoThread->run();
//    EXPECT_FALSE(m_loadInfoThread->m_Running);
//}

//TEST_F(UT_ThreadExecXrandr,UT_ThreadExecXrandr_start){
//    m_threadExecXrandr->run();
//    EXPECT_TRUE(m_threadExecXrandr->m_Gpu);
//}


