// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_simplelistview.h"

#include "../../src/views/simplelistview.h"

Ut_SimpleListView::Ut_SimpleListView()
{
}

TEST_F(Ut_SimpleListView, connect)
{
    SimpleListView *m_simpleListView = new SimpleListView();
    m_simpleListView->verticalScrollBar()->rangeChanged(1, 1);
    EXPECT_EQ(m_simpleListView->viewportMargins(), QMargins(0, 0, 0, 0));
    delete m_simpleListView;
}

QAction *stub_exec_simple(const QPoint &pos, QAction *at = nullptr)
{
    Q_UNUSED(pos)
    Q_UNUSED(at)

    return nullptr;
}

TEST_F(Ut_SimpleListView, contextMenuEvent)
{
    SimpleListView *m_simpleListView = new SimpleListView();
    SimpleListModel *m_model = new SimpleListModel;
    m_simpleListView->setModel(m_model);
    m_model->appendText("1", true);
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *)) ADDR(QMenu, exec), stub_exec_simple);
    //    QContextMenuEvent *e = new QContextMenuEvent(QContextMenuEvent::Reason::Mouse, m_simpleListView->pos());
    m_simpleListView->listItemFill(true);
    QContextMenuEvent *e = new QContextMenuEvent(QContextMenuEvent::Reason::Mouse,
                                                 m_simpleListView->pos(), m_simpleListView->pos(),
                                                 Qt::KeyboardModifier::NoModifier);
    m_simpleListView->contextMenuEvent(e);
    EXPECT_EQ(e->pos(), m_simpleListView->pos());
    delete e;
    delete m_simpleListView;
    delete m_model;
}

TEST_F(Ut_SimpleListView, listItemFill)
{
    SimpleListView *m_simpleListView = new SimpleListView();
    m_simpleListView->listItemFill(true);
    EXPECT_TRUE(m_simpleListView->m_itemfill);
    delete m_simpleListView;
}

TEST_F(Ut_SimpleListView, showTextEditMenuByAltM)
{
    SimpleListView *m_simpleListView = new SimpleListView();
    SimpleListModel *m_simpleListModel = new SimpleListModel();
    m_simpleListModel->updataList("11", 0);
    m_simpleListModel->updataList("22", 1);
    m_simpleListModel->updataList("33", 2);
    m_simpleListModel->appendText("1", true);
    m_simpleListModel->appendText("2", true);
    m_simpleListModel->appendText("3", true);
    m_simpleListView->setModel(m_simpleListModel);
    m_simpleListView->setCurrentIndex(m_simpleListModel->index(1, 0));
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *)) ADDR(QMenu, exec), stub_exec_simple);
    //    QContextMenuEvent *e = new QContextMenuEvent(QContextMenuEvent::Reason::Mouse, m_simpleListView->pos());
    m_simpleListView->listItemFill(true);

    m_simpleListView->showTextEditMenuByAltM(m_simpleListView->currentIndex());
    EXPECT_EQ(m_simpleListView->currentIndex().row(), 1);
    delete m_simpleListView;
    delete m_simpleListModel;
}

TEST_F(Ut_SimpleListView, mouseMoveEvent)
{
    SimpleListView *m_simpleListView = new SimpleListView();
    m_simpleListView->m_mode = 1;
    m_simpleListView->m_itemfill = true;
    m_simpleListView->m_ispressed = true;
    QMouseEvent *m = new QMouseEvent(QMouseEvent::Type::MouseMove, m_simpleListView->pos(), Qt::MouseButton::LeftButton, Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    m_simpleListView->mouseMoveEvent(m);
    EXPECT_FALSE(m_simpleListView->m_presschanged);
    m_simpleListView->m_ispressed = false;
    m_simpleListView->mouseMoveEvent(m);
    EXPECT_FALSE(m_simpleListView->m_presschanged);

    m_simpleListView->m_mode = 0;
    SimpleListModel *m_model = new SimpleListModel;
    SimpleListDelegate *m_delegate = new SimpleListDelegate(0);
    m_simpleListView->setModel(m_model);
    m_simpleListView->setItemDelegate(m_delegate);
    m_model->appendText("1", true);
    m_simpleListView->mouseMoveEvent(m);
    EXPECT_FALSE(m_simpleListView->m_presschanged);
    delete m;
    delete m_simpleListView;
    delete m_model;
    delete m_delegate;
}

TEST_F(Ut_SimpleListView, adjustScrollbarMargins)
{
    SimpleListView *m_simpleListView = new SimpleListView();
    m_simpleListView->adjustScrollbarMargins();
    EXPECT_EQ(m_simpleListView->viewportMargins(), QMargins(0, 0, 0, 0));
    delete m_simpleListView;
}

TEST_F(Ut_SimpleListView, mousePressEvent)
{
    SimpleListView *m_simpleListView = new SimpleListView();
    SimpleListModel *m_model = new SimpleListModel;
    SimpleListDelegate *m_delegate = new SimpleListDelegate(0);
    m_simpleListView->setModel(m_model);
    m_simpleListView->setItemDelegate(m_delegate);
    m_model->appendText("1", true);
    m_simpleListView->m_mode = 1;
    QMouseEvent *m = new QMouseEvent(QMouseEvent::Type::MouseButtonPress,
                                     m_simpleListView->pos(), Qt::MouseButton::LeftButton,
                                     Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
    m_simpleListView->mousePressEvent(m);
    EXPECT_EQ(m_simpleListView->m_currentrow, 0);
    EXPECT_TRUE(m_simpleListView->m_ispressed);
    delete m;
    delete m_simpleListView;
    delete m_model;
    delete m_delegate;
}

TEST_F(Ut_SimpleListView, mouseReleaseEvent)
{
    SimpleListView *m_simpleListView = new SimpleListView();
    SimpleListModel *m_model = new SimpleListModel;
    SimpleListDelegate *m_delegate = new SimpleListDelegate(0);
    m_simpleListView->setModel(m_model);
    m_simpleListView->setItemDelegate(m_delegate);
    m_model->appendText("1", true);
    m_simpleListView->m_mode = 1;
    QMouseEvent *m = new QMouseEvent(QMouseEvent::Type::MouseButtonPress,
                                     m_simpleListView->pos(), Qt::MouseButton::LeftButton,
                                     Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
    m_simpleListView->mouseReleaseEvent(m);
    EXPECT_FALSE(m_simpleListView->m_ispressed);
    EXPECT_EQ(m_simpleListView->m_presspoint, QPoint());
    delete m;
    delete m_simpleListView;
    delete m_model;
    delete m_delegate;
}

TEST_F(Ut_SimpleListView, keyPressEvent)
{
    SimpleListView *m_simpleListView = new SimpleListView();
    SimpleListModel *m_simpleListModel = new SimpleListModel();
    SimpleListDelegate *m_simpleListDelegate = new SimpleListDelegate(0);
    m_simpleListModel->updataList("11", 0);
    m_simpleListModel->updataList("22", 1);
    m_simpleListModel->updataList("33", 2);
    m_simpleListModel->appendText("1", true);
    m_simpleListModel->appendText("2", true);
    m_simpleListModel->appendText("3", true);
    m_simpleListView->setModel(m_simpleListModel);
    m_simpleListView->setItemDelegate(m_simpleListDelegate);
    m_simpleListView->setCurrentIndex(m_simpleListModel->index(1, 0));
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QKeyEvent *k1 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QKeyEvent *k2 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QKeyEvent *k3 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    m_simpleListView->setCurrentIndex(m_simpleListView->model()->index(0, 0));
    m_simpleListView->keyPressEvent(k);
    m_simpleListView->keyPressEvent(k1);
    EXPECT_EQ(m_simpleListView->m_currentindexrow, 0);
    m_simpleListView->keyPressEvent(k2);
    EXPECT_EQ(m_simpleListView->m_currentindexrow, 1);
    m_simpleListView->keyPressEvent(k3);
    delete k;
    delete k1;
    delete k2;
    delete k3;

    delete m_simpleListView;
    delete m_simpleListModel;
    delete m_simpleListDelegate;
}

TEST_F(Ut_SimpleListView, focusInEvent)
{
    SimpleListView *m_simpleListView = new SimpleListView();
    SimpleListModel *m_model = new SimpleListModel;
    SimpleListDelegate *m_delegate = new SimpleListDelegate(0);
    m_simpleListView->setModel(m_model);
    m_simpleListView->setItemDelegate(m_delegate);
    m_model->appendText("1", true);
    m_simpleListView->m_mode = 1;
    QFocusEvent *f = new QFocusEvent(QFocusEvent::Type::FocusIn);
    m_simpleListView->focusInEvent(f);
    EXPECT_EQ(m_simpleListView->currentIndex().row(), 0);
    delete f;
    delete m_simpleListView;
    delete m_model;
    delete m_delegate;
}
