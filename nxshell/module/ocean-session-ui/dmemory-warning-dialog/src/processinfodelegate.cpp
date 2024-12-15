// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processinfodelegate.h"
#include "processinfomodel.h"

#include <QApplication>
#include <QDebug>
#include <QPainter>

ProcessInfoDelegate::ProcessInfoDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void ProcessInfoDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.row() % 2)
        painter->fillRect(option.rect, QColor(0, 0, 0, 255 * 0.05));

    if (index.column() != COLUMN_ICON)
        return QStyledItemDelegate::paint(painter, option, index);

    const QPixmap pix = index.data(ProcessInfoModel::IconRole).value<QPixmap>();
    painter->drawPixmap(option.rect.center() - pix.rect().center() / pix.devicePixelRatio(), pix);
}
