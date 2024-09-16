// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CmdButtonWidget.h"
#include "ut_Head.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_CmdButtonWidget : public UT_HEAD
{
public:
    void SetUp()
    {
        m_cmdBtnWidget = new CmdButtonWidget;
    }
    void TearDown()
    {
        delete m_cmdBtnWidget;
    }
    CmdButtonWidget *m_cmdBtnWidget;
};

TEST_F(UT_CmdButtonWidget, UT_CmdButtonWidget_paintEvent)
{
    QPaintEvent paint(QRect(m_cmdBtnWidget->rect()));
    m_cmdBtnWidget->paintEvent(&paint);
    EXPECT_FALSE(m_cmdBtnWidget->grab().isNull());
}
