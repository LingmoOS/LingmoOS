// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_prolistview.h"

#include "../../src/views/prolistview.h"

Ut_ProListView::Ut_ProListView()
{

}

TEST_F(Ut_ProListView, keyPressEvent)
{
    ProListView *m_proListView = new ProListView();
    ProListModel *m_proListModel = new ProListModel();
    ProListDelegate *m_prolistdegate = new ProListDelegate();
    m_proListView->setModel(m_proListModel);
    m_proListView->setItemDelegate(m_prolistdegate);
    m_proListModel->updataList(Quantity(5));
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QKeyEvent *k1 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QKeyEvent *k2 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    m_proListView->keyPressEvent(k);
    m_proListView->keyPressEvent(k1);
    m_proListView->keyPressEvent(k2);
    delete k;
    delete k1;
    delete k2;
    //设置焦点函数，无assert
    delete m_proListView;
    delete m_proListModel;
    delete m_prolistdegate;
}

TEST_F(Ut_ProListView, focusIndex)
{
    ProListView *m_proListView = new ProListView();
    ProListModel *m_proListModel = new ProListModel();
    ProListDelegate *m_prolistdegate = new ProListDelegate();
    m_proListView->setModel(m_proListModel);
    m_proListView->setItemDelegate(m_prolistdegate);
    m_proListModel->updataList(Quantity(5));
    m_proListView->focusIndex();
    EXPECT_EQ(m_proListView->focusIndex(), m_proListView->model()->index(m_proListView->m_focusrow, 0));
    delete m_proListView;
    delete m_proListModel;
    delete m_prolistdegate;
}

QAction *stub_exec_pro(const QPoint &pos, QAction *at = nullptr)
{
    Q_UNUSED(pos)
    Q_UNUSED(at)

    return nullptr;
}

TEST_F(Ut_ProListView, contextMenuEvent)
{
    ProListView *m_proListView = new ProListView;
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), stub_exec_pro);
//    QContextMenuEvent *e = new QContextMenuEvent(QContextMenuEvent::Reason::Mouse, m_proListView->pos());
    QContextMenuEvent *e = new QContextMenuEvent(QContextMenuEvent::Reason::Mouse,
                                                 m_proListView->pos(), m_proListView->pos(),
                                                 Qt::KeyboardModifier::NoModifier);
    m_proListView->contextMenuEvent(e);
    EXPECT_EQ(e->pos(), m_proListView->pos());
    delete e;
    delete m_proListView;
}

TEST_F(Ut_ProListView, showTextEditMenuByAltM)
{
    ProListView *m_proListView = new ProListView;
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), stub_exec_pro);
//    QContextMenuEvent *e = new QContextMenuEvent(QContextMenuEvent::Reason::Mouse, m_proListView->pos());
    m_proListView->showTextEditMenuByAltM(QModelIndex());
    EXPECT_TRUE(m_proListView->m_isMenuAltM);
    delete m_proListView;
}
