// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logspinnerwidget.h"

#include <gtest/gtest.h>
#include <stub.h>

#include <QDebug>
TEST(LogSpinnerWidget_Constructor_UT, LogSpinnerWidget_Constructor_UT_001)
{
    LogSpinnerWidget *p = new LogSpinnerWidget(nullptr);
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(p->m_spinner->size(), QSize(32, 32))<<"check the status after Constructor()";
    p->deleteLater();

}

TEST(LogSpinnerWidget_initUI_UT, LogSpinnerWidget_initUI_UT_001)
{
    LogSpinnerWidget *p = new LogSpinnerWidget(nullptr);
    p->initUI();
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(p->m_spinner->size(), QSize(32, 32))<<"check the status after initUI()";
    p->deleteLater();
}

TEST(LogSpinnerWidget_spinnerStart_UT, LogSpinnerWidget_spinnerStart_UT)
{
    LogSpinnerWidget *p = new LogSpinnerWidget(nullptr);
    p->spinnerStart();
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(p->m_spinner->size(), QSize(32, 32))<<"check the status after spinnerStart()";
    p->deleteLater();
}

TEST(LogSpinnerWidget_spinnerStop_UT, LogSpinnerWidget_spinnerStop_UT)
{
    LogSpinnerWidget *p = new LogSpinnerWidget(nullptr);
    p->spinnerStop();
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(p->m_spinner->size(), QSize(32, 32))<<"check the status after spinnerStop()";
    p->deleteLater();
}
