// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_prolistdelegate.h"

Ut_ProListDelegate::Ut_ProListDelegate()
{

}

void Ut_ProListDelegate::SetUp()
{
    m_prolistview1 = new ProListView();
    m_model1 = new ProListModel();
    m_proListDelegate1 = new ProListDelegate();
    m_proListDelegate = new ProListDelegate();
    m_model = new ProListModel();
}

void Ut_ProListDelegate::TearDown()
{
    m_prolistview1->deleteLater();
    delete m_model1;
    m_model1 = nullptr;
    delete m_proListDelegate;
    m_proListDelegate = nullptr;
    delete m_proListDelegate1;
    m_proListDelegate1 = nullptr;
    delete m_model;
    m_model = nullptr;
}

TEST_F(Ut_ProListDelegate, setThemeType)
{
    m_proListDelegate->setThemeType(1);
    EXPECT_EQ(m_proListDelegate->m_themeType, 1);
}

TEST_F(Ut_ProListDelegate, currentfocusindex)
{
    m_proListDelegate->currentfocusindex(QModelIndex());
    EXPECT_EQ(m_proListDelegate->m_focusindex, QModelIndex());
}

TEST_F(Ut_ProListDelegate, sizeHint)
{
    m_proListDelegate->sizeHint(QStyleOptionViewItem(), m_model->index(0, 0));
    EXPECT_EQ(m_proListDelegate->sizeHint(QStyleOptionViewItem(), m_model->index(3, 0)), QSize(451, 28));
}

TEST_F(Ut_ProListDelegate, editorEvent)
{
    QStyleOptionViewItem item;
    QEvent *e = new QEvent(QEvent::Type::None);
    EXPECT_TRUE(m_proListDelegate->editorEvent(e, m_model, item, m_model->index(0, 0)));
    delete e;
}

//bool stub_focus_prolistdelegateT()
//{
//    return true;
//}

bool stub_focus_prolistdelegateF()
{
    return false;
}

QModelIndex stub_index_prolistdelegate1()
{
    ProListModel m_model;
    return m_model.index(0, 0, QModelIndex());
}

QModelIndex stub_index_prolistdelegate3()
{
    ProListModel m_model;
    return m_model.index(3, 0, QModelIndex());
}

TEST_F(Ut_ProListDelegate, paintnofocus)
{
    m_prolistview1->setModel(m_model1);
    m_prolistview1->setItemDelegate(m_proListDelegate1);
    QPainter *painter = new QPainter();
    Stub stub;
    stub.set(ADDR(ProListView, hasFocus), stub_focus_prolistdelegateF);
    QStyleOptionViewItem option;
    option.init(m_prolistview1);
    Stub stub1;
    stub1.set(ADDR(ProListView, currentIndex), stub_index_prolistdelegate3);
    m_proListDelegate1->m_themeType = 0;
    m_prolistview1->setCurrentIndex(m_model1->index(3, 0, QModelIndex()));
    m_proListDelegate1->paint(painter, option, m_model1->index(3, 0, QModelIndex()));
    Stub stub2;
    stub2.set(ADDR(ProListView, currentIndex), stub_index_prolistdelegate1);
    m_proListDelegate1->m_themeType = 1;
    m_prolistview1->setCurrentIndex(m_model1->index(0, 0, QModelIndex()));
    m_proListDelegate1->paint(painter, option, m_model1->index(0, 0, QModelIndex()));
    delete painter;
}
