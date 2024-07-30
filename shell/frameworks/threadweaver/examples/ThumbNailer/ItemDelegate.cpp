/* -*- C++ -*-
    This file implements the SMIVItemDelegate class.

    SPDX-FileCopyrightText: 2005 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

    License: LGPL with the following explicit clarification:
        This code may be linked against any version of the Qt toolkit
        from Trolltech, Norway.

    $Id: SMIVItemDelegate.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include <QModelIndex>
#include <QPainter>

#include "Image.h"
#include "ItemDelegate.h"
#include "Model.h"

const int ItemDelegate::FrameWidth = 2;
const int ItemDelegate::TextMargin = 6;
const int ItemDelegate::Margin = 3;

ItemDelegate::ItemDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const bool itemIsSelected = option.showDecorationSelected && (option.state & QStyle::State_Selected);
    const Image *image = index.data(Model::Role_ImageRole).value<const Image *>();
    // calculate some constants:
    const int y0 = option.rect.top();
    const int x0 = option.rect.left();
    const int width = option.rect.width();
    // const int height = option.rect.height();

    painter->save();

    // draw the background color, depending on focus:
    if (itemIsSelected) {
        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
        painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
    }

    // draw the image frame:
    painter->setPen(Qt::blue);
    painter->setBrush(Qt::white);
    painter->drawRect(x0 + FrameWidth + Margin, y0 + FrameWidth + Margin, Image::ThumbWidth + 1, Image::ThumbHeight + 1);

    // draw the image:
    if (image->progress().first >= Image::Step_LoadImage) {
        const QImage &thumb = image->thumbNail();
        QPoint orig = QPoint(x0 + FrameWidth + Margin + 1, y0 + FrameWidth + Margin + 1);
        painter->drawImage(orig, thumb, QRect(0, qMax(0, (thumb.height() - Image::ThumbHeight) / 2), Image::ThumbWidth, Image::ThumbHeight));
    }

    // render the text next to the image:
    painter->setPen(Qt::black);
    QFontMetrics font1Metrics(option.font);
    int textx0 = x0 + FrameWidth + Margin + Image::ThumbWidth + TextMargin;
    QRect text1Rect = QRect(textx0, y0 + TextMargin, width - TextMargin - textx0, font1Metrics.lineSpacing());
    painter->drawText(text1Rect, image->description());
    if (itemIsSelected) {
        painter->setPen(Qt::white);
    } else {
        painter->setPen(Qt::darkGray);
    }
    QFont font2 = option.font;
    font2.setPointSize((int)(0.9 * option.font.pointSize()));
    painter->setFont(font2);
    QFontMetrics font2Metrics(font2);
    QRect text2Rect = text1Rect.adjusted(0, font1Metrics.lineSpacing(), 0, font2Metrics.lineSpacing());
    painter->drawText(text2Rect, image->details());
    QRect text3Rect = text2Rect.adjusted(0, font2Metrics.lineSpacing(), 0, font2Metrics.lineSpacing());
    painter->drawText(text3Rect, image->details2());

    painter->restore();
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    static const int Width = Image::ThumbWidth + 2 * FrameWidth + 2 * Margin + 2;
    static const int Height = Image::ThumbHeight + 2 * FrameWidth + 2 * Margin + 2;
    return QSize(Width, Height);
}

#include "moc_ItemDelegate.cpp"
