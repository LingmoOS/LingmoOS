// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bottombutton.h"

BottomButton::BottomButton(DWidget *parent)
    : DImageButton(parent)
{
    setCheckable(true);
    setFixedSize(QSize(85,30));
}

void BottomButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QFont font;
    font.setFamily("SourceHanSansSC");
    font.setStyleName("Medium");
    font.setPixelSize(14);

    QFontMetrics fm(font);
    QRect rec = fm.boundingRect(m_text);
    setFixedWidth(rec.width());

    QPainter painter(this);
    painter.setFont(font);
    painter.setPen(QPen(QColor(0,129,255)));
    painter.drawText(rect(), m_text);
}

void BottomButton::setText(const QString text)
{
    m_text = text;
}

void BottomButton::setRect(const QRect rect)
{
    m_rect = rect;
}
