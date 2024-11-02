/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kprogressbar.h"

#include <QPainter>
#include <QPainterPath>
#include <QDebug>

KProgressBar::KProgressBar(QWidget *parent, QString barType)
    : QWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    m_curStatus = STATUS_NORMAL;
    m_rMaxValue = 100;
    m_rCurValue = 0;
    m_rThresholdHigh = m_rMaxValue+1;
    m_rThresholdVHigh = m_rMaxValue+1;
    m_colorNormal = QColor("#F9C53D");
    m_colorHigh = QColor("#F68C27");
    m_colorVHigh = QColor("#F3222D");
    m_colorLeft = QColor("#52C429");
    m_barType = barType;
}

void KProgressBar::setThresholdValue(qreal rHigh, qreal rVHigh)
{
    if (rHigh > rVHigh) {
        rHigh = rVHigh;
    }
    m_rThresholdHigh = rHigh;
    m_rThresholdVHigh = rVHigh;
    if (m_rCurValue >= m_rThresholdVHigh) {
        m_curStatus = STATUS_VHIGH;
    } else if (m_rCurValue >= m_rThresholdHigh) {
        m_curStatus = STATUS_HIGH;
    } else {
        m_curStatus = STATUS_NORMAL;
    }
    repaint();
}

void KProgressBar::setColorAttr(QColor colorNormal, QColor colorLeft, QColor colorHigh, QColor colorVHigh)
{
    m_colorNormal = colorNormal;
    m_colorLeft = colorLeft;
    m_colorHigh = colorHigh;
    m_colorVHigh = colorVHigh;
}

void KProgressBar::drawRoundedRect(QPainter *pa, const QRect &rect,
                                    qreal xRadius, qreal yRadius, Corners corners, Qt::SizeMode mode) const
{
    QRectF r = rect.normalized();

    if (r.isNull())
        return;

    if (mode == Qt::AbsoluteSize) {
        qreal w = r.width() / 2;
        qreal h = r.height() / 2;

        if (w == 0) {
            xRadius = 0;
        } else {
            xRadius = 100 * qMin(xRadius, w) / w;
        }
        if (h == 0) {
            yRadius = 0;
        } else {
            yRadius = 100 * qMin(yRadius, h) / h;
        }
    } else {
        if (xRadius > 100)                          // fix ranges
            xRadius = 100;

        if (yRadius > 100)
            yRadius = 100;
    }

    if (xRadius <= 0 || yRadius <= 0) {             // add normal rectangle
        pa->drawRect(r);
        return;
    }

    QPainterPath path;
    qreal x = r.x();
    qreal y = r.y();
    qreal w = r.width();
    qreal h = r.height();
    qreal rxx2 = w * xRadius / 100;
    qreal ryy2 = h * yRadius / 100;

    path.arcMoveTo(x, y, rxx2, ryy2, 180);

    if (corners & TopLeftCorner) {
        path.arcTo(x, y, rxx2, ryy2, 180, -90);
    } else {
        path.lineTo(r.topLeft());
    }

    if (corners & TopRightCorner) {
        path.arcTo(x + w - rxx2, y, rxx2, ryy2, 90, -90);
    } else {
        path.lineTo(r.topRight());
    }

    if (corners & BottomRightCorner) {
        path.arcTo(x + w - rxx2, y + h - ryy2, rxx2, ryy2, 0, -90);
    } else {
        path.lineTo(r.bottomRight());
    }

    if (corners & BottomLeftCorner) {
        path.arcTo(x, y + h - ryy2, rxx2, ryy2, 270, -90);
    } else {
        path.lineTo(r.bottomLeft());
    }

    path.closeSubpath();
    pa->drawPath(path);
}

void KProgressBar::setMaxValue(qreal rMaxValue)
{
    m_rMaxValue = rMaxValue;
    repaint();
}

void KProgressBar::setCurValue(qreal rCurValue)
{
    if (rCurValue > m_rMaxValue) {
        rCurValue = m_rMaxValue;
    }
    m_rCurValue = rCurValue;
    if (m_rCurValue >= m_rThresholdVHigh) {
        m_curStatus = STATUS_VHIGH;
    } else if (m_rCurValue >= m_rThresholdHigh) {
        m_curStatus = STATUS_HIGH;
    } else {
        m_curStatus = STATUS_NORMAL;
    }
    repaint();
}

qreal KProgressBar::getCurValue()
{
    return m_rCurValue;
}

qreal KProgressBar::getMaxValue()
{
    return m_rMaxValue;
}

void KProgressBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setOpacity(1);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);
    // left
    QPainterPath pathLeft;
    pathLeft.addRoundedRect(this->rect(), 4, 4);
    painter.setBrush(m_colorLeft);
    painter.drawPath(pathLeft);
    // used
    if (m_barType == "filesystem") {
        painter.setBrush(this->palette().highlight());
    } else {
        if (m_curStatus == STATUS_VHIGH) {
            painter.setBrush(m_colorVHigh);
        } else if (m_curStatus == STATUS_HIGH) {
            painter.setBrush(m_colorHigh);
        } else {
            painter.setBrush(m_colorNormal);
        }
    }
    // calc
    QRect rectUsed = this->rect();
    unsigned uUsedLenth = rectUsed.width()*m_rCurValue/m_rMaxValue;
    rectUsed.setWidth(uUsedLenth);
    QPainterPath pathUsed;
    unsigned uSubLens = this->rect().width()-uUsedLenth;
    if (uSubLens<=4) {
        pathUsed.addRoundedRect(rectUsed, 4, 4);
        painter.drawPath(pathUsed);
    } else {
        drawRoundedRect(&painter, rectUsed, 4, 4, (Corners)(TopLeftCorner|BottomLeftCorner));
    }
}
