// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageInfo.h"
#include "PageMultiInfo.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class PageInfo_UT : public UT_HEAD
{
public:
    void SetUp()
    {
    }
    void TearDown()
    {
    }
    PageInfo *m_pageInfo = nullptr;
};

TEST_F(PageInfo_UT, PageInfo_UT_getDeviceInfoNum)
{
    PageMultiInfo *p = new PageMultiInfo;
    m_pageInfo = dynamic_cast<PageInfo *>(p);
    m_pageInfo->getDeviceInfoNum();
    QMap<QString, QString> map;
    map.insert("/", "/");
    m_pageInfo->updateInfo(map);
    m_pageInfo->setLabel("", "");
    m_pageInfo->clearContent();
    EXPECT_EQ(0,m_pageInfo->m_AllInfoNum);
    delete p;
}

TEST_F(PageInfo_UT, PageInfo_UT_paintEvent)
{
    PageMultiInfo *p = new PageMultiInfo;
    m_pageInfo = dynamic_cast<PageInfo *>(p);
    QPaintEvent paint(QRect(m_pageInfo->rect()));
    m_pageInfo->paintEvent(&paint);
    delete p;
}
