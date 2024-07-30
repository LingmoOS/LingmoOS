/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Jason Harris <kstars@30doradus.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kplotpoint.h"

#include <QPointF>
#include <QtAlgorithms>

class KPlotPoint::Private
{
public:
    Private(KPlotPoint *qq, const QPointF &p, const QString &l, double bw)
        : q(qq)
        , point(p)
        , label(l)
        , barWidth(bw)
    {
    }

    KPlotPoint *q;

    QPointF point;
    QString label;
    double barWidth;
};

KPlotPoint::KPlotPoint()
    : d(new Private(this, QPointF(), QString(), 0.0))
{
}

KPlotPoint::KPlotPoint(double x, double y, const QString &label, double barWidth)
    : d(new Private(this, QPointF(x, y), label, barWidth))
{
}

KPlotPoint::KPlotPoint(const QPointF &p, const QString &label, double barWidth)
    : d(new Private(this, p, label, barWidth))
{
}

KPlotPoint::~KPlotPoint() = default;

QPointF KPlotPoint::position() const
{
    return d->point;
}

void KPlotPoint::setPosition(const QPointF &pos)
{
    d->point = pos;
}

double KPlotPoint::x() const
{
    return d->point.x();
}

void KPlotPoint::setX(double x)
{
    d->point.setX(x);
}

double KPlotPoint::y() const
{
    return d->point.y();
}

void KPlotPoint::setY(double y)
{
    d->point.setY(y);
}

QString KPlotPoint::label() const
{
    return d->label;
}

void KPlotPoint::setLabel(const QString &label)
{
    d->label = label;
}

double KPlotPoint::barWidth() const
{
    return d->barWidth;
}

void KPlotPoint::setBarWidth(double w)
{
    d->barWidth = w;
}
