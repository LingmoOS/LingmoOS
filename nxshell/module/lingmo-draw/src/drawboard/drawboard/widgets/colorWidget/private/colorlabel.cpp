// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "colorlabel.h"

#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QImage>
#include <QRgb>

#include <cmath>

#include "utils/baseutils.h"


ColorLabel::ColorLabel(QWidget *parent)
    : QLabel(parent)
    , m_workToPick(true)
    , m_picking(true)
    , m_pressed(false)
    , m_tipPoint(this->rect().center())
{
    setMouseTracking(true);
    connect(this, &ColorLabel::clicked, this, [ = ] {
        if (m_picking && m_workToPick)
        {
            pickColor(m_clickedPos, true);
        }
    });
    //setCursor(pickColorCursor());
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
    //calImage();
    auto rs = getColorPos(m_pickedColor);

    m_clickedPos = QPoint();
    if (std::get<0>(rs)) {
        m_clickedPos = std::get<1>(rs);
    }

    update();
}

void ColorLabel::pickColor(QPoint pos, bool picked)
{
    if (pos.x() < 0 || pos.y() < 0 || pos.x() >= this->width() || pos.y() >= this->height()) {
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
        emit pickedColor(m_pickedColor);
    } else {
        emit signalPreViewColor(m_pickedColor);
    }
}

void ColorLabel::setSelectColor(QColor c)
{
    m_pickedColor = c;
    auto r = getColorPos(c);
    if (std::get<0>(r)) {
        m_clickedPos = std::get<1>(r);
    } else {
        m_clickedPos = QPoint();
    }
    this->update();
}

QSize ColorLabel::sizeHint() const
{
    return QSize(200, 200);
}

QSize ColorLabel::minimumSizeHint() const
{
    return QSize(200, 140);
}

void ColorLabel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    calImage();
    painter.drawImage(this->rect(), backgroundImage);

    if (m_clickedPos != QPoint()) {
        QImage image = pickColorCursor().pixmap().toImage();
        QPoint offset = QPoint(image.size().width() / 2, image.size().height() / 2);
        painter.drawImage(m_clickedPos - offset, pickColorCursor().pixmap().toImage());
    }
}

void ColorLabel::resizeEvent(QResizeEvent *event)
{
    //calImage();
    QLabel::resizeEvent(event);
}

void ColorLabel::enterEvent(QEvent *e)
{

    if (!m_workToPick)
        return;

    m_lastCursor = this->cursor();
    QLabel::enterEvent(e);
}

void ColorLabel::leaveEvent(QEvent *e)
{

    if (!m_workToPick)
        return;

    QLabel::leaveEvent(e);
}

void ColorLabel::mousePressEvent(QMouseEvent *e)
{
    if (!m_workToPick)
        return ;

    m_pressed = true;
    m_tipPoint = this->mapFromGlobal(cursor().pos());
    QLabel::mousePressEvent(e);
}

void ColorLabel::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_workToPick)
        return;

    if (m_pressed) {
        m_tipPoint = this->mapFromGlobal(cursor().pos());
        pickColor(m_tipPoint, false);
    }
    update();
    QLabel::mouseMoveEvent(e);
}

void ColorLabel::mouseReleaseEvent(QMouseEvent *e)
{

    if (m_pressed) {
        m_clickedPos = e->pos();
        emit clicked();
    }

    m_pressed = false;
    QLabel::mouseReleaseEvent(e);
    update();

}

void ColorLabel::calImage()
{
    QColor penColor;
    backgroundImage = QImage(this->width(), this->height(), QImage::Format_ARGB32);
    for (int s = 0; s < this->width(); ++s) {
        for (int v = 0; v < this->height(); ++v) {
            penColor = getColor(m_hue, qreal(s) / this->width(), qreal(v) / this->height());
            if (!penColor.isValid()) {
                continue;
            }
            backgroundImage.setPixelColor(s, this->height() - 1 - v, penColor);
        }
    }
}

std::tuple<bool, QPoint> ColorLabel::getColorPos(QColor c)
{
    QColor penColor;
    bool bfind = false;
    QPoint pos;

    QPixmap pickPixmap;
    pickPixmap = this->grab(QRect(0, 0, this->width(), this->height()));
    QImage pickImg = pickPixmap.toImage();

    if (!pickImg.isNull() && pickImg.width() == width() && pickImg.height() == height()) {
        for (int w = 0; w < width(); w++) {
            for (int h = 0; h < height(); h++) {
                pos.setX(w);
                pos.setY(h);
                QRgb pickRgb = pickImg.pixel(pos);
                QColor cc = QColor(qRed(pickRgb), qGreen(pickRgb), qBlue(pickRgb));
                if (cc == c) {
                    bfind = true;
                    break;
                }
            }
            if (bfind)
                break;
        }
    }

    return std::make_tuple(bfind, pos);
}

ColorLabel::~ColorLabel()
{
}
