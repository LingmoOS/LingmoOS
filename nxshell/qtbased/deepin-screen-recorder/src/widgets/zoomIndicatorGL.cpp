// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "zoomIndicatorGL.h"
#include "../utils/baseutils.h"
#include "../utils/tempfile.h"

#include <QCursor>
#include <QTextOption>
#include <QDebug>
#include <QRgb>

namespace {
const QSize BACKGROUND_SIZE = QSize(59, 59);
//const int SCALE_VALUE = 4;
const int IMG_WIDTH =  12;
const int INDICATOR_WIDTH = 49;
const int CENTER_RECT_WIDTH = 12;
const int BOTTOM_RECT_HEIGHT = 14;
}
ZoomIndicatorGL::ZoomIndicatorGL(DWidget *parent)
    : QOpenGLWidget(parent)
{
    setFixedSize(BACKGROUND_SIZE);
//    setStyleSheet(getFileContent(":/resources/qss/zoomindicator.qss"));
    setAttribute(Qt::WA_TransparentForMouseEvents);
//    QString t_zoomStyle = QString("border-image: url(:/resources/images/action/magnifier.png);");
//    this->setStyleSheet(t_zoomStyle);

    m_centerRect = QRect((BACKGROUND_SIZE.width() - CENTER_RECT_WIDTH) / 2 + 1,
                         (BACKGROUND_SIZE.width() - CENTER_RECT_WIDTH) / 2 + 1,
                         CENTER_RECT_WIDTH, CENTER_RECT_WIDTH);

    m_globalRect = QRect(-4, -4, BACKGROUND_SIZE.width() + 8, BACKGROUND_SIZE.height() + 8);
}

ZoomIndicatorGL::~ZoomIndicatorGL() {}


void ZoomIndicatorGL::paintGL()
{
//    using namespace utils;
    QPoint centerPos =  this->cursor().pos();
    centerPos = QPoint(std::max(centerPos.x() - this->window()->x(), 0),
                       std::max(centerPos.y() - this->window()->y(), 0));

    QPainter painter(this);
    const QPixmap &fullscreenImgFile = TempFile::instance()->getFullscreenPixmap();
    qreal ration = this->devicePixelRatioF();
    QImage fullscreenImg = fullscreenImgFile.toImage();
    fullscreenImg =  fullscreenImg.scaled(static_cast<int>(fullscreenImg.width() / ration),
                                          static_cast<int>(fullscreenImg.height() / ration), Qt::KeepAspectRatio);
    const QRgb centerRectRgb = fullscreenImg.pixel(centerPos);
    QPixmap zoomPix = QPixmap(fullscreenImgFile).scaled(
                          fullscreenImg.width(), fullscreenImg.height()).copy(
                          centerPos.x() - IMG_WIDTH / 2, centerPos.y() - IMG_WIDTH / 2, IMG_WIDTH, IMG_WIDTH);

    zoomPix = zoomPix.scaled(QSize(INDICATOR_WIDTH,  INDICATOR_WIDTH),
                             Qt::KeepAspectRatio);

    painter.drawPixmap(QRect(0, 0, INDICATOR_WIDTH + 10, INDICATOR_WIDTH + 10), zoomPix);


    painter.drawPixmap(m_centerRect, QPixmap(":/images/action/center_rect.png"));
    painter.drawPixmap(m_globalRect, QPixmap(":/images/action/magnifier.png"));

    m_lastCenterPosBrush = QBrush(QColor(qRed(centerRectRgb),
                                         qGreen(centerRectRgb), qBlue(centerRectRgb)));
    painter.fillRect(QRect(INDICATOR_WIDTH / 2 + 2, INDICATOR_WIDTH / 2 + 2,
                           CENTER_RECT_WIDTH - 4, CENTER_RECT_WIDTH - 4), m_lastCenterPosBrush);

    painter.fillRect(QRect(2, INDICATOR_WIDTH - 8, INDICATOR_WIDTH + 6, BOTTOM_RECT_HEIGHT),
                     QBrush(QColor(0, 0, 0, 125)));
    QFont posFont;
    posFont.setPixelSize(9);
    painter.setFont(posFont);
    painter.setPen(QColor(Qt::white));
    QTextOption posTextOption;
    posTextOption.setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    painter.drawText(QRectF(5, INDICATOR_WIDTH - 9, INDICATOR_WIDTH, INDICATOR_WIDTH),
                     QString("%1, %2").arg(centerPos.x()).arg(centerPos.y()), posTextOption);
}

void ZoomIndicatorGL::showMagnifier(QPoint pos)
{
    this->show();
    this->move(pos);
    update();
}
