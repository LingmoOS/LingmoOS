// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "colorslider.h"

#include <QStyleOptionSlider>
#include <QDebug>
#include <QPainter>
#include <QStyle>

const int IMAGE_HEIGHT = 10;

ColorSlider::ColorSlider(QWidget *parent)
    : QSlider(parent)
{
    setMinimum(0);
    setMaximum(359);
    setOrientation(Qt::Horizontal);
    QRect rect = this->rect();

    m_backgroundImage = QImage(rect.width(), IMAGE_HEIGHT, QImage::Format_ARGB32);

    for (qreal s = 0; s < m_backgroundImage.width(); s++) {
        for (qreal v = 1; v <= m_backgroundImage.height() ; v++) {
            QColor penColor = getColor(qreal(s / rect.width() * maximum()), 1, 1);

            if (!penColor.isValid()) {
                continue;
            }

            m_backgroundImage.setPixelColor(std::min(int(s), rect.width()), m_backgroundImage.height() - int(v), penColor);
        }
    }
}

ColorSlider::~ColorSlider()
{
}

//h∈(0, 360), s∈(0, 1), v∈(0, 1)
QColor ColorSlider::getColor(qreal h, qreal s, qreal v)
{
    int hi = int(h / 60) % 6;
    qreal f = h / 60 - hi;

    qreal p = v * (1 - s);
    qreal q = v * (1 - f * s);
    qreal t = v * (1 - (1 - f) * s);

    if (q < 0) {
        q = 0;
    }

    QColor color;

    if (hi == 0) {
        color =  QColor(std::min(int(255 * p), 255), std::min(int(255 * q), 255), std::min(int(255 * v), 255));
    } else if (hi == 1) {
        color =  QColor(std::min(int(255 * t), 255), std::min(int(255 * p), 255), std::min(int(255 * v), 255));
    } else if (hi == 2) {
        color =  QColor(std::min(int(255 * v), 255), std::min(int(255 * p), 255), int(255 * q));
    } else if (hi == 3) {
        color = QColor(std::min(int(255 * v), 255), std::min(int(255 * t), 255), std::min(int(255 * p), 255));
    } else if (hi == 4) {
        color = QColor(std::min(int(255 * q), 255), std::min(int(255 * v), 255), std::min(int(255 * p), 255));
    } else {
        color = QColor(std::min(int(255 * p), 255), std::min(int(255 * v), 255), std::min(int(255 * t), 255));
    }

    return color;
}

void ColorSlider::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev)

    QRect rect = this->rect();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawImage(QRect(rect.x(), rect.y() + 2, rect.width(),
                            IMAGE_HEIGHT), m_backgroundImage);

    const int offset = 2;
    const qreal k = qreal(value() - minimum()) / qreal(maximum() - minimum());
    qreal x = ((rect.width() - 2 * offset) * k) + offset;

    QPen pen;
    pen.setWidth(1);
    pen.setColor(QColor(0, 0, 0, 51));
    painter.setPen(pen);
    painter.setBrush(QBrush(Qt::white));
    painter.drawRect(QRectF(QPointF(x - offset, rect.top()), QPointF(x + offset, rect.bottom())));
}
