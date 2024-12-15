// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "roundscrollarea.h"

#include <QPainter>
#include <QPainterPath>

#include <DPalette>
#include <DPaletteHelper>

DWIDGET_USE_NAMESPACE

RoundScrollArea::RoundScrollArea(QWidget *parent)
    : QScrollArea(parent)
    , m_radius(18)
{
    DPalette pa = DPaletteHelper::instance()->palette(this);
    pa.setBrush(DPalette::Window, Qt::transparent);
    DPaletteHelper::instance()->setPalette(this, pa);
}

void RoundScrollArea::setRadius(int radius)
{
    m_radius = radius;
    update();
}

void RoundScrollArea::paintEvent(QPaintEvent *e)
{
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(viewport()->rect(), m_radius, m_radius);
    painter.setClipPath(path);
    painter.fillPath(path, palette().window());
}
