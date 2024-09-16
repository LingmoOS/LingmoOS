// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemdelegate.h"
#include "listview.h"

#include <DStyle>

#include <QPainter>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
using namespace dfm_wallpapersetting;

ItemDelegate::ItemDelegate(ListView *parent) : QAbstractItemDelegate(parent)
{
    updateBorderColor();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &ItemDelegate::updateBorderColor, Qt::QueuedConnection);
}

ListView *ItemDelegate::view() const
{
    return qobject_cast<ListView *>(parent());
}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setRenderHint(QPainter::Antialiasing);
    // draw selection state
    if (option.state & QStyle::State_Selected) {
        painter->save();
        const int penWidth = LISTVIEW_BORDER_WIDTH;
        QPen pen(option.palette.highlight(), penWidth);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        QRect focusRect = option.rect.marginsRemoved(QMargins(penWidth, penWidth, penWidth, penWidth));
        painter->drawRoundedRect(focusRect, LISTVIEW_BORDER_ROUND, LISTVIEW_BORDER_ROUND); //draw pen inclusive
        painter->restore();
    }

    const int margin = LISTVIEW_ICON_MARGIN;
    auto contentRect = option.rect.marginsRemoved(QMargins(margin, margin, margin, margin));

    QPainterPath path;
    path.addRoundedRect(contentRect, LISTVIEW_ICON_ROUND, LISTVIEW_ICON_ROUND);

    // draw pixmap
    auto pix = index.data(Qt::DisplayRole).value<QPixmap>();
    if (!pix.isNull()) {
        painter->save();
        painter->setClipPath(path);
        painter->drawPixmap(contentRect, pix);
        painter->restore();
    } else {
        // draw background
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(index.data(Qt::BackgroundRole).value<QColor>());
        painter->drawPath(path);
        painter->restore();
    }

    //draw border
    {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(border);
        QPainterPath in;
        in.addRoundedRect(contentRect.marginsRemoved(QMargins(1, 1, 1, 1)), LISTVIEW_ICON_ROUND, LISTVIEW_ICON_ROUND);
        painter->drawPath(path - in);
        painter->restore();
    }
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(LISTVIEW_ICON_WIDTH + 2 * LISTVIEW_ICON_MARGIN,
                 LISTVIEW_ICON_HEIGHT + 2 * LISTVIEW_ICON_MARGIN);
}

void ItemDelegate::updateBorderColor()
{
    bool dark = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
    border = dark ? QColor(255, 255, 255, 255 * 0.08) : QColor(0, 0, 0, 255 * 0.08);
}
