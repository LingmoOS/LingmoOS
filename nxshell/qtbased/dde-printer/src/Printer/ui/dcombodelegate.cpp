// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dcombodelegate.h"

#include <DPalette>
#include <DApplication>
#include <DApplicationHelper>
/*
DComboDelegate::DComboDelegate(QObject *parent):QItemDelegate(parent)
{

}

void DComboDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const
{
    int iState = index.data(COMBOITEMROLE::BACKCOLORROLE).toInt();

    if(CONFLICT == iState)
    {
        painter->fillRect(option.rect, Qt::white);
    }
    else
    {
        painter->fillRect(option.rect,Qt::red);
    }


    paint(painter, option, index);
}
*/

/*
ItemDelegate::ItemDelegate(QObject * parent)
    : QStyledItemDelegate(parent)
{

}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem  viewOption(option);
    if (viewOption.state & QStyle::State_HasFocus)
    {
        viewOption.state = viewOption.state ^ QStyle::State_HasFocus;
    }

    QStyledItemDelegate::paint(painter, viewOption, index);

    int height = (viewOption.rect.height() - 9) / 2;
    QPixmap pixmap = QPixmap(":/delete");
    QRect decorationRect = QRect(viewOption.rect.left() + viewOption.rect.width() - 30, viewOption.rect.top() + height, 9, 9);
    painter->drawPixmap(decorationRect, pixmap);
}
*/

ComItemDelegate::ComItemDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

void ComItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QMap<unsigned int, QRect> actions;
    QList<unsigned int> flags;

    //    qInfo() << index;
    QItemDelegate::paint(painter, option, index);
}

QSize ComItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    int iRow = index.row();

    //    qInfo() << ITEM_Height;

    if (0 == iRow)
        return QSize(72, 30);
    else if (1 == iRow)
        return QSize(102, 30);
    else if (2 == iRow)
        return QSize(137, 30);
    else if (3 == iRow)
        return QSize(131, 30);
    else if (4 == iRow)
        return QSize(65, 30);
    else if (5 == iRow)
        return QSize(124, 30);
    else if (6 == iRow)
        return QSize(146, 30);

    return option.rect.size();
}
