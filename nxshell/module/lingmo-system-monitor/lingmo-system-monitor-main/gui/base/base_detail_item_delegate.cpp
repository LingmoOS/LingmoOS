// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base_detail_item_delegate.h"

#include <DPalette>
#include <DStyle>
#include <DApplicationHelper>

#include <QHelpEvent>
#include <QAbstractItemView>
#include <QToolTip>
#include <QStyleOptionViewItem>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE

#define SUMARY_ROW_BG_ALPH 0.03

const int space = 10;
const int margin = 10;
BaseDetailItemDelegate::BaseDetailItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

BaseDetailItemDelegate::~BaseDetailItemDelegate()
{

}

void BaseDetailItemDelegate::paint(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    QBrush background;
    QColor backgroundColor;
    if (DApplicationHelper::instance()->themeType() == Dtk::Gui::DGuiApplicationHelper::ColorType::LightType)
    {
        backgroundColor = QColor(0, 0, 0);
        backgroundColor.setAlphaF(0);
        if (!(index.row() & 1))
            backgroundColor.setAlphaF(SUMARY_ROW_BG_ALPH);
    }
    else
    {
        backgroundColor = QColor(255, 255, 255);
        backgroundColor.setAlphaF(0);
        if (!(index.row() & 1))
            backgroundColor.setAlphaF(SUMARY_ROW_BG_ALPH);
    }
    background = backgroundColor;
    painter->save();
    QPainterPath clipPath;
    clipPath.addRoundedRect(option.widget->rect().adjusted(1, 1, -1, -1), 6, 6);
    painter->setClipPath(clipPath);

    painter->setPen(Qt::NoPen);
    painter->setBrush(background);
    painter->drawRect(option.rect);

    if (index.isValid())
    {
        const auto &ltextpalette = index.data(Qt::TextColorRole).value<QColor>();
        painter->setPen(ltextpalette);

        QString ltext = index.data().toString();
        QString rtext = index.data(Qt::UserRole).toString();

        int rtmpW = painter->fontMetrics().width(rtext) + space;
        int ltmpW = painter->fontMetrics().width(ltext) + space;

        int leftWidth = qMin(qMax(option.rect.width() - rtmpW - 2 * margin, 68), ltmpW);
        int rightWidth = qMin(option.rect.width() - leftWidth - 2 * margin, rtmpW);

        ltext = painter->fontMetrics().elidedText(ltext, Qt::ElideRight, leftWidth);
        painter->drawText(option.rect.adjusted(margin, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, ltext);

        const auto &rtextpalette = DApplicationHelper::instance()->applicationPalette();
        painter->setPen(rtextpalette.color(DPalette::TextTips));

        rtext = painter->fontMetrics().elidedText(rtext, Qt::ElideRight, rightWidth);
        painter->drawText(option.rect.adjusted(0, 0, -margin, 0), Qt::AlignRight | Qt::AlignVCenter, rtext);
    }
    painter->restore();
}

QSize BaseDetailItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const
{
    return QSize(option.rect.width(), 36);
}

bool BaseDetailItemDelegate::helpEvent(QHelpEvent *e, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!e || !view)
        return false;

    // only process tooltip events for now
    if (e->type() == QEvent::ToolTip) {
        QFontMetrics fm(option.font);
        QString ltext = index.data().toString();
        QString rtext = index.data(Qt::UserRole).toString();

        int rtmpW = fm.width(rtext) + space;
        int ltmpW = fm.width(ltext) + space;

        int leftWidth = qMin(qMax(option.rect.width() - rtmpW - 2 * margin, 68), ltmpW);
        int rightWidth = qMin(option.rect.width() - leftWidth - 2 * margin, rtmpW);

        QRect leftRect = option.rect;
        leftRect.setWidth(leftWidth);

        QRect rightRect = option.rect;
        rightRect.setWidth(rightWidth);

        if (leftRect.contains(e->pos()) && leftWidth < ltmpW) {
            QToolTip::showText(e->globalPos(), QString("<div>%1</div>").arg(ltext.toHtmlEscaped()), view);
            return true;
        }

        if (rightRect.contains(e->pos()) && rightWidth < rtmpW) {
            QToolTip::showText(e->globalPos(), QString("<div>%1</div>").arg(rtext.toHtmlEscaped()), view);
            return true;
        }

        if (!QStyledItemDelegate::helpEvent(e, view, option, index))
            QToolTip::hideText();

        return true;
    }

    return QStyledItemDelegate::helpEvent(e, view, option, index);
}
