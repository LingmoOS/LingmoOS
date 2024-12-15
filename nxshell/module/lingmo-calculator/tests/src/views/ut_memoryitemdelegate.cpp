// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_memoryitemdelegate.h"

#include "../../src/views/memorylistwidget.h"
#include <QPainter>

Ut_MemoryItemDelegate::Ut_MemoryItemDelegate()
{

}

void Ut_MemoryItemDelegate::SetUp()
{
    m_memoryItemDelegate = new MemoryItemDelegate;
}

void Ut_MemoryItemDelegate::TearDown()
{
    delete m_memoryItemDelegate;
}

//bool stub_focus_memdelegateT()
//{
//    return true;
//}

bool stub_focus_memdelegateF()
{
    return false;
}

TEST_F(Ut_MemoryItemDelegate, paint)
{
    MemoryListWidget *listwidget = new MemoryListWidget;
    listwidget->setItemDelegate(m_memoryItemDelegate);

    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(QSize(20, 20));
    MemoryItemWidget *widget = new MemoryItemWidget();
    widget->setFixedSize(QSize(20, 20));

    listwidget->insertItem(0, item);
    listwidget->setItemWidget(item, widget);

    QPainter *painter = new QPainter();
    QStyleOptionViewItem option;
    option.styleObject = listwidget;
    QModelIndex index;
    Stub stub;
    stub.set(ADDR(MemoryListWidget, hasFocus), stub_focus_memdelegateF);
//    Stub stub1;
//    stub1.set(ADDR(MemoryListWidget, rect), stub_rect_memdelegate);
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::DarkType);
    m_memoryItemDelegate->paint(painter, option, index);
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::LightType);
    m_memoryItemDelegate->paint(painter, option, index);
    delete painter;
    //paint函数，无assert
    listwidget->deleteLater();
    delete item;
    widget->deleteLater();
}

TEST_F(Ut_MemoryItemDelegate, updateEditorGeometry)
{
    QWidget *widget = new QWidget();
    QStyleOptionViewItem item;
    m_memoryItemDelegate->updateEditorGeometry(widget, item, QModelIndex());
    EXPECT_EQ(widget->geometry(), item.rect);
    widget->deleteLater();
}
