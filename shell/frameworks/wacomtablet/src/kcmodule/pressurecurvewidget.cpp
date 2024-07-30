/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pressurecurvewidget.h"

// Qt includes
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QTabletEvent>

using namespace Wacom;

PressureCurveWidget::PressureCurveWidget(QWidget *parent)
    : QWidget(parent)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void PressureCurveWidget::setControlPoints(qreal p1, qreal p2, qreal p3, qreal p4)
{
    // change y values upside down (xsetwacom has 0,0 in the lower left QWidget in the upper left)
    p2 = 100 - p2;
    p4 = 100 - p4;
    m_cP1 = QPointF((p1 / 100.0) * width(), (p2 / 100.0) * height());
    m_cP2 = QPointF((p3 / 100.0) * width(), (p4 / 100.0) * height());
}

void PressureCurveWidget::mousePressEvent(QMouseEvent *event)
{
    setNearestPoint(event->localPos());
}

void PressureCurveWidget::mouseMoveEvent(QMouseEvent *event)
{
    moveControlPoint(event->localPos());
    update();
}

void PressureCurveWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_activePoint = 0;
}

void PressureCurveWidget::resizeEvent(QResizeEvent *event)
{
    // avoid unpredictable ratio on first initialisation
    if (event->oldSize().width() == -1 || event->oldSize().width() == 0 || event->oldSize().height() == 0) {
        return;
    }

    const qreal xRatio = static_cast<qreal>(event->size().width()) / event->oldSize().width();
    const qreal yRatio = static_cast<qreal>(event->size().height()) / event->oldSize().height();

    m_cP1.setX(m_cP1.x() * xRatio);
    m_cP1.setY(m_cP1.y() * yRatio);
    m_cP2.setX(m_cP2.x() * xRatio);
    m_cP2.setY(m_cP2.y() * yRatio);
}

void PressureCurveWidget::tabletEvent(QTabletEvent *event)
{
    event->accept();
    m_pressure = event->pressure();

    constexpr qreal threshold = 0.001;
    if (m_pressure <= threshold) {
        m_activePoint = 0;
    }

    if (m_activePoint > 0) {
        moveControlPoint(event->pos());
    } else if (m_pressure > threshold) {
        setNearestPoint(event->pos());
    }

    update();
}

void PressureCurveWidget::setNearestPoint(const QPointF &pos)
{
    qreal d1 = QLineF(pos, m_cP1).length();
    qreal d2 = QLineF(pos, m_cP2).length();

    if (d1 < m_pointSize) {
        m_activePoint = 1;
    } else if (d2 < m_pointSize) {
        m_activePoint = 2;
    }
}

void PressureCurveWidget::moveControlPoint(const QPointF &pos)
{
    int x;
    int y;

    if (pos.x() > width()) {
        x = width();
    } else if (pos.x() < 0) {
        x = 0;
    } else {
        x = qRound(pos.x());
    }

    if (pos.y() > height()) {
        y = height();
    } else if (pos.y() < 0) {
        y = 0;
    } else {
        y = qRound(pos.y());
    }

    switch (m_activePoint) {
    case 1:
        m_cP1 = QPoint(x, y);
        m_cP2 = QPoint(y, x); // seems xsetwacom is expecting to be both controllpoints the same with switches xy values
        // makes no sense to me, but otherwise the wacom driver complains
        // the wacom_utility does it the same way though
        break;
    case 2:
        m_cP2 = QPoint(x, y);
        m_cP1 = QPoint(y, x); // see above
        break;
    }

    // build string with controlpoints as used in xsetwacom settings
    int p1 = qRound((m_cP1.x() / width()) * 100.0);
    int p2 = qRound((m_cP1.y() / height()) * 100.0);
    int p3 = qRound((m_cP2.x() / width()) * 100.0);
    int p4 = qRound((m_cP2.y() / height()) * 100.0);

    // change y values upside down
    p2 = 100 - p2;
    p4 = 100 - p4;
    QString pointsString = QString::fromLatin1("%1 %2 %3 %4").arg(p1).arg(p2).arg(p3).arg(p4);

    emit controlPointsChanged(pointsString);
}

void PressureCurveWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.translate(+0.5, +0.5);

    // draw the background grid
    int yStep = height() / 10;
    int xStep = width() / 10;
    painter.setPen(QColor(Qt::gray));

    for (int i = 1; i < 10; i++) {
        painter.drawLine(i * xStep, 0, i * xStep, height());
        painter.drawLine(0, i * yStep, width(), i * yStep);
    }

    // draw controllpoint lines
    painter.setPen(QColor(Qt::gray));
    painter.drawLine(m_cP1, QPoint(0, height()));
    painter.drawLine(m_cP2, QPoint(width(), 0));

    // create presscurve
    QPainterPath curvePath;
    curvePath.moveTo(0, height());
    curvePath.cubicTo(m_cP1, m_cP2, QPoint(width(), 0));

    // create polygon for the area below the curve
    QPainterPath areaBelowCurve(curvePath);
    areaBelowCurve.lineTo(width(), height());
    areaBelowCurve.lineTo(0, height());
    QRectF clearRect(m_pressure * width(), 0, width(), height());
    QPainterPath subtract;
    subtract.addRect(clearRect);

    // draw below curve area
    painter.setPen(QPen());
    painter.setBrush(QColor(0, 102, 255));
    painter.drawPath(areaBelowCurve.subtracted(subtract));

    // draw presscurve
    QPen curvePen;
    curvePen.setWidth(2);
    curvePen.setColor(m_curveColor);
    painter.setPen(curvePen);
    painter.setBrush(QBrush());
    painter.drawPath(curvePath);

    // draw controllpoints
    painter.setPen(QColor(226, 8, 0));
    painter.setBrush(QColor(226, 8, 0));
    painter.drawEllipse(m_cP1, m_pointSize, m_pointSize);
    painter.drawEllipse(m_cP2, m_pointSize, m_pointSize);
}

#include "moc_pressurecurvewidget.cpp"
