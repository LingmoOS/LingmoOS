/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "oceandial.h"
#include <QGuiApplication>
#include <QPainter>

class OceanDialPrivate
{
    Q_DECLARE_PUBLIC(OceanDial)
    Q_DISABLE_COPY(OceanDialPrivate)
public:
    OceanDialPrivate(OceanDial *qq)
        : q_ptr(qq)
    {
    }
    OceanDial *const q_ptr;

    QFontMetricsF fontMetrics = QFontMetricsF(QGuiApplication::font());

    QColor backgroundColor;
    QColor backgroundBorderColor;
    QColor fillColor;
    QColor fillBorderColor;
    qreal angle = -140.0; // Range of QQuickDial::angle() is -140 to 140
    qreal grooveThickness = 0;
    bool notchesVisible = false;
};

OceanDial::OceanDial(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , d_ptr(new OceanDialPrivate(this))
{
    Q_D(OceanDial);
    connect(qGuiApp, &QGuiApplication::fontChanged, this, [this, d]() {
        d->fontMetrics = QFontMetricsF(QGuiApplication::font());
        update();
    });
}

OceanDial::~OceanDial() noexcept
{
}

void OceanDial::paint(QPainter *painter)
{
    Q_D(OceanDial);
    if (width() <= 0 || height() <= 0 || d->grooveThickness <= 0)
        return;

    QRectF paintRect;
    paintRect.setWidth(qMin(boundingRect().width() - d->grooveThickness, boundingRect().height() - d->grooveThickness));
    paintRect.setHeight(paintRect.width());
    paintRect.moveCenter(boundingRect().center());

    QPen backgroundBorderPen(d->backgroundBorderColor, d->grooveThickness, Qt::SolidLine, Qt::RoundCap);
    QPen backgroundPen(d->backgroundColor, d->grooveThickness - 2, Qt::SolidLine, Qt::RoundCap);
    QPen fillBorderPen(d->fillBorderColor, d->grooveThickness, Qt::SolidLine, Qt::RoundCap);
    QPen fillPen(d->fillColor, d->grooveThickness - 2, Qt::SolidLine, Qt::RoundCap);

    const qreal startAngle = -130 * 16;
    const qreal backgroundSpanAngle = -280 * 16;
    const qreal fillSpanAngle = (-d->angle - 140) * 16;

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(backgroundBorderPen);
    painter->drawArc(paintRect, startAngle, backgroundSpanAngle);
    painter->setPen(backgroundPen);
    painter->drawArc(paintRect, startAngle, backgroundSpanAngle);
    painter->setPen(fillBorderPen);
    painter->drawArc(paintRect, startAngle, fillSpanAngle);
    painter->setPen(fillPen);
    painter->drawArc(paintRect, startAngle, fillSpanAngle);
}

QColor OceanDial::backgroundBorderColor() const
{
    Q_D(const OceanDial);
    return d->backgroundBorderColor;
}

void OceanDial::setBackgroundBorderColor(const QColor &color)
{
    Q_D(OceanDial);
    if (d->backgroundBorderColor == color)
        return;

    d->backgroundBorderColor = color;
    update();
    Q_EMIT backgroundBorderColorChanged();
}

QColor OceanDial::backgroundColor() const
{
    Q_D(const OceanDial);
    return d->backgroundColor;
}

void OceanDial::setBackgroundColor(const QColor &color)
{
    Q_D(OceanDial);
    if (d->backgroundColor == color)
        return;

    d->backgroundColor = color;
    update();
    Q_EMIT backgroundColorChanged();
}

QColor OceanDial::fillBorderColor() const
{
    Q_D(const OceanDial);
    return d->fillBorderColor;
}

void OceanDial::setFillBorderColor(const QColor &color)
{
    Q_D(OceanDial);
    if (d->fillBorderColor == color)
        return;

    d->fillBorderColor = color;
    update();
    Q_EMIT fillBorderColorChanged();
}

QColor OceanDial::fillColor() const
{
    Q_D(const OceanDial);
    return d->fillColor;
}

void OceanDial::setFillColor(const QColor &color)
{
    Q_D(OceanDial);
    if (d->fillColor == color)
        return;

    d->fillColor = color;
    update();
    Q_EMIT fillColorChanged();
}

qreal OceanDial::angle() const
{
    Q_D(const OceanDial);
    return d->angle;
}

void OceanDial::setAngle(const qreal angle)
{
    Q_D(OceanDial);
    if (d->angle == angle)
        return;

    d->angle = angle;
    update();
    Q_EMIT angleChanged();
}

qreal OceanDial::grooveThickness() const
{
    Q_D(const OceanDial);
    return d->grooveThickness;
}

void OceanDial::setGrooveThickness(const qreal grooveThickness)
{
    Q_D(OceanDial);
    if (d->grooveThickness == grooveThickness)
        return;

    d->grooveThickness = grooveThickness;
    update();
    Q_EMIT grooveThicknessChanged();
}

bool OceanDial::notchesVisible() const
{
    Q_D(const OceanDial);
    return d->notchesVisible;
}

void OceanDial::setNotchesVisible(const bool notchesVisible)
{
    Q_D(OceanDial);
    if (d->notchesVisible == notchesVisible)
        return;

    d->notchesVisible = notchesVisible;
    update();
    Q_EMIT notchesVisibleChanged();
}

#include "moc_oceandial.cpp"
