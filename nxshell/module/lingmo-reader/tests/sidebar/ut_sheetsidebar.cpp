// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SheetSidebar.h"
#include "DocSheet.h"
#include "ThumbnailWidget.h"
#include "CatalogWidget.h"
#include "BookMarkWidget.h"
#include "NotesWidget.h"
#include "SearchResWidget.h"
#include "SideBarImageListview.h"
#include "ut_common.h"

#include "stub.h"

//#include <QStackedLayout>
#include <QTest>
#include <gtest/gtest.h>

class UT_SheetSidebar : public ::testing::Test
{
public:
    UT_SheetSidebar() {}

public:
    virtual void SetUp()
    {
        QString strPath = UTSOURCEDIR;
        strPath += "/files/1.pdf";
        m_sheet = new DocSheet(Dr::PDF, strPath, nullptr);
        m_tester = new SheetSidebar(m_sheet, PREVIEW_THUMBNAIL | PREVIEW_CATALOG | PREVIEW_BOOKMARK | PREVIEW_NOTE);
    }

    virtual void TearDown()
    {
        delete m_tester;
        delete m_sheet;
    }

protected:
    DocSheet *m_sheet = nullptr;
    SheetSidebar *m_tester = nullptr;
};

TEST_F(UT_SheetSidebar, initTest)
{

}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onBtnClicked)
{
    m_tester->onBtnClicked(0);
    EXPECT_TRUE(m_tester->m_stackLayout->currentIndex() == 0);
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_setBookMark)
{
    m_tester->setBookMark(0, 0);
    EXPECT_TRUE(m_tester->m_bookmarkWidget != nullptr);
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_setCurrentPage)
{
    m_tester->setCurrentPage(0);
    EXPECT_TRUE(m_tester->m_catalogWidget != nullptr);
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_handleOpenSuccess)
{
    m_tester->handleOpenSuccess();
    EXPECT_TRUE(m_tester->m_bOpenDocOpenSuccess == true);
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onHandleOpenSuccessDelay)
{
    m_tester->onHandleOpenSuccessDelay();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onHandWidgetDocOpenSuccess_001)
{
    Stub stub;
    UTCommon::stub_QWidget_isVisible(stub, true);
    m_tester->m_stackLayout->setCurrentIndex(0);
    m_tester->onHandWidgetDocOpenSuccess();
    EXPECT_TRUE(m_tester->m_thumbnailWidget->bIshandOpenSuccess == true);
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onHandWidgetDocOpenSuccess_002)
{
    Stub stub;
    UTCommon::stub_QWidget_isVisible(stub, true);
    m_tester->m_stackLayout->setCurrentIndex(1);
    m_tester->onHandWidgetDocOpenSuccess();
    EXPECT_TRUE(m_tester->m_catalogWidget->bIshandOpenSuccess == true);
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onHandWidgetDocOpenSuccess_003)
{
    Stub stub;
    UTCommon::stub_QWidget_isVisible(stub, true);
    m_tester->m_stackLayout->setCurrentIndex(2);
    m_tester->onHandWidgetDocOpenSuccess();
    EXPECT_TRUE(m_tester->m_bookmarkWidget->bIshandOpenSuccess == true);
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onHandWidgetDocOpenSuccess_004)
{
    Stub stub;
    UTCommon::stub_QWidget_isVisible(stub, true);
    m_tester->m_stackLayout->setCurrentIndex(3);
    m_tester->onHandWidgetDocOpenSuccess();
    EXPECT_TRUE(m_tester->m_notesWidget->bIshandOpenSuccess == true);
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_handleSearchStart)
{
    m_tester->handleSearchStart("123");
    EXPECT_TRUE(m_tester->m_searchWidget->m_searchKey == "123");
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_handleSearchStop)
{
    m_tester->handleSearchStop();
    EXPECT_TRUE(m_tester->m_searchWidget->m_searchKey == "");
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_handleFindFinished)
{
    EXPECT_TRUE(m_tester->handleFindFinished() == 0);
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_handleRotate)
{
    m_tester->handleRotate();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_handleAnntationMsg)
{
    m_tester->handleAnntationMsg(0, 0, nullptr);
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_createBtn)
{
    EXPECT_TRUE(m_tester->createBtn("123", "testBtn") != nullptr);
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_resizeEvent)
{
    QResizeEvent *event = new QResizeEvent(QSize(100, 100), QSize(80, 80));
    m_tester->resizeEvent(event);
    delete event;
    qInfo() << m_tester->width();
    EXPECT_TRUE(m_tester->width() == 266);
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_adaptWindowSize)
{
    m_tester->adaptWindowSize(1.0);
    EXPECT_TRUE(m_tester->m_scale == 1.0);
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_keyPressEvent)
{
    QTest::keyPress(m_tester, Qt::Key_PageUp);
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_showEvent)
{
    QShowEvent *event = new QShowEvent();
    m_tester->showEvent(event);
    delete event;
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

static QString g_funcname;
void SideBarImageListView_showMenu_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_showMenu)
{
    Stub stub;
    stub.set(ADDR(SideBarImageListView, showMenu), SideBarImageListView_showMenu_stub);
    m_tester->showMenu();
    EXPECT_TRUE(g_funcname != "SideBarImageListView_showMenu_stub");
}

void SheetSidebar_onJumpToPrevPage_stub()
{
    g_funcname = __FUNCTION__;
}

void SheetSidebar_onJumpToPageUp_stub()
{
    g_funcname = __FUNCTION__;
}

void SheetSidebar_onJumpToNextPage_stub()
{
    g_funcname = __FUNCTION__;
}

void SheetSidebar_onJumpToPageDown_stub()
{
    g_funcname = __FUNCTION__;
}

void SheetSidebar_deleteItemByKey_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_dealWithPressKey_001)
{
    Stub stub;
    stub.set(ADDR(SheetSidebar, onJumpToPrevPage), SheetSidebar_onJumpToPrevPage_stub);
    m_tester->dealWithPressKey(Dr::key_up);
    EXPECT_TRUE(g_funcname == "SheetSidebar_onJumpToPrevPage_stub");
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_dealWithPressKey_002)
{
    Stub stub;
    stub.set(ADDR(SheetSidebar, onJumpToPageUp), SheetSidebar_onJumpToPageUp_stub);
    m_tester->dealWithPressKey(Dr::key_pgUp);
    EXPECT_TRUE(g_funcname == "SheetSidebar_onJumpToPageUp_stub");
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_dealWithPressKey_003)
{
    Stub stub;
    stub.set(ADDR(SheetSidebar, onJumpToNextPage), SheetSidebar_onJumpToNextPage_stub);
    m_tester->dealWithPressKey(Dr::key_down);
    EXPECT_TRUE(g_funcname == "SheetSidebar_onJumpToNextPage_stub");
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_dealWithPressKey_004)
{
    Stub stub;
    stub.set(ADDR(SheetSidebar, onJumpToPageDown), SheetSidebar_onJumpToPageDown_stub);
    m_tester->dealWithPressKey(Dr::key_pgDown);
    EXPECT_TRUE(g_funcname == "SheetSidebar_onJumpToPageDown_stub");
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_dealWithPressKey_005)
{
    Stub stub;
    stub.set(ADDR(SheetSidebar, deleteItemByKey), SheetSidebar_deleteItemByKey_stub);
    m_tester->dealWithPressKey(Dr::key_delete);
    EXPECT_TRUE(g_funcname == "SheetSidebar_deleteItemByKey_stub");
}

void ThumbnailWidget_prevPage_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onJumpToPrevPage_001)
{
    Stub stub;
    stub.set(ADDR(ThumbnailWidget, prevPage), ThumbnailWidget_prevPage_stub);
    m_tester->m_stackLayout->setCurrentIndex(0);
    m_tester->onJumpToPrevPage();
    EXPECT_TRUE(g_funcname == "ThumbnailWidget_prevPage_stub");
}

void BookMarkWidget_prevPage_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onJumpToPrevPage_002)
{
    Stub stub;
    stub.set(ADDR(BookMarkWidget, prevPage), BookMarkWidget_prevPage_stub);
    m_tester->m_stackLayout->setCurrentIndex(2);
    m_tester->onJumpToPrevPage();
    EXPECT_TRUE(g_funcname == "BookMarkWidget_prevPage_stub");
}

void NotesWidget_prevPage_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onJumpToPrevPage_003)
{
    Stub stub;
    stub.set(ADDR(NotesWidget, prevPage), NotesWidget_prevPage_stub);
    m_tester->m_stackLayout->setCurrentIndex(3);
    m_tester->onJumpToPrevPage();
    EXPECT_TRUE(g_funcname == "NotesWidget_prevPage_stub");
}

void CatalogWidget_prevPage_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onJumpToPrevPage_004)
{
    Stub stub;
    stub.set(ADDR(CatalogWidget, prevPage), CatalogWidget_prevPage_stub);
    m_tester->m_stackLayout->setCurrentIndex(1);
    m_tester->onJumpToPrevPage();
    EXPECT_TRUE(g_funcname == "CatalogWidget_prevPage_stub");
}

void ThumbnailWidget_pageUp_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onJumpToPageUp_001)
{
    Stub stub;
    stub.set(ADDR(ThumbnailWidget, pageUp), ThumbnailWidget_pageUp_stub);
    m_tester->m_stackLayout->setCurrentIndex(0);
    m_tester->onJumpToPageUp();
    EXPECT_TRUE(g_funcname == "ThumbnailWidget_pageUp_stub");
}

void BookMarkWidget_pageUp_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onJumpToPageUp_002)
{
    Stub stub;
    stub.set(ADDR(BookMarkWidget, pageUp), BookMarkWidget_pageUp_stub);
    m_tester->m_stackLayout->setCurrentIndex(2);
    m_tester->onJumpToPageUp();
    EXPECT_TRUE(g_funcname == "BookMarkWidget_pageUp_stub");
}

void NotesWidget_pageUp_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onJumpToPageUp_003)
{
    Stub stub;
    stub.set(ADDR(NotesWidget, pageUp), NotesWidget_pageUp_stub);
    m_tester->m_stackLayout->setCurrentIndex(3);
    m_tester->onJumpToPageUp();
    EXPECT_TRUE(g_funcname == "NotesWidget_pageUp_stub");
}

void CatalogWidget_pageUp_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onJumpToPageUp_004)
{
    Stub stub;
    stub.set(ADDR(CatalogWidget, pageUp), CatalogWidget_pageUp_stub);
    m_tester->m_stackLayout->setCurrentIndex(1);
    m_tester->onJumpToPageUp();
    EXPECT_TRUE(g_funcname == "CatalogWidget_pageUp_stub");
}

void ThumbnailWidget_nextPage_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onJumpToNextPage_001)
{
    Stub stub;
    stub.set(ADDR(ThumbnailWidget, nextPage), ThumbnailWidget_nextPage_stub);
    m_tester->m_stackLayout->setCurrentIndex(0);
    m_tester->onJumpToNextPage();
    EXPECT_TRUE(g_funcname == "ThumbnailWidget_nextPage_stub");
}

void BookMarkWidget_nextPage_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onJumpToNextPage_002)
{
    Stub stub;
    stub.set(ADDR(BookMarkWidget, nextPage), BookMarkWidget_nextPage_stub);
    m_tester->m_stackLayout->setCurrentIndex(2);
    m_tester->onJumpToNextPage();
    EXPECT_TRUE(g_funcname == "BookMarkWidget_nextPage_stub");
}

void NotesWidget_nextPage_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onJumpToNextPage_003)
{
    Stub stub;
    stub.set(ADDR(NotesWidget, nextPage), NotesWidget_nextPage_stub);
    m_tester->m_stackLayout->setCurrentIndex(3);
    m_tester->onJumpToNextPage();
    EXPECT_TRUE(g_funcname == "NotesWidget_nextPage_stub");
}

void CatalogWidget_nextPage_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onJumpToNextPage_004)
{
    Stub stub;
    stub.set(ADDR(CatalogWidget, nextPage), CatalogWidget_nextPage_stub);
    m_tester->m_stackLayout->setCurrentIndex(1);
    m_tester->onJumpToNextPage();
    EXPECT_TRUE(g_funcname == "CatalogWidget_nextPage_stub");
}

void ThumbnailWidget_pageDown_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onJumpToPageDown_001)
{
    Stub stub;
    stub.set(ADDR(ThumbnailWidget, pageDown), ThumbnailWidget_pageDown_stub);
    m_tester->m_stackLayout->setCurrentIndex(0);
    m_tester->onJumpToPageDown();
    EXPECT_TRUE(g_funcname == "ThumbnailWidget_pageDown_stub");
}

void BookMarkWidget_pageDown_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onJumpToPageDown_002)
{
    Stub stub;
    stub.set(ADDR(BookMarkWidget, pageDown), BookMarkWidget_pageDown_stub);
    m_tester->m_stackLayout->setCurrentIndex(2);
    m_tester->onJumpToPageDown();
    EXPECT_TRUE(g_funcname == "BookMarkWidget_pageDown_stub");
}

void NotesWidget_pageDown_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onJumpToPageDown_003)
{
    Stub stub;
    stub.set(ADDR(NotesWidget, pageDown), NotesWidget_pageDown_stub);
    m_tester->m_stackLayout->setCurrentIndex(3);
    m_tester->onJumpToPageDown();
    EXPECT_TRUE(g_funcname == "NotesWidget_pageDown_stub");
}

void CatalogWidget_pageDown_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onJumpToPageDown_004)
{
    Stub stub;
    stub.set(ADDR(CatalogWidget, pageDown), CatalogWidget_pageDown_stub);
    m_tester->m_stackLayout->setCurrentIndex(1);
    m_tester->onJumpToPageDown();
    EXPECT_TRUE(g_funcname == "CatalogWidget_pageDown_stub");
}

void BookMarkWidget_deleteItemByKey_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_deleteItemByKey_001)
{
    Stub stub;
    stub.set(ADDR(BookMarkWidget, deleteItemByKey), BookMarkWidget_deleteItemByKey_stub);
    m_tester->m_stackLayout->setCurrentIndex(2);
    m_tester->deleteItemByKey();
    EXPECT_TRUE(g_funcname == "BookMarkWidget_deleteItemByKey_stub");
}

void NotesWidget_deleteItemByKey_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_deleteItemByKey_002)
{
    Stub stub;
    stub.set(ADDR(NotesWidget, deleteItemByKey), NotesWidget_deleteItemByKey_stub);
    m_tester->m_stackLayout->setCurrentIndex(3);
    m_tester->deleteItemByKey();
    EXPECT_TRUE(g_funcname == "NotesWidget_deleteItemByKey_stub");
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_onUpdateWidgetTheme)
{
    m_tester->onUpdateWidgetTheme();
    bool bTrue = false;
    if (m_tester->m_btnGroupMap.count() > 0) {
        bTrue = m_tester->m_btnGroupMap[0]->icon().isNull();
    }
    EXPECT_TRUE(bTrue == true);
}

void NotesWidget_changeResetModelData_stub()
{
    g_funcname = __FUNCTION__;
}

TEST_F(UT_SheetSidebar, UT_SheetSidebar_changeResetModelData)
{
    Stub stub;
    stub.set(ADDR(NotesWidget, changeResetModelData), NotesWidget_changeResetModelData_stub);
    m_tester->changeResetModelData();
    EXPECT_TRUE(g_funcname == "NotesWidget_changeResetModelData_stub");
}
