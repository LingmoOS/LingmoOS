// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TableWidget.h"
#include "logtreeview.h"
#include "DeviceInfo.h"
#include "DeviceInput.h"
#include "ut_Head.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
//#include <QSignalSpy>

#include <gtest/gtest.h>

class UT_TableWidget : public UT_HEAD
{
public:
    void SetUp()
    {
        m_tableWidget = new TableWidget;
    }
    void TearDown()
    {
        delete m_tableWidget;
    }
    TableWidget *m_tableWidget;
};

TEST_F(UT_TableWidget, UT_TableWidget_setHeaderLabels)
{
    m_tableWidget->setHeaderLabels(QStringList() << "item1" << "item2");
    EXPECT_EQ(m_tableWidget->mp_Table->header()->count(),1);
}

TEST_F(UT_TableWidget, UT_TableWidget_setItem)
{
    DStandardItem *item = new DStandardItem("item");
    m_tableWidget->setItem(0, 0, item);
    m_tableWidget->setColumnAverage();
    m_tableWidget->updateCurItemEnable(0, true);
    EXPECT_STREQ(m_tableWidget->mp_Table->mp_Model->item(0,0)->text().toStdString().c_str(),"item");
    delete item;
    m_tableWidget->clear();
    EXPECT_EQ(m_tableWidget->mp_Table->mp_Model->rowCount(),0);
    m_tableWidget->setRowNum(1);
    EXPECT_EQ(m_tableWidget->mp_Table->m_RowCount,1);
}

TEST_F(UT_TableWidget, UT_TableWidget_PaintEvent)
{
    QPaintEvent paint(QRect(m_tableWidget->rect()));
    m_tableWidget->paintEvent(&paint);
    EXPECT_FALSE(m_tableWidget->grab().isNull());
}

TEST_F(UT_TableWidget, UT_TableWidget_slotActionExport)
{
    m_tableWidget->slotActionExport();
    m_tableWidget->slotActionRefresh();
    m_tableWidget->slotActionEnable();
    EXPECT_STREQ("Disable",m_tableWidget->mp_Enable->text().toStdString().c_str());
}

TEST_F(UT_TableWidget, UT_TableWidget_slotItemClicked)
{
    DStandardItem *item = new DStandardItem("item");
    m_tableWidget->mp_Table->mp_Model->insertRow(0);
    m_tableWidget->mp_Table->mp_Model->insertColumn(0);
    m_tableWidget->setItem(0, 0, item);
    QModelIndex index = m_tableWidget->mp_Table->mp_Model->item(0)->index();
    m_tableWidget->slotItemClicked(index);
    EXPECT_EQ(m_tableWidget->mp_Table->mp_Model->rowCount(),1);
    delete item;
}

TEST_F(UT_TableWidget, UT_TableWidget_initWidget)
{
    m_tableWidget->initWidget();
    EXPECT_TRUE(m_tableWidget->m_HLayout);
}
