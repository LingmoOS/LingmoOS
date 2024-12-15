// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "BrowserMenu.h"
#include "BrowserPage.h"
#include "stub.h"

#include <QHideEvent>
#include <QPainter>
#include <QSignalSpy>

#include <gtest/gtest.h>

/********测试BrowserMenu***********/
class TestBrowserMenu : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    QWidget *m_widget = nullptr;
    BrowserMenu *m_tester = nullptr;
};

void TestBrowserMenu::SetUp()
{
    m_widget = new QWidget;
    m_tester = new BrowserMenu(m_widget);
}

void TestBrowserMenu::TearDown()
{
    delete m_widget;
}

/************桩函数***************/
bool hasBookMark_stub(int)
{
    return true;
}

static QObject *g_object;
static QObject *sender_stub()
{
    g_object = new QObject;
    return g_object;
}
/************测试用例***************/
TEST_F(TestBrowserMenu, UT_TestBrowserMenu_addTask_001)
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *docsheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    docsheet->m_bookmarks.insert(1);
    SheetMenuType_e type = DOC_MENU_ANNO_ICON;
    QString copytext("test");
    m_tester->initActions(docsheet, 0, type, copytext);
    EXPECT_TRUE(m_tester->m_type == DOC_MENU_ANNO_ICON);

    delete docsheet;
}

TEST_F(TestBrowserMenu, UT_TestBrowserMenu_addTask_002)
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *docsheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    docsheet->m_bookmarks.insert(1);
    SheetMenuType_e type = DOC_MENU_ANNO_ICON;
    QString copytext("test");
    m_tester->initActions(docsheet, 1, type, copytext);
    EXPECT_TRUE(m_tester->m_type == DOC_MENU_ANNO_ICON);

    delete docsheet;
}

TEST_F(TestBrowserMenu, UT_TestBrowserMenu_addTask_003)
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *docsheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    docsheet->m_bookmarks.insert(1);
    SheetMenuType_e type = DOC_MENU_ANNO_HIGHLIGHT;
    QString copytext("test");
    m_tester->initActions(docsheet, 1, type, copytext);
    EXPECT_TRUE(m_tester->m_type == DOC_MENU_ANNO_HIGHLIGHT);

    delete docsheet;
}

TEST_F(TestBrowserMenu, UT_TestBrowserMenu_addTask_004)
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *docsheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    docsheet->m_bookmarks.insert(1);
    SheetMenuType_e type = DOC_MENU_KEY;
    QString copytext("test");
    m_tester->initActions(docsheet, 1, type, copytext);
    EXPECT_TRUE(m_tester->m_type == DOC_MENU_KEY);

    delete docsheet;
}

TEST_F(TestBrowserMenu, UT_TestBrowserMenu_addTask_005)
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *docsheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    docsheet->m_bookmarks.insert(1);
    SheetMenuType_e type = DOC_MENU_DEFAULT;
    QString copytext("test");
    m_tester->initActions(docsheet, 1, type, copytext);
    EXPECT_TRUE(m_tester->m_type == DOC_MENU_DEFAULT);

    delete docsheet;
}

TEST_F(TestBrowserMenu, UT_TestBrowserMenu_hideEvent_001)
{
    QSignalSpy spy(m_tester, SIGNAL(sigMenuHide()));
    QHideEvent *event = new QHideEvent;
    m_tester->hideEvent(event);
    EXPECT_TRUE(spy.count() == 1);
    delete event;
}

TEST_F(TestBrowserMenu, UT_TestBrowserMenu_createAction_001)
{
    QAction *action = m_tester->createAction("Add bookmark", "AddBookmark");
    EXPECT_TRUE(action->text() == "Add bookmark");
}

TEST_F(TestBrowserMenu, UT_TestBrowserMenu_onItemClicked_001)
{
    Stub s;
    s.set(ADDR(QObject, sender), sender_stub);

    QSignalSpy spy(m_tester, SIGNAL(signalMenuItemClicked(const QString &)));
    m_tester->onItemClicked();

    EXPECT_TRUE(spy.count() == 1);
    delete g_object;
}

TEST_F(TestBrowserMenu, UT_TestBrowserMenu_onSetHighLight_001)
{
    QSignalSpy spy(m_tester, SIGNAL(signalMenuItemClicked(const QString &)));

    m_tester->m_type = DOC_MENU_SELECT_TEXT;
    m_tester->onSetHighLight();
    EXPECT_TRUE(spy.count() == 1);

    m_tester->m_type = DOC_MENU_ANNO_HIGHLIGHT;
    m_tester->onSetHighLight();
    EXPECT_TRUE(spy.count() == 2);
}
