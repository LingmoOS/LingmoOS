// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "appchooserdelegate.h"
#include "appchoosermodel.h"

#include <QPainter>
#include <QDebug>
AppChooserDelegate::AppChooserDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void AppChooserDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
//    const QString &desktop = index.data(AppChooserModel::DataRole).toString();
    const QString &icon = index.data(AppChooserModel::IconRole).toString();
    const QString &name = index.data(AppChooserModel::NameRole).toString();

    // draw selected color
    bool selected = index.data(AppChooserModel::SelectRole).toBool();
    if (selected) {
        painter->fillRect(option.rect, Qt::lightGray);
    }

    // draw icon
    QRect iconRect = QRect(option.rect.x(), option.rect.y(), 40, 40);
    QPixmap pix = QIcon::fromTheme(icon).pixmap(40, 40);
    painter->drawPixmap(iconRect, pix);

    // draw name
    QRect nameRect = QRect(option.rect.x() + 40, option.rect.y(), option.rect.width() - iconRect.width(), option.rect.height());
    painter->drawText(nameRect, Qt::AlignVCenter, QFontMetrics(option.font).elidedText(name, Qt::ElideRight, option.rect.width() - iconRect.width()));
}

QSize AppChooserDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(240, 40);
}