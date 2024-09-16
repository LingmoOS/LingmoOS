// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "../deb-installer/model/deblistmodel.h"
#include "../deb-installer/model/packagelistview.h"
#include "../deb-installer/manager/packagesmanager.h"
#include "../deb-installer/manager/PackageDependsStatus.h"
#include "utils/utils.h"

#include <stub.h>

class ut_packagelistview_Test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() { m_listview = new PackagesListView; }
    void TearDown() { delete m_listview; }

    PackagesListView *m_listview = nullptr;
};

TEST_F(ut_packagelistview_Test, packagelistview_UT_initUI)
{
    m_listview->initUI();
    EXPECT_TRUE(m_listview->hasAutoScroll());
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_initConnection)
{
    m_listview->initConnections();
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_initShortcus)
{
    m_listview->initShortcuts();
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_initRightContextMenu)
{
    m_listview->initRightContextMenu();
    EXPECT_EQ(1, m_listview->m_rightMenu->actions().size());
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_onShortcutDeleteAction)
{
    m_listview->slotShortcutDeleteAction();
    EXPECT_FALSE(m_listview->m_rightMenu->isVisible());
    EXPECT_EQ(-1, m_listview->m_currModelIndex.row());
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_onListViewShowContextMenu)
{
    QModelIndex index;
    m_listview->slotListViewShowContextMenu(index);
    EXPECT_FALSE(m_listview->m_bShortcutDelete);
    EXPECT_FALSE(m_listview->m_bIsRightMenuShow);
    EXPECT_FALSE(m_listview->m_rightMenu->isVisible());
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_onRightMenuDeleteAction)
{
    m_listview->slotRightMenuDeleteAction();
    EXPECT_EQ(-1, m_listview->m_currModelIndex.row());
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_getPos)
{
    m_listview->m_currentIndex = 1;
    m_listview->slotGetPos(QRect(1, 1, 1, 1), 1);
    EXPECT_EQ(1, m_listview->m_rightMenuPos.y());
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_setRightMenuShowStatus)
{
    m_listview->setRightMenuShowStatus(true);
    EXPECT_TRUE(m_listview->m_bIsRightMenuShow);
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_setSelection)
{
    QItemSelectionModel::SelectionFlags command;
    m_listview->setSelection(QRect(10, 10, 10, 10), command);
    ASSERT_FALSE(m_listview->m_bShortcutDelete);
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_mousePressEvent)
{
    QMouseEvent mousePressEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_listview->mousePressEvent(&mousePressEvent);
    EXPECT_TRUE(m_listview->m_bLeftMouse);
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_PaintEvent)
{
    QPaintEvent paint(QRect(m_listview->rect()));
    m_listview->paintEvent(&paint);
    EXPECT_EQ(-1, m_listview->m_currentIndex);
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_keyPressEvent)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QCoreApplication::sendEvent(m_listview, &keyPressEvent);
    EXPECT_TRUE(m_listview->m_bLeftMouse);
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_focusInEvent)
{
    QFocusEvent focus(QFocusEvent::FocusIn);
    QCoreApplication::sendEvent(m_listview, &focus);
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_event)
{
    QEvent event(QEvent::FontChange);
    QCoreApplication::sendEvent(m_listview, &event);
}
