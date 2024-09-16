// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toptips.h"
#include "../utils/baseutils.h"

#include <DPalette>
#include <QImage>
#include <QSizePolicy>
#include <QDebug>
#include <QApplication>
#include <QScreen>

#include <cmath>

DGUI_USE_NAMESPACE

TopTips::TopTips(DWidget *parent)
    : QLabel(parent)
{
    setFixedSize(90, 30);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
//    this->setStyleSheet(" TopTips { background-color: transparent;"
//                        "border-image: url(:/resources/images/action/sizetip.png)  no-repeat;"
//                        "color: white;"
//                        "font-size: 12px;}");
//    this->setPixmap(QPixmap::fromImage(QImage(":/resources/images/action/sizetip.png")));

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    DPalette pa = this->palette();
    pa.setColor(DPalette::Foreground, Qt::white);
    this->setPalette(pa);
    this->setAlignment(Qt::AlignLeft | Qt::AlignBottom);


}

void TopTips::setContent(const QSize &size)
{
    QString text = QString("%1X%2").arg(size.width()).arg(size.height());
    if(m_showRecorderTips && size.width() * size.height() > 1920 * 1080 && size.width() != m_width && size.height() != m_height) {
        // 1920 / 1080 = w / h
        // w h 等比缩放
        setFixedSize(500, 30);
        int h = static_cast<int>(sqrt(1920.0 * 1080 * size.height() / size.width()));
        int w = static_cast<int>(sqrt(1920.0 * 1080 * size.width() / size.height()));
        QString recorderTips = tr(" Adjust the recording area within %1*%2 to get better video effect");
        setText(text + recorderTips.arg(w).arg(h));
    } else {
        setText(text);
    }
}

void TopTips::updateTips(QPoint pos, const QSize &size)
{
    if (!this->isVisible())
        this->show();

    QPoint startPoint = pos;
    qreal pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    QSize s = QSize(static_cast<int>(size.width()*pixelRatio),static_cast<int>(size.height()*pixelRatio));
    setContent(s);
    startPoint.setX(pos.x());

    if (pos.y() > this->height()) {
        startPoint.setY(pos.y() - this->height() - 3);
    } else {
        startPoint.setY(pos.y() + 3);
    }

    this->move(startPoint);
}

/*
void TopTips::mousePressEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);
    qDebug() << "";
    return;
}
*/

TopTips::~TopTips() {}

void TopTips::setRecorderTipsInfo(const bool showState)
{
    if(QSysInfo::currentCpuArchitecture().startsWith(QString("mips"))){
        m_showRecorderTips = showState;
    }
}
