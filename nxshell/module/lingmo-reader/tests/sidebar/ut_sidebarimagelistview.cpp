// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SideBarImageListview.h"
#include "DocSheet.h"
#include "SideBarImageViewModel.h"

#include "stub.h"

#include <gtest/gtest.h>
#include <QTest>
#include <QListView>
#include <QScroller>

class TestSideBarImageListView : public ::testing::Test
{
public:
    TestSideBarImageListView(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strPath = UTSOURCEDIR;
        strPath += "/files/1.pdf";
        sheet = new DocSheet(Dr::PDF, strPath, nullptr);
        m_tester = new SideBarImageListView(sheet);
    }

    virtual void TearDown()
    {
        delete m_tester;
        delete sheet;
    }

protected:
    DocSheet *sheet = nullptr;
    SideBarImageListView *m_tester = nullptr;
};

bool isValid_stub()
{
    return true;
}

TEST_F(TestSideBarImageListView, initTest)
{

}

TEST_F(TestSideBarImageListView, testshowMenu)
{
    m_tester->showMenu();
    EXPECT_TRUE(m_tester->m_docSheet != nullptr);
}

TEST_F(TestSideBarImageListView, testsetListType)
{
    m_tester->setListType(0);
    EXPECT_TRUE(m_tester->m_listType == 0);
}

TEST_F(TestSideBarImageListView, testhandleOpenSuccess)
{
    m_tester->m_listType = E_SideBar::SIDE_THUMBNIL;
    m_tester->handleOpenSuccess();
    EXPECT_TRUE(m_tester->m_imageModel->m_pagelst.count() == 0);

    m_tester->m_listType = E_SideBar::SIDE_BOOKMARK;
    m_tester->handleOpenSuccess();
    EXPECT_TRUE(m_tester->m_imageModel->m_pagelst.count() == 0);

    m_tester->m_listType = E_SideBar::SIDE_NOTE;
    m_tester->handleOpenSuccess();
    EXPECT_TRUE(m_tester->m_imageModel->m_pagelst.count() == 0);
}

TEST_F(TestSideBarImageListView, testonItemClicked)
{
    m_tester->onItemClicked(QModelIndex());
    EXPECT_TRUE(m_tester->m_docSheet != nullptr);
}

static Qt::GestureType g_grabGesture_result = Qt::TapGesture;
Qt::GestureType grabGesture_stub(QObject *, QScroller::ScrollerGestureType)
{
    g_grabGesture_result = Qt::SwipeGesture;
    return g_grabGesture_result;
}

TEST_F(TestSideBarImageListView, testonSetThumbnailListSlideGesture)
{
    Stub stub;
    stub.set(ADDR(QScroller, grabGesture), grabGesture_stub);
    m_tester->onSetThumbnailListSlideGesture();
    EXPECT_TRUE(g_grabGesture_result == Qt::SwipeGesture);
}

TEST_F(TestSideBarImageListView, testonRemoveThumbnailListSlideGesture)
{
    Stub stub;
    stub.set(ADDR(QScroller, grabGesture), grabGesture_stub);
    m_tester->onRemoveThumbnailListSlideGesture();
    EXPECT_TRUE(g_grabGesture_result == Qt::SwipeGesture);
}

TEST_F(TestSideBarImageListView, testscrollToIndex)
{
    m_tester->scrollToIndex(0, true);
    EXPECT_TRUE(m_tester->currentIndex() == QModelIndex());
}

TEST_F(TestSideBarImageListView, testscrollToModelInexPage)
{
    m_tester->scrollToModelInexPage(QModelIndex(), true);
    EXPECT_TRUE(m_tester->m_docSheet != nullptr);
}

static QString g_showNoteMenu_result;
void showNoteMenu_stub(const QPoint &)
{
    g_showNoteMenu_result = __FUNCTION__;
}

static QString g_showBookMarkMenu_result;
void showBookMarkMenu_stub(const QPoint &)
{
    g_showBookMarkMenu_result = __FUNCTION__;
}

TEST_F(TestSideBarImageListView, testmousePressEvent)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), isValid_stub);
    stub.set(ADDR(SideBarImageListView, showNoteMenu), showNoteMenu_stub);
    stub.set(ADDR(SideBarImageListView, showBookMarkMenu), showBookMarkMenu_stub);

    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress, QPointF(50, 50), Qt::RightButton, Qt::RightButton, Qt::NoModifier);

    m_tester->m_listType = E_SideBar::SIDE_NOTE;
    m_tester->mousePressEvent(event);
    EXPECT_TRUE(g_showNoteMenu_result == "showNoteMenu_stub");

    m_tester->m_listType = E_SideBar::SIDE_BOOKMARK;
    m_tester->mousePressEvent(event);
    EXPECT_TRUE(g_showBookMarkMenu_result == "showBookMarkMenu_stub");

    delete event;
}

QAction *menu_exec_stub1(const QPoint &, QAction *)
{
    return nullptr;
}

TEST_F(TestSideBarImageListView, testshowNoteMenu)
{
    Stub stub;
    stub.set((QAction * (DMenu::*)(const QPoint &, QAction * at))ADDR(DMenu, exec), menu_exec_stub1);
    m_tester->showNoteMenu(QPoint(0, 0));
    EXPECT_TRUE(m_tester->m_pNoteMenu != nullptr);
}

TEST_F(TestSideBarImageListView, testshowBookMarkMenu)
{
    Stub stub;
    stub.set((QAction * (DMenu::*)(const QPoint &, QAction * at))ADDR(DMenu, exec), menu_exec_stub1);
    m_tester->showBookMarkMenu(QPoint(0, 0));
    EXPECT_TRUE(m_tester->m_pBookMarkMenu != nullptr);
}

TEST_F(TestSideBarImageListView, testgetModelIndexForPageIndex)
{
    EXPECT_TRUE(m_tester->getModelIndexForPageIndex(0) == -1);
}

TEST_F(TestSideBarImageListView, testgetPageIndexForModelIndex)
{
    EXPECT_TRUE(m_tester->getPageIndexForModelIndex(0) == -1);
}

TEST_F(TestSideBarImageListView, testpageUpIndex)
{
    EXPECT_TRUE(m_tester->pageUpIndex() == QModelIndex());
}

TEST_F(TestSideBarImageListView, testpageDownIndex)
{
    EXPECT_TRUE(m_tester->pageDownIndex() == QModelIndex());
}
