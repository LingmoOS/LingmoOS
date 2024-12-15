// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageInfo.h"
#include "WaitingWidget.h"
#include "DeviceInput.h"
#include "DeviceInfo.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_WaitingWidget : public UT_HEAD
{
public:
    void SetUp()
    {
        m_waitingWidget = new WaitingWidget;
    }
    void TearDown()
    {
        delete m_waitingWidget;
    }
    WaitingWidget *m_waitingWidget = nullptr;
};

TEST_F(UT_WaitingWidget, UT_WaitingWidget_start)
{
    m_waitingWidget->start();
    EXPECT_TRUE(m_waitingWidget->mp_Spinner->isPlaying());
    m_waitingWidget->stop();
    EXPECT_FALSE(m_waitingWidget->mp_Spinner->isPlaying());
}
