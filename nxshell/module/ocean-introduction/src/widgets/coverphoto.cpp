// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "coverphoto.h"

CoverPhoto::CoverPhoto(QWidget *parent)
: DFrame(parent)
{

}

void CoverPhoto::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;

    update();
}

void CoverPhoto::paintEvent(QPaintEvent *event)
{
    DFrame::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRect pixRect(QPoint(0, 0), m_pixmap.size()/* * devicePixelRatioF()*/);

    // draw background
//    QPainterPath contentPath;
//    contentPath.addRoundedRect(pixRect, 10, 10);
//    painter.setClipPath(contentPath);
    painter.drawPixmap(pixRect, m_pixmap);
}
