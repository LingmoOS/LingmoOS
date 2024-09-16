// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceWidget.h"
#include "DeviceFactory.h"
#include "X86Generator.h"
#include "GenerateDevicePool.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_GenerateTask : public UT_HEAD
{
public:
    void SetUp()
    {
        m_generateTask = new GenerateTask(type);
    }
    void TearDown()
    {
        delete m_generateTask;
    }
    GenerateTask *m_generateTask;
    DeviceType type = DT_Cpu;
};

class UT_GenerateDevicePool : public UT_HEAD
{
public:
    void SetUp()
    {
        m_generateDevicePool = new GenerateDevicePool;
    }
    void TearDown()
    {
        delete m_generateDevicePool;
    }
    GenerateDevicePool *m_generateDevicePool;
};

// void initType();
TEST_F(UT_GenerateDevicePool,UT_GenerateDevicePool_initType){
    m_generateDevicePool->m_TypeList.clear();
    m_generateDevicePool->initType();
    EXPECT_EQ(16, m_generateDevicePool->m_TypeList.size());
}

TEST_F(UT_GenerateDevicePool,UT_GenerateDevicePool_generateDevice){
    m_generateDevicePool->generateDevice();
    EXPECT_EQ(m_generateDevicePool->m_FinishedGenerator,0);
}
