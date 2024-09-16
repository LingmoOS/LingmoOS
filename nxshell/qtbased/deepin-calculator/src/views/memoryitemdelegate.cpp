// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "memoryitemdelegate.h"

#include <DStyleHelper>
#include <DApplicationHelper>

MemoryItemDelegate::MemoryItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

MemoryItemDelegate::~MemoryItemDelegate()
{

}

/**
 * @brief 绘制memorylistwidget背景色
 */
void MemoryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    MemoryListWidget *dragWidget = qobject_cast<MemoryListWidget *>(option.styleObject); //将该object强转为memorylistwidget
    QRectF rect = dragWidget->rect(); //获取memorylistwidget.rect绘制背景色

    painter->setRenderHint(QPainter::Antialiasing, true);
    int type = DGuiApplicationHelper::instance()->themeType();
    if (type == 1) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#F8F8F8"));
        painter->drawRect(rect);
    } else {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#252525"));
        painter->drawRect(rect);
    }
    //内存焦点选中边框绘制
    if (dragWidget->hasFocus()) {
        QRectF itemrect(dragWidget->visualItemRect(dragWidget->currentItem()));
        QRectF frame(itemrect.left() + 1, itemrect.top() + 1, itemrect.width() - 2, itemrect.height() - 2);
        QPen pen;
        pen.setColor(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name());
        pen.setWidth(2);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(frame, 8, 8); //focus边框
    }
    QStyledItemDelegate::paint(painter, option, index);
}

/**
 * @brief 可以通过editor->setGeometry()更新编辑组件大小,保证editor显示的位置及大小,保证控件塞满计算器
 */
void MemoryItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}
