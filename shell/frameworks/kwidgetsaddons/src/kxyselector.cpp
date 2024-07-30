/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Martin Jones <mjones@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kxyselector.h"

#include "loggingcategory.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionFrame>

//-----------------------------------------------------------------------------
/*
 * 2D value selector.
 * The contents of the selector are drawn by derived class.
 */

class KXYSelectorPrivate
{
public:
    KXYSelectorPrivate(KXYSelector *qq)
        : q(qq)
        , xPos(0)
        , yPos(0)
        , minX(0)
        , maxX(100)
        , minY(0)
        , maxY(100)
        , m_markerColor(Qt::white)
    {
    }

    void setValues(int _xPos, int _yPos);

    KXYSelector *const q;
    int px;
    int py;
    int xPos;
    int yPos;
    int minX;
    int maxX;
    int minY;
    int maxY;
    QColor m_markerColor;
};

KXYSelector::KXYSelector(QWidget *parent)
    : QWidget(parent)
    , d(new KXYSelectorPrivate(this))
{
}

KXYSelector::~KXYSelector() = default;

int KXYSelector::xValue() const
{
    return d->xPos;
}

int KXYSelector::yValue() const
{
    return d->yPos;
}

void KXYSelector::setRange(int _minX, int _minY, int _maxX, int _maxY)
{
    if (_maxX == _minX) {
        qCWarning(KWidgetsAddonsLog) << "KXYSelector::setRange invalid range: " << _maxX << " == " << _minX << " (for X) ";
        return;
    }
    if (_maxY == _minY) {
        qCWarning(KWidgetsAddonsLog) << "KXYSelector::setRange invalid range: " << _maxY << " == " << _minY << " (for Y) ";
        return;
    }

    int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    d->px = w;
    d->py = w;
    d->minX = _minX;
    d->minY = _minY;
    d->maxX = _maxX;
    d->maxY = _maxY;
}

void KXYSelector::setXValue(int _xPos)
{
    setValues(_xPos, d->yPos);
}

void KXYSelector::setYValue(int _yPos)
{
    setValues(d->xPos, _yPos);
}

void KXYSelector::setValues(int _xPos, int _yPos)
{
    d->setValues(_xPos, _yPos);
}

void KXYSelectorPrivate::setValues(int _xPos, int _yPos)
{
    int w = q->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    xPos = _xPos;
    yPos = _yPos;

    if (xPos > maxX) {
        xPos = maxX;
    } else if (xPos < minX) {
        xPos = minX;
    }

    if (yPos > maxY) {
        yPos = maxY;
    } else if (yPos < minY) {
        yPos = minY;
    }

    Q_ASSERT(maxX != minX);
    int xp = w + (q->width() - 2 * w) * xPos / (maxX - minX);

    Q_ASSERT(maxY != minY);
    int yp = q->height() - w - (q->height() - 2 * w) * yPos / (maxY - minY);

    q->setPosition(xp, yp);
}

void KXYSelector::setMarkerColor(const QColor &col)
{
    d->m_markerColor = col;
}

QRect KXYSelector::contentsRect() const
{
    int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    return rect().adjusted(w, w, -w, -w);
}

QSize KXYSelector::minimumSizeHint() const
{
    int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    return QSize(2 * w, 2 * w);
}

void KXYSelector::paintEvent(QPaintEvent * /* ev */)
{
    QStyleOptionFrame opt;
    opt.initFrom(this);

    QPainter painter;
    painter.begin(this);

    drawContents(&painter);
    drawMarker(&painter, d->px, d->py);

    style()->drawPrimitive(QStyle::PE_Frame, &opt, &painter, this);

    painter.end();
}

void KXYSelector::mousePressEvent(QMouseEvent *e)
{
    mouseMoveEvent(e);
}

void KXYSelector::mouseMoveEvent(QMouseEvent *e)
{
    int xVal;
    int yVal;
    int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    valuesFromPosition(e->pos().x() - w, e->pos().y() - w, xVal, yVal);
    setValues(xVal, yVal);

    Q_EMIT valueChanged(d->xPos, d->yPos);
}

void KXYSelector::wheelEvent(QWheelEvent *e)
{
    setValues(xValue() + e->angleDelta().x() / 120, yValue() + e->angleDelta().y() / 120);
    Q_EMIT valueChanged(d->xPos, d->yPos);
}

void KXYSelector::valuesFromPosition(int x, int y, int &xVal, int &yVal) const
{
    int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    xVal = ((d->maxX - d->minX) * (x - w)) / (width() - 2 * w);
    yVal = d->maxY - (((d->maxY - d->minY) * (y - w)) / (height() - 2 * w));

    if (xVal > d->maxX) {
        xVal = d->maxX;
    } else if (xVal < d->minX) {
        xVal = d->minX;
    }

    if (yVal > d->maxY) {
        yVal = d->maxY;
    } else if (yVal < d->minY) {
        yVal = d->minY;
    }
}

void KXYSelector::setPosition(int xp, int yp)
{
    int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    if (xp < w) {
        xp = w;
    } else if (xp > width() - w) {
        xp = width() - w;
    }

    if (yp < w) {
        yp = w;
    } else if (yp > height() - w) {
        yp = height() - w;
    }

    d->px = xp;
    d->py = yp;

    update();
}

void KXYSelector::drawContents(QPainter *)
{
}

void KXYSelector::drawMarker(QPainter *p, int xp, int yp)
{
    QPen pen(d->m_markerColor);
    p->setPen(pen);

    /*
      p->drawLine( xp - 6, yp - 6, xp - 2, yp - 2 );
      p->drawLine( xp - 6, yp + 6, xp - 2, yp + 2 );
      p->drawLine( xp + 6, yp - 6, xp + 2, yp - 2 );
      p->drawLine( xp + 6, yp + 6, xp + 2, yp + 2 );
    */
    p->drawEllipse(xp - 4, yp - 4, 8, 8);
}

#include "moc_kxyselector.cpp"
