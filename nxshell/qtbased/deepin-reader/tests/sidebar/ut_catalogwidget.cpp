// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CatalogWidget.h"
#include "DocSheet.h"
#include "CatalogTreeView.h"

#include "stub.h"

#include <gtest/gtest.h>

class UT_CatalogWidget : public ::testing::Test
{
public:
    UT_CatalogWidget() {}

public:
    virtual void SetUp()
    {
        QString strPath = UTSOURCEDIR;
        strPath += "/files/1.pdf";
        m_sheet = new DocSheet(Dr::PDF, strPath, nullptr);
        m_tester = new CatalogWidget(m_sheet);
    }

    virtual void TearDown()
    {
        delete m_tester;
        delete m_sheet;
    }

protected:
    DocSheet *m_sheet = nullptr;
    CatalogWidget *m_tester = nullptr;
};

TEST_F(UT_CatalogWidget, initTest)
{

}

TEST_F(UT_CatalogWidget, UT_CatalogWidget_resizeEvent)
{
    m_tester->m_strTheme = "123";
    QResizeEvent *event = new QResizeEvent(QSize(100, 100), QSize(80, 80));
    m_tester->resizeEvent(event);
    delete event;
    EXPECT_TRUE(m_tester->titleLabel->text() == "123");
}

TEST_F(UT_CatalogWidget, UT_CatalogWidget_handleOpenSuccess)
{
    m_tester->m_strTheme = "123";
    m_tester->handleOpenSuccess();
    EXPECT_TRUE(m_tester->titleLabel->text() == "");
}

TEST_F(UT_CatalogWidget, UT_CatalogWidget_handlePage)
{
    m_tester->handlePage(1);
    EXPECT_TRUE(m_tester->m_pTree->rightnotifypagechanged == true);
}

TEST_F(UT_CatalogWidget, UT_CatalogWidget_nextPage)
{
    m_tester->nextPage();
    EXPECT_TRUE(m_tester->m_pTree != nullptr);
}

TEST_F(UT_CatalogWidget, UT_CatalogWidget_prevPage)
{
    m_tester->prevPage();
    EXPECT_TRUE(m_tester->m_pTree != nullptr);
}

TEST_F(UT_CatalogWidget, UT_CatalogWidget_pageDown)
{
    m_tester->pageDown();
    EXPECT_TRUE(m_tester->m_pTree != nullptr);
}

TEST_F(UT_CatalogWidget, UT_CatalogWidget_pageUp)
{
    m_tester->pageUp();
    EXPECT_TRUE(m_tester->m_pTree != nullptr);
}
