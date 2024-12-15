// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

#include <QTime>
#include <QDebug>
#include "performancemonitor.h"

namespace {
class TestPerformanceMonitor: public testing::Test
{

protected:
    void SetUp()
    {
        m_pM = new PerformanceMonitor;
    }
    void TearDown()
    {
        delete  m_pM;
    }

    PerformanceMonitor *m_pM;
};

qint64 stub_toMSecSZero()
{
    qint64 i = 0;
    return i;
}

qint64 stub_toMSecSOne()
{
    qint64 i = 1;
    return i;
}

}

TEST_F(TestPerformanceMonitor, checkInitializeAppStart)
{
    Stub s;
    s.set(ADDR(QDateTime, toMSecsSinceEpoch), stub_toMSecSZero);

    m_pM->initializeAppStart();

    EXPECT_TRUE(m_pM->initializeAppStartMs == 0);
}

TEST_F(TestPerformanceMonitor, checkInitializeAppFinish)
{
    Stub s;
    s.set(ADDR(QDateTime, toMSecsSinceEpoch), stub_toMSecSOne);

    m_pM->initializeAppFinish();
    EXPECT_TRUE(m_pM->initializeAppFinishMs == 1);
}

TEST_F(TestPerformanceMonitor, checkInstallFontStart)
{
    Stub s;
    s.set(ADDR(QDateTime, toMSecsSinceEpoch), stub_toMSecSOne);

    m_pM->installFontStart();
    EXPECT_TRUE(m_pM->installFontStartMs == 1);
}


TEST_F(TestPerformanceMonitor, checkExportFontFinish)
{
    Stub s;
    s.set(ADDR(QDateTime, toMSecsSinceEpoch), stub_toMSecSOne);

    m_pM->exportFontFinish(1);
    EXPECT_TRUE(m_pM->exportFontFinishMs == 1);
}

TEST_F(TestPerformanceMonitor, checkDeleteFontStart)
{
    Stub s;
    s.set(ADDR(QDateTime, toMSecsSinceEpoch), stub_toMSecSOne);

    m_pM->deleteFontStart();
    EXPECT_TRUE(m_pM->deleteFontStartMs == 1);
}

