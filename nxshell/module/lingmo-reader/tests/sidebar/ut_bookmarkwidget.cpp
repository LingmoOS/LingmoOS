// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "BookMarkWidget.h"
#include "DocSheet.h"
#include "SideBarImageListview.h"
#include "MsgHeader.h"

#include "stub.h"

#include <DPushButton>
#include <DApplicationHelper>

#include <gtest/gtest.h>
#include <QTest>
#include <QListView>

class TestBookMarkWidget : public ::testing::Test
{
public:
    TestBookMarkWidget(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strPath = UTSOURCEDIR;
        strPath += "/files/1.pdf";
        sheet = new DocSheet(Dr::PDF, strPath, nullptr);
        m_tester = new BookMarkWidget(sheet);
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete sheet;
        delete m_tester;
    }

protected:
    DocSheet *sheet = nullptr;
    BookMarkWidget *m_tester = nullptr;
};

TEST_F(TestBookMarkWidget, initTest)
{

}

TEST_F(TestBookMarkWidget, testprevPage)
{
    m_tester->prevPage();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(TestBookMarkWidget, testpageUp)
{
    m_tester->pageUp();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(TestBookMarkWidget, testnextPage)
{
    m_tester->pageUp();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(TestBookMarkWidget, testpageDown)
{
    m_tester->pageDown();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(TestBookMarkWidget, testhandleOpenSuccess)
{
    m_tester->handleOpenSuccess();
    EXPECT_TRUE(m_tester->bIshandOpenSuccess == true);
}

TEST_F(TestBookMarkWidget, testhandlePage)
{
    m_tester->handlePage(0);
    EXPECT_TRUE(m_tester->m_pAddBookMarkBtn->isEnabled() == true);
}

TEST_F(TestBookMarkWidget, testhandleBookMark)
{
    m_tester->handleBookMark(0, 0);
    EXPECT_TRUE(m_tester->m_pImageListView != nullptr);
}

TEST_F(TestBookMarkWidget, testdeleteItemByKey)
{
    m_tester->deleteItemByKey();
    EXPECT_TRUE(m_tester->m_sheet->m_bookmarks.count() == 0);
}

TEST_F(TestBookMarkWidget, testdeleteAllItem)
{
    m_tester->deleteAllItem();
    EXPECT_TRUE(m_tester->m_sheet->m_bookmarks.count() == 0);
}

TEST_F(TestBookMarkWidget, testonAddBookMarkClicked)
{
    m_tester->onAddBookMarkClicked();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(TestBookMarkWidget, testadaptWindowSize)
{
    m_tester->adaptWindowSize(1);
    EXPECT_TRUE(m_tester->m_pImageListView->property("adaptScale") == 1);
    EXPECT_TRUE(m_tester->m_pImageListView->itemSize() == QSize(266, 80));
}

TEST_F(TestBookMarkWidget, testshowMenu)
{
    m_tester->showMenu();
    EXPECT_TRUE(m_tester->m_pImageListView != nullptr);
    EXPECT_TRUE(m_tester->m_pImageListView->count() == 0);
}

TEST_F(TestBookMarkWidget, testonUpdateTheme)
{
    m_tester->onUpdateTheme();
    DPalette paFrame = DApplicationHelper::instance()->palette(m_tester);
    EXPECT_TRUE(DApplicationHelper::instance()->palette(m_tester) == paFrame);
}

static QString g_deleteItemByKey_result;
void deleteItemByKey_stub()
{
    g_deleteItemByKey_result = __FUNCTION__;
}


TEST_F(TestBookMarkWidget, testonListMenuClick)
{
    Stub stub;
    stub.set(ADDR(BookMarkWidget, deleteItemByKey), deleteItemByKey_stub);
    m_tester->onListMenuClick(E_MENU_ACTION::E_BOOKMARK_DELETE);
    EXPECT_TRUE(g_deleteItemByKey_result == "deleteItemByKey_stub");
}
