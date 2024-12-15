// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconbutton.h"

#include <QPainter>
#include <QPainterPath>

IconButton::IconButton(QWidget *parent)
    : QPushButton (parent)
{
    setAccessibleName("modeSwitchButton");
    setCheckable(true);
}

void IconButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);

    QPainterPath path;
    if (isChecked()) {
        QColor brushColor(Qt::white);
        brushColor.setAlpha(static_cast<int>(0.7 * 255));

        painter.setBrush(brushColor);
        painter.drawRoundedRect(rect().marginsRemoved(QMargins(1, 1, 1, 1)), 8, 8);
    }

    return QPushButton::paintEvent(event);
}
