// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "colorbutton.h"

#include <QPainter>
#include <QDebug>

const QSize BUTTON_SIZE = QSize(16, 16);
//const int  ELLIPSE_MARGIN = 1;
ColorButton::ColorButton(QColor bgColor, DWidget *parent)
    : DPushButton(parent)
{
    setFixedSize(BUTTON_SIZE);
    setCheckable(true);
    m_bgColor = bgColor;

    connect(this, &ColorButton::clicked, this, &ColorButton::setColorBtnChecked);
}

void ColorButton::setColorBtnChecked()
{
    update();
    if (this->isChecked()) {
        qDebug() << "updatePaintColor:" << m_bgColor;
        emit updatePaintColor(m_bgColor);
    }
}
ColorButton::~ColorButton() {}
