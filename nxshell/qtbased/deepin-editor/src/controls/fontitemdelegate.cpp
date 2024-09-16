// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fontitemdelegate.h"
#include <QPainter>

FontItemDelegate::FontItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

FontItemDelegate::~FontItemDelegate()
{
}

void FontItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QString text = index.data(Qt::DisplayRole).toString();
    bool isSelected = option.state & QStyle::State_Selected;

    painter->setPen(Qt::black);

    if (isSelected) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#2CA7F8"));
        painter->drawRect(option.rect);
        painter->setPen(Qt::white);
    }

    QFont font(painter->font());
    font.setFamily(text);
    painter->setFont(font);

    QRect textRect = option.rect;
    painter->drawText(QRect(textRect.x() + 10,
                            textRect.y(),
                            textRect.width() - 10,
                            textRect.height()), Qt::AlignVCenter | Qt::AlignLeft, text);
}

QSize FontItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(-1, 30);
}
