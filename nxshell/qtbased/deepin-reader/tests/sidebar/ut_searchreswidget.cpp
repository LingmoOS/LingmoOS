// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SearchResWidget.h"
#include "DocSheet.h"
#include "CatalogTreeView.h"
#include "SideBarImageListview.h"

#include "stub.h"

#include <QStackedLayout>
#include <gtest/gtest.h>

class UT_SearchResWidget : public ::testing::Test
{
public:
    UT_SearchResWidget() {}

public:
    virtual void SetUp()
    {
        QString strPath = UTSOURCEDIR;
        strPath += "/files/1.pdf";
        m_sheet = new DocSheet(Dr::PDF, strPath, nullptr);
        m_tester = new SearchResWidget(m_sheet);
    }

    virtual void TearDown()
    {
        delete m_tester;
        delete m_sheet;
    }

protected:
    DocSheet *m_sheet = nullptr;
    SearchResWidget *m_tester = nullptr;
};

TEST_F(UT_SearchResWidget, initTest)
{

}

TEST_F(UT_SearchResWidget, UT_SearchResWidget_handleSearchResultComming)
{
    deepin_reader::PageLine line;
    line.text = "123";

    deepin_reader::PageSection section;
    section << line;

    deepin_reader::SearchResult search;
    search.sections << section;

    m_tester->handleSearchResultComming(search);
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_SearchResWidget, UT_SearchResWidget_handleFindFinished)
{
    m_tester->handleFindFinished();
    EXPECT_TRUE(m_tester->m_stackLayout->currentIndex() == 1);
}

TEST_F(UT_SearchResWidget, UT_SearchResWidget_clearFindResult)
{
    m_tester->clearFindResult();
    EXPECT_TRUE(m_tester->m_stackLayout->currentIndex() == 0);
    EXPECT_TRUE(m_tester->m_searchKey.count() == 0);
}

TEST_F(UT_SearchResWidget, UT_SearchResWidget_searchKey)
{
    m_tester->searchKey("123");
    EXPECT_TRUE(m_tester->m_searchKey == "123");
}

TEST_F(UT_SearchResWidget, UT_SearchResWidget_addSearchsItem)
{
    m_tester->addSearchsItem(0, "123", 1);
    EXPECT_TRUE(m_tester->m_stackLayout->currentIndex() == 0);
}

TEST_F(UT_SearchResWidget, UT_SearchResWidget_adaptWindowSize)
{
    m_tester->adaptWindowSize(1);
    EXPECT_TRUE(m_tester->m_pImageListView->property("adaptScale") == 1);
}
