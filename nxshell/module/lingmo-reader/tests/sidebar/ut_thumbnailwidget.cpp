// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ThumbnailWidget.h"
#include "DocSheet.h"
#include "CatalogTreeView.h"
#include "SideBarImageListview.h"
#include "PagingWidget.h"

#include "stub.h"

#include <QStackedLayout>
#include <gtest/gtest.h>

class UT_ThumbnailWidget : public ::testing::Test
{
public:
    UT_ThumbnailWidget() {}

public:
    virtual void SetUp()
    {
        QString strPath = UTSOURCEDIR;
        strPath += "/files/1.pdf";
        m_sheet = new DocSheet(Dr::PDF, strPath, nullptr);
        m_tester = new ThumbnailWidget(m_sheet);
    }

    virtual void TearDown()
    {
        delete m_tester;
        delete m_sheet;
    }

protected:
    DocSheet *m_sheet = nullptr;
    ThumbnailWidget *m_tester = nullptr;
};

TEST_F(UT_ThumbnailWidget, initTest)
{

}

TEST_F(UT_ThumbnailWidget, UT_ThumbnailWidget_handleOpenSuccess_001)
{
    m_tester->bIshandOpenSuccess = true;
    m_tester->handleOpenSuccess();
    EXPECT_TRUE(m_tester->bIshandOpenSuccess == true);
}

TEST_F(UT_ThumbnailWidget, UT_ThumbnailWidget_handleOpenSuccess_002)
{
    m_tester->bIshandOpenSuccess = false;
    m_tester->handleOpenSuccess();
    EXPECT_TRUE(m_tester->bIshandOpenSuccess == true);
}

TEST_F(UT_ThumbnailWidget, UT_ThumbnailWidget_handleRotate)
{
    m_tester->handleRotate();
    EXPECT_TRUE(m_tester->m_pImageListView != nullptr);
}

TEST_F(UT_ThumbnailWidget, UT_ThumbnailWidget_handlePage)
{
    m_tester->handlePage(0);
    EXPECT_TRUE(m_tester->m_pPageWidget->m_curIndex == 0);
}

TEST_F(UT_ThumbnailWidget, UT_ThumbnailWidget_setBookMarkState)
{
    m_tester->setBookMarkState(0, 0);
    EXPECT_TRUE(m_tester->m_pImageListView->getImageModel() != nullptr);
}

TEST_F(UT_ThumbnailWidget, UT_ThumbnailWidget_prevPage)
{
    m_tester->prevPage();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_ThumbnailWidget, UT_ThumbnailWidget_pageUp)
{
    m_tester->prevPage();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_ThumbnailWidget, UT_ThumbnailWidget_nextPage)
{
    m_tester->nextPage();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_ThumbnailWidget, UT_ThumbnailWidget_pageDown)
{
    m_tester->pageDown();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_ThumbnailWidget, UT_ThumbnailWidget_adaptWindowSize)
{
    m_tester->adaptWindowSize(1);
    EXPECT_TRUE(m_tester->m_pImageListView->property("adaptScale") == 1);
}

TEST_F(UT_ThumbnailWidget, UT_ThumbnailWidget_scrollToCurrentPage)
{
    m_tester->scrollToCurrentPage();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_ThumbnailWidget, UT_ThumbnailWidget_setTabOrderWidget)
{
    QList<QWidget *> tabWidgetlst;
    m_tester->setTabOrderWidget(tabWidgetlst);
    EXPECT_EQ(tabWidgetlst.count(), 3);
}
