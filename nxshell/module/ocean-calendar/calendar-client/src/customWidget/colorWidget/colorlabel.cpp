// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "colorlabel.h"

#include <QImage>
#include <QRgb>
#include <QBitmap>

#include <cmath>

ColorLabel::ColorLabel(DWidget *parent)
    : DLabel(parent)
    , m_pressed(false)
{
    setMouseTracking(true);
    m_dotCursor = pickColorCursor();
}

//h∈(0, 360), s∈(0, 1), v∈(0, 1)
QColor ColorLabel::getColor(qreal h, qreal s, qreal v)
{
    int hi = int(h / 60) % 6;
    qreal f = h / 60 - hi;

    qreal p = v * (1 - s);
    qreal q = v * (1 - f * s);
    qreal t = v * (1 - (1 - f) * s);

    if (hi == 0) {
        return QColor(std::min(int(255 * p), 255), std::min(int(255 * q), 255), std::min(int(255 * v), 255));
    } else if (hi == 1) {
        return QColor(std::min(int(255 * t), 255), std::min(int(255 * p), 255), std::min(int(255 * v), 255));
    } else if (hi == 2) {
        return QColor(std::min(int(255 * v), 255), std::min(int(255 * p), 255), int(255 * q));
    } else if (hi == 3) {
        return QColor(std::min(int(255 * v), 255), std::min(int(255 * t), 255), std::min(int(255 * p), 255));
    } else if (hi == 4) {
        return QColor(std::min(int(255 * q), 255), std::min(int(255 * v), 255), std::min(int(255 * p), 255));
    } else {
        return QColor(std::min(int(255 * p), 255), std::min(int(255 * v), 255), std::min(int(255 * t), 255));
    }

}

void ColorLabel::setHue(int hue)
{
    m_hue = hue;
    update();
}

void ColorLabel::pickColor(QPoint pos, bool picked)
{
    if (!rect().contains(pos)) {
        return;
    }

    QPixmap pickPixmap;
    pickPixmap = this->grab(QRect(0, 0, this->width(), this->height()));
    QImage pickImg = pickPixmap.toImage();

    if (!pickImg.isNull()) {
        QRgb pickRgb = pickImg.pixel(pos);
        m_pickedColor = QColor(qRed(pickRgb), qGreen(pickRgb), qBlue(pickRgb));
    } else {
        m_pickedColor = QColor(0, 0, 0);
    }

    if (picked) {
        emit signalpickedColor(m_pickedColor);
    } else {
        emit signalPreViewColor(m_pickedColor);
    }
}

void ColorLabel::paintEvent(QPaintEvent *)
{
    if (m_entered) {
        setCursor(m_dotCursor);
    } else {
        setCursor(QCursor());
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QImage backgroundImage(this->width(), this->height(), QImage::Format_ARGB32);
    QColor penColor;
    for (qreal s = 0; s < this->width(); s++) {
        for (qreal v = 0; v < this->height(); v++) {
            penColor = getColor(m_hue, s / this->width(), v / this->height());
            if (!penColor.isValid()) {
                continue;
            }
            backgroundImage.setPixelColor(int(s), this->height() - 1 - int(v), penColor);
        }
    }
    QPixmap pm = QPixmap::fromImage(backgroundImage);
    QSize size(this->width(), this->height());
    QBitmap mask(size);
    QPainter painterMask(&mask);
    painterMask.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painterMask.fillRect(0, 0, size.width(), size.height(), Qt::white);
    painterMask.setBrush(QColor(0, 0, 0));
    painterMask.drawRoundedRect(0, 0, size.width(), size.height(), 5, 5);

    QPixmap image = pm.scaled(size);
    image.setMask(mask);
    painter.drawPixmap(this->rect(),image);

}

void ColorLabel::mousePressEvent(QMouseEvent *e)
{
    m_pressed = true;
    pickColor(e->pos(), m_pressed);
    QLabel::mousePressEvent(e);
}

void ColorLabel::mouseMoveEvent(QMouseEvent *e)
{
    if (rect().contains(e->pos())) {
        m_entered = true;
    } else {
        m_entered = false;
    }
    update();
    pickColor(e->pos(), m_pressed);
    //移动事件不传递到父控件中
    e->accept();
}

void ColorLabel::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_pressed && rect().contains(e->pos())) {
        emit clicked();
    }
    m_pressed = false;
    QLabel::mouseReleaseEvent(e);
}

QCursor ColorLabel::pickColorCursor()
{
    int tipWidth = 11;
    QPixmap cursorPix = QPixmap(QSize(tipWidth, tipWidth));
    cursorPix.fill(QColor(Qt::transparent));

    QPen whitePen;
    whitePen.setWidth(1);
    whitePen.setCapStyle(Qt::FlatCap);
    whitePen.setJoinStyle(Qt::RoundJoin);
    whitePen.setColor(QColor(255, 255, 255, 255));

    QPen blackPen;
    blackPen.setWidth(1);
    blackPen.setCapStyle(Qt::FlatCap);
    blackPen.setJoinStyle(Qt::RoundJoin);
    blackPen.setColor(QColor(0, 0, 0, 125));

    QPainter painter(&cursorPix);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(blackPen);
    painter.drawEllipse(cursorPix.rect().center(), tipWidth / 2 - 1, tipWidth / 2 - 1);
    painter.drawEllipse(cursorPix.rect().center(), tipWidth / 2 - 3, tipWidth / 2 - 3);
    painter.setPen(whitePen);
    painter.drawEllipse(cursorPix.rect().center(), tipWidth / 2 - 2, tipWidth / 2 - 2);

    return QCursor(cursorPix, -1, -1);
}

