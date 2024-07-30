/*
    SPDX-FileCopyrightText: 2012 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NoSelectionRectDelegate.h"

NoSelectionRectDelegate::NoSelectionRectDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void NoSelectionRectDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // For some reason some styles don't honor the views SelectionRectVisible
    // and I just hate that selection rect thing...
    QStyleOptionViewItem opt(option);
    if (opt.state & QStyle::State_HasFocus) {
        opt.state ^= QStyle::State_HasFocus;
    }
    QStyledItemDelegate::paint(painter, opt, index);
}

#include "moc_NoSelectionRectDelegate.cpp"
