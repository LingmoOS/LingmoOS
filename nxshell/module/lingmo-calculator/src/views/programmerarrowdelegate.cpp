// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "programmerarrowdelegate.h"

#include <DStyleHelper>
#include <DApplicationHelper>
#include "memorylistwidget.h"
#include "programmeritemwidget.h"

ProgrammerArrowDelegate::ProgrammerArrowDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

ProgrammerArrowDelegate::~ProgrammerArrowDelegate()
{

}

void ProgrammerArrowDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //边框绘制
    MemoryListWidget *dragWidget = qobject_cast<MemoryListWidget *>(option.styleObject);
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
    if (dragWidget->hasFocus()) {
        QRectF itemrect(dragWidget->visualItemRect(dragWidget->currentItem()));
        QColor color(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name());
        painter->setBrush(color);
        painter->drawRect(itemrect); //背景填充
    }
    QStyledItemDelegate::paint(painter, option, index);
}

void ProgrammerArrowDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}
