// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "roundedbackground.h"

#include <QPainter>
#include <QPainterPath>
#include <DPaletteHelper>

#define RADIUS 8

using namespace GrandSearch;
DWIDGET_USE_NAMESPACE

RoundedBackground::RoundedBackground(QWidget *parent) : QWidget(parent)
{

}

void RoundedBackground::paintEvent(QPaintEvent *event)
{
    const DPalette &dp = DPaletteHelper::instance()->palette(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(dp.brush(DPalette::ItemBackground));

    QRect roundRect(0, 0, RADIUS * 2, RADIUS * 2);
    QPainterPath path;
    int w = width();
    int h = height();

    if (m_topRound) {
        // 左上圆角
        path.moveTo(0, RADIUS);
        roundRect.moveTo(0, 0);
        path.arcTo(roundRect, 180, -90);

        // 右上圆角
        path.lineTo(w - RADIUS, 0);
        roundRect.moveTo(w - roundRect.width(), 0);
        path.arcTo(roundRect, 90, -90);
    } else {
        path.moveTo(0, 0);
        path.lineTo(w, 0);
    }

    if (m_bottomRound) {
        // 右下圆角
        path.lineTo(w, h - RADIUS);
        roundRect.moveTo(w - roundRect.width(), h - roundRect.width());
        path.arcTo(roundRect, 0, -90);

        // 左下圆角
        path.lineTo(RADIUS, h);
        roundRect.moveTo(0, h - roundRect.height());
        path.arcTo(roundRect, 270, -90);
    } else {
        path.lineTo(w, h);
        path.lineTo(0, h);
    }

    path.closeSubpath();
    p.drawPath(path);

    return DWidget::paintEvent(event);
}
