// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageInfo.h"
#include "PageTableHeader.h"
#include "TableWidget.h"
#include "logtreeview.h"
#include "DeviceInput.h"
#include "DeviceInfo.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_PageTableHeader : public UT_HEAD
{
public:
    void SetUp()
    {
        m_pageTableHeader = new PageTableHeader;
    }
    void TearDown()
    {
        delete m_pageTableHeader;
    }
    PageTableHeader *m_pageTableHeader = nullptr;
};

TEST_F(UT_PageTableHeader, UT_PageTableHeader_updateCurItemEnable)
{
//    QStandardItem* item = new QStandardItem("item");
//    m_pageTableHeader->mp_Table->mp_Table->mp_Model->insertRow(0);
//    m_pageTableHeader->mp_Table->mp_Table->mp_Model->insertColumn(0);
//    m_pageTableHeader->mp_Table->mp_Table->mp_Model->setItem(0,0,item);
//    m_pageTableHeader->updateCurItemEnable(0, true);
//    EXPECT_STREQ(m_pageTableHeader->mp_Table->mp_Table->mp_Model->item(0)->text().toStdString().c_str(),"item");
//    m_pageTableHeader->updateCurItemEnable(0, false);
//    EXPECT_STREQ(m_pageTableHeader->mp_Table->mp_Table->mp_Model->item(0)->text().toStdString().c_str(),"(Disable)item");
//    delete item;
}

TEST_F(UT_PageTableHeader, UT_PageTableHeader_paintEvent)
{
    QPaintEvent paint(QRect(m_pageTableHeader->rect()));
    m_pageTableHeader->paintEvent(&paint);
    EXPECT_FALSE(m_pageTableHeader->grab().isNull());
}
