// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageInfoWidget.h"
#include "DeviceInput.h"
#include "PageInfo.h"
#include "PageMultiInfo.h"
#include "PageOverview.h"
#include "LongTextLabel.h"
#include "stub.h"
#include "ut_Head.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class PageInfoWidget_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_pageInfoWidget = new PageInfoWidget;
    }
    void TearDown()
    {
        delete m_pageInfoWidget;
    }
    PageInfoWidget *m_pageInfoWidget = nullptr;
};

TEST_F(PageInfoWidget_UT, PageInfoWidget_UT_updateTable)
{
    PageMultiInfo *p = new PageMultiInfo;
    m_pageInfoWidget->mp_PageInfo = dynamic_cast<PageInfo *>(p);
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    m_pageInfoWidget->updateTable("", bInfo);
    EXPECT_TRUE(m_pageInfoWidget->mp_PageInfo);
    DeviceInput *device1 = new DeviceInput;
    bInfo.append(device1);
    m_pageInfoWidget->updateTable("", bInfo);
    EXPECT_TRUE(m_pageInfoWidget->mp_PageInfo);
    bInfo.append(device);
    m_pageInfoWidget->updateTable("", bInfo);
    EXPECT_TRUE(m_pageInfoWidget->mp_PageInfo);
    delete device;
    delete device1;
    delete p;
}

TEST_F(PageInfoWidget_UT, PageInfoWidget_UT_updateTable2)
{
    PageMultiInfo *p = new PageMultiInfo;
    m_pageInfoWidget->mp_PageInfo = dynamic_cast<PageInfo *>(p);
    QMap<QString, QString> map;
    map.insert("Overview", "/");
    m_pageInfoWidget->updateTable(map);
    EXPECT_EQ("/",m_pageInfoWidget->mp_PageOverviewInfo->mp_DeviceLabel->text());
    delete p;
}

TEST_F(PageInfoWidget_UT, PageInfoWidget_UT_resizeEvent)
{
    QResizeEvent resizeevent(QSize(10, 10), QSize(10, 10));
    m_pageInfoWidget->resizeEvent(&resizeevent);
}

