// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "listviewdelegate.h"
#include <dguiapplicationhelper.h>

#include <DGuiApplicationHelper>

#include <QPainter>

ListViewDelegate::ListViewDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
}

void ListViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Dtk::Widget::DStyledItemDelegate::paint(painter, option, index);

    if (!(option.state & QStyle::State_MouseOver))
       return;

    if (Dtk::Gui::DGuiApplicationHelper::instance()->themeType() == Dtk::Gui::DGuiApplicationHelper::DarkType) {
        QColor color(Qt::white);
        color.setAlpha(30);
        painter->fillRect(option.rect, color);
    } else {
        QColor color(Qt::black);
        color.setAlpha(20);
        painter->fillRect(option.rect, color);
    }
}
