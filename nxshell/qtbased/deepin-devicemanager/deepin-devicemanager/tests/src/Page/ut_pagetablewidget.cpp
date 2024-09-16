// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageTableWidget.h"
#include "DetailTreeView.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_PageTableWidget : public UT_HEAD
{
public:
    void SetUp()
    {
        m_pageTableWidget = new PageTableWidget;
    }
    void TearDown()
    {
        delete m_pageTableWidget;
    }
    PageTableWidget *m_pageTableWidget = nullptr;
};

int ut_setTableHeight01()
{
    return 10;
}

TEST_F(UT_PageTableWidget, UT_PageTableWidget_setCurDeviceState_001)
{
    m_pageTableWidget->setCurDeviceState(false, false);
    EXPECT_FALSE(m_pageTableWidget->mp_Table->m_IsEnable);
}

TEST_F(UT_PageTableWidget, UT_PageTableWidget_setCurDeviceState_002)
{
    Stub stub;
    stub.set(ADDR(DetailTreeView, setTableHeight), ut_setTableHeight01);
    m_pageTableWidget->setCurDeviceState(true, true);
    EXPECT_TRUE(m_pageTableWidget->mp_Table->m_IsEnable);
}

TEST_F(UT_PageTableWidget, UT_PageTableWidget_expandTable)
{
    m_pageTableWidget->mp_Table->mp_CommandBtn = new DCommandLinkButton("");
    m_pageTableWidget->mp_Table->m_IsExpand = true;
    m_pageTableWidget->expandTable();
    EXPECT_STREQ("More", m_pageTableWidget->mp_Table->mp_CommandBtn->text().toStdString().c_str());
}
