// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "NotesWidget.h"
#include "SideBarImageListview.h"
#include "SideBarImageViewModel.h"
#include "DocSheet.h"
#include "MsgHeader.h"
#include "SaveDialog.h"
#include "SheetBrowser.h"
#include "ut_common.h"

#include "stub.h"

#include <gtest/gtest.h>
#include <QTest>
#include <QListView>
#include <QPainter>

DWIDGET_USE_NAMESPACE

class UT_NotesWidget : public ::testing::Test
{
public:
    UT_NotesWidget() {}

public:
    virtual void SetUp()
    {
        QString strPath = UTSOURCEDIR;
        strPath += "/files/1.pdf";
        m_sheet = new DocSheet(Dr::PDF, strPath, nullptr);
        m_tester = new NotesWidget(m_sheet);
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
        delete m_sheet;
    }

protected:
    DocSheet *m_sheet = nullptr;
    NotesWidget *m_tester = nullptr;
};

TEST_F(UT_NotesWidget, initTest)
{

}

TEST_F(UT_NotesWidget, UT_NotesWidget_prevPage)
{
    m_tester->pageUp();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_NotesWidget, UT_NotesWidget_nextPage)
{
    m_tester->nextPage();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_NotesWidget, UT_NotesWidget_pageDown)
{
    m_tester->pageDown();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_NotesWidget, UT_NotesWidget_deleteItemByKey)
{
    m_tester->deleteItemByKey();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_NotesWidget, UT_NotesWidget_deleteAllItem)
{
    m_tester->deleteAllItem();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_NotesWidget, UT_NotesWidget_handleOpenSuccess_001)
{
    m_tester->bIshandOpenSuccess = true;
    m_tester->handleOpenSuccess();
    EXPECT_TRUE(m_tester->bIshandOpenSuccess == true);
}

TEST_F(UT_NotesWidget, UT_NotesWidget_handleOpenSuccess_002)
{
    m_tester->bIshandOpenSuccess = false;
    m_tester->handleOpenSuccess();
    EXPECT_TRUE(m_tester->bIshandOpenSuccess == true);
}

TEST_F(UT_NotesWidget, UT_NotesWidget_onListMenuClick_001)
{
    m_tester->onListMenuClick(E_NOTE_COPY);
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_NotesWidget, UT_NotesWidget_onListMenuClick_002)
{
    m_tester->onListMenuClick(E_NOTE_DELETE);
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

static QString g_funcname;
int saveDialog_showTipDialog_stub()
{
    g_funcname = __FUNCTION__;
    return 1;
}

TEST_F(UT_NotesWidget, UT_NotesWidget_onListMenuClick_003)
{
    Stub stub;
    stub.set(ADDR(SaveDialog, showTipDialog), saveDialog_showTipDialog_stub);

    m_tester->onListMenuClick(E_NOTE_DELETE_ALL);
    EXPECT_TRUE(g_funcname == "saveDialog_showTipDialog_stub");
}

TEST_F(UT_NotesWidget, UT_NotesWidget_onListItemClicked)
{
    m_tester->onListItemClicked(0);
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_NotesWidget, UT_NotesWidget_onAddAnnotation)
{
    m_tester->onAddAnnotation();
    EXPECT_TRUE(m_tester->m_sheet->m_browser->m_annotationInserting == true);
}

TEST_F(UT_NotesWidget, UT_NotesWidget_copyNoteContent)
{
    m_tester->copyNoteContent();
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(UT_NotesWidget, UT_NotesWidget_adaptWindowSize)
{
    m_tester->adaptWindowSize(20);
    EXPECT_TRUE(m_tester->m_pImageListView->property("adaptScale") == 20);
}

TEST_F(UT_NotesWidget, UT_NotesWidget_showMenu)
{
    Stub stub;
    UTCommon::stub_DMenu_exec(stub);
    m_tester->showMenu();
    EXPECT_TRUE(m_tester->m_pImageListView != nullptr);
}

TEST_F(UT_NotesWidget, UT_NotesWidget_setTabOrderWidget)
{
    QList<QWidget *> listWgt;
    m_tester->setTabOrderWidget(listWgt);
    EXPECT_TRUE(listWgt.count() == 1);
}

