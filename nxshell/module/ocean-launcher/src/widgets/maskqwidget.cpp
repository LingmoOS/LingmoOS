// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "maskqwidget.h"
#include "constants.h"

#include <QPainter>

MaskQWidget::MaskQWidget(QWidget *parent)
    : QWidget(parent)
    , m_color(Qt::transparent)
{
}

void MaskQWidget::setColor(QColor color)
{
    m_color = color;
}

void MaskQWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setBrush(QBrush(m_color));
    painter.setPen(Qt::transparent);
    QRect rect = this->rect();
    rect.setWidth(rect.width());
    rect.setHeight(rect.height() - 7);
    painter.drawRoundedRect(rect, DLauncher::APPHBOX_RADIUS, DLauncher::APPHBOX_RADIUS);
    QWidget::paintEvent(event);
}
