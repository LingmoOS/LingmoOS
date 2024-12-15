// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DriverWaitingWidget.h"
#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>


class UT_DriverWaitingWidget : public UT_HEAD
{
public:
    void SetUp()
    {
        m_DriverWaitingWidget = new DriverWaitingWidget("");
    }
    void TearDown()
    {
        delete m_DriverWaitingWidget;
    }

    DriverWaitingWidget *m_DriverWaitingWidget;
};

TEST_F(UT_DriverWaitingWidget, UT_WaitingWidget_setValue)
{
    m_DriverWaitingWidget->setValue(10);

    EXPECT_EQ(10, m_DriverWaitingWidget->mp_Progress->value());
}

TEST_F(UT_DriverWaitingWidget, UT_WaitingWidget_setText)
{
    m_DriverWaitingWidget->setText("10");
    EXPECT_STREQ("10", m_DriverWaitingWidget->mp_Label->text().toStdString().c_str());
}
