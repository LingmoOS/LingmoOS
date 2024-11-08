/*
* Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include <QPainterPath>

#include "noteHead.h"
#include "ui_noteHead.h"
#include "paletteWidget.h"

noteHead::noteHead(QWidget *parent) :
    QWidget(parent)
  , colorWidget(PaletteWidget::LINGMO_BLUE)
  , ui(new Ui::noteHead)
{
    ui->setupUi(this);
    setMinimumSize(250, 8);
}

noteHead::~noteHead()
{
    delete ui;
}

void noteHead::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QPainterPath rectPath;
    rectPath.addRect(this->rect());
    p.fillPath(rectPath,QBrush(colorWidget));
    QWidget::paintEvent(event);
}
