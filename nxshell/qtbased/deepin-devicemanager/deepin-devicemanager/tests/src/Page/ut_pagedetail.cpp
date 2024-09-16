// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageBoardInfo.h"
#include "DeviceInfo.h"
#include "DeviceBios.h"
#include "PageDetail.h"
#include "TextBrowser.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>

#include <gtest/gtest.h>

class PageDetail_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_pageDetail = new PageDetail;
    }
    void TearDown()
    {
        delete m_pageDetail;
    }
    PageDetail *m_pageDetail;
};

class DetailButton_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_detailButton = new DetailButton("More");
    }
    void TearDown()
    {
        delete m_detailButton;
    }
    DetailButton *m_detailButton;
};

class DetailSeperator_UT : public UT_HEAD
{
public:
    void SetUp()
    {
    }
    void TearDown()
    {
        delete m_detailSeperator;
    }
    DetailSeperator *m_detailSeperator;
};

class ScrollAreaWidget_UT : public UT_HEAD
{
public:
    void SetUp()
    {
    }
    void TearDown()
    {
        delete m_scrollWidget;
    }
    ScrollAreaWidget *m_scrollWidget;
};

TEST_F(DetailButton_UT, DetailButton_UT_updateText)
{
    m_detailButton->updateText();
    EXPECT_EQ("Collapse", m_detailButton->text());
}

TEST_F(DetailButton_UT, DetailButton_UT_paintEvent)
{
    QPaintEvent paint(QRect(m_detailButton->rect()));
    m_detailButton->paintEvent(&paint);
}

TEST_F(DetailSeperator_UT, DetailSeperator_UT_paintEvent)
{
    m_detailSeperator = new DetailSeperator;
    QPaintEvent paint(QRect(m_detailSeperator->rect()));
    m_detailSeperator->paintEvent(&paint);
}

TEST_F(ScrollAreaWidget_UT, ScrollAreaWidget_UT_paintEvent)
{
    m_scrollWidget = new ScrollAreaWidget;
    QPaintEvent paint(QRect(m_scrollWidget->rect()));
    m_scrollWidget->paintEvent(&paint);
}

TEST_F(PageDetail_UT, PageDetail_UT_showDeviceInfo)
{
    DeviceBios *device = new DeviceBios;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setBiosInfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    m_pageDetail->showDeviceInfo(bInfo);
    EXPECT_EQ(0, m_pageDetail->mp_ScrollArea->verticalScrollBar()->value());
    delete device;
}

TEST_F(PageDetail_UT, PageDetail_UT_showInfoOfNum)
{
    m_pageDetail->showInfoOfNum(2);
    EXPECT_EQ(0, m_pageDetail->mp_ScrollArea->verticalScrollBar()->value());
    m_pageDetail->enableDevice(0, true);
    EXPECT_EQ(0, m_pageDetail->m_ListTextBrowser.size());
}

TEST_F(PageDetail_UT, PageDetail_UT_paintEvent)
{
    QPaintEvent paint(QRect(m_pageDetail->rect()));
    m_pageDetail->paintEvent(&paint);
}

TEST_F(PageDetail_UT, PageDetail_UT_resizeEvent)
{
    QResizeEvent resizeevent(QSize(10, 10), QSize(10, 10));
    m_pageDetail->resizeEvent(&resizeevent);
}

TEST_F(PageDetail_UT, PageDetail_UT_addWidgets)
{
    TextBrowser *m_tBrowser = new TextBrowser;
    m_pageDetail->addWidgets(m_tBrowser, true);
    EXPECT_FALSE(m_pageDetail->mp_ScrollAreaLayout->widget());
    m_pageDetail->slotCopyAllInfo();
    EXPECT_TRUE(!m_pageDetail->m_ListTextBrowser.isEmpty());
    delete m_tBrowser;
}

TEST_F(PageDetail_UT, PageDetail_UT_slotBtnClicked)
{
    m_pageDetail->slotBtnClicked();
    EXPECT_TRUE(m_pageDetail->m_ListTextBrowser.isEmpty());
}
