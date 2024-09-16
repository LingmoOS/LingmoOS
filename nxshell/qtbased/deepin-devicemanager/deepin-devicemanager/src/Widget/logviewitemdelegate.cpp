// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// DTK
#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyle>
#include <DStyleHelper>

// Qt
#include <QLoggingCategory>
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QPainterPath>

// other
#include "logviewitemdelegate.h"

DWIDGET_USE_NAMESPACE

LogViewItemDelegate::LogViewItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void LogViewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    if (!index.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!(opt.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            cg = DPalette::Active;
        }
    }

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    if (!style)
        return;
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    QBrush background;
    QPen forground;
    if (opt.features & QStyleOptionViewItem::Alternate) {
        background = palette.color(cg, DPalette::AlternateBase);
    } else {
        background = palette.color(cg, DPalette::Base);
    }

    bool enableAndSelect = false;
    forground.setColor(palette.color(cg, DPalette::Text));
    if (opt.state & DStyle::State_Enabled) {
        if (opt.state & DStyle::State_Selected) {
            background = palette.color(cg, DPalette::Highlight);
            forground.setColor(palette.color(cg, DPalette::HighlightedText));
            enableAndSelect = true;
        }
    }
    painter->setPen(forground);

    QRect rect = opt.rect;

    QFontMetrics fm(opt.font);
    QPainterPath path, clipPath;

    QRect textRect = rect;

    QRect iconRect = rect;
    if (opt.viewItemPosition == QStyleOptionViewItem::Beginning &&
            index.data(Qt::DecorationRole).isValid()) {
        iconRect.setX(rect.x() - margin);
        iconRect.setWidth(64);
        QIcon ic = index.data(Qt::DecorationRole).value<QIcon>();
        ic.paint(painter, iconRect);
    }

    if (opt.viewItemPosition == QStyleOptionViewItem::Beginning) {
        textRect.setX(textRect.x() + margin * 2);
    } else {
        textRect.setX(textRect.x() + margin);
    }

    QString text = fm.elidedText(opt.text, opt.textElideMode, textRect.width());

    QPen p = painter->pen();
    if (text.startsWith("(" + tr("Disable") + ")") && !enableAndSelect) {
        p.setColor(QColor("#FF5736"));
    }else if(text.startsWith("(" + tr("Unavailable") + ")")){
        palette.color(cg, DPalette::PlaceholderText);
    }
    painter->setPen(p);
    painter->drawText(textRect, Qt::TextSingleLine | static_cast<int>(opt.displayAlignment), text);
    painter->restore();
}

QWidget *LogViewItemDelegate::createEditor(QWidget *, const QStyleOptionViewItem &,
                                           const QModelIndex &) const
{
    return nullptr;
}

QSize LogViewItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(std::max(36, size.height()));
    return size;
}

void LogViewItemDelegate::initStyleOption(QStyleOptionViewItem *option,
                                          const QModelIndex &index) const
{
    option->showDecorationSelected = true;
    bool ok = false;
    if (index.data(Qt::TextAlignmentRole).isValid()) {
        uint value = index.data(Qt::TextAlignmentRole).toUInt(&ok);
        option->displayAlignment = static_cast<Qt::Alignment>(value);
    }

    if (!ok)
        option->displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    option->textElideMode = Qt::ElideRight;
    option->features = QStyleOptionViewItem::HasDisplay;
    if (index.row() % 2 == 0)
        option->features |= QStyleOptionViewItem::Alternate;

    if (index.data(Qt::DisplayRole).isValid())
        option->text = index.data().toString();
}
