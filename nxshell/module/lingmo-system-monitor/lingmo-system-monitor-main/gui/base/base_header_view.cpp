// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base_header_view.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyle>
#include <DStyleHelper>

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QtMath>
#include <QModelIndex>
#include <QPainterPath>

// Constructor
BaseHeaderView::BaseHeaderView(Qt::Orientation orientation, QWidget *parent)
    : DHeaderView(orientation, parent)
{
    installEventFilter(this);
}

// Get header view's hint size
QSize BaseHeaderView::sizeHint() const
{
    return QSize(width(), 37);
}

// Paint event handler
void BaseHeaderView::paintEvent(QPaintEvent *event)
{
    // draw focus
    DHeaderView::paintEvent(event);
    if (hasFocus() && m_focusReason == Qt::TabFocusReason)
    {
        QPainter painter(viewport());
        painter.setRenderHint(QPainter::Antialiasing);

        QRect rect = viewport()->rect();
        QStyleOptionHeader option;
        initStyleOption(&option);

        auto *style = dynamic_cast<DStyle *>(DApplication::style());
        QStyleOptionFocusRect o;
        o.QStyleOption::operator=(option);
        // need take scroll offset into consideration
        QRect focusRect{rect.x() - offset(), rect.y(), length() - sectionPosition(0), rect.height()};
        o.rect = style->visualRect(layoutDirection(), rect, focusRect);
        style->drawPrimitive(DStyle::PE_FrameFocusRect, &o, &painter);
    }
    QPainter painter(viewport());
    QPainterPath innerPath,outPath;
    innerPath.addRoundedRect(viewport()->rect(),8,8);
    outPath.addRect(viewport()->rect());
    outPath=outPath.subtracted(innerPath);
    painter.fillPath(outPath,DApplicationHelper::instance()->applicationPalette().color(DPalette::Active,DPalette::AlternateBase));
}

void BaseHeaderView::focusInEvent(QFocusEvent *event)
{
    DHeaderView::focusInEvent(event);
    m_focusReason =  event->reason();
}

// Event filter handler
bool BaseHeaderView::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::KeyPress) {
        QKeyEvent *kev = dynamic_cast<QKeyEvent *>(ev);
        if (kev->key() == Qt::Key_Space ||
                kev->key() == Qt::Key_Up ||
                kev->key() == Qt::Key_Down) {
            return true;
        }
    }
    return DHeaderView::eventFilter(obj, ev);
}
