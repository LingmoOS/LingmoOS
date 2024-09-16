// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dccslider.h"

#include <QDebug>
#include <QWheelEvent>

namespace def {
namespace widgets {

DCCSlider::DCCSlider(DCCSlider::SliderType type, QWidget *parent)
    : DSlider(Qt::Horizontal, parent)
{
    setType(type);
    DSlider::slider()->setTracking(false);
}

DCCSlider::DCCSlider(Qt::Orientation orientation, QWidget *parent)
    : DSlider(orientation, parent)
{
    DSlider::slider()->setTracking(false);
}

void DCCSlider::setType(DCCSlider::SliderType type)
{
    switch (type) {
    case Vernier:
        setProperty("handleType", "Vernier");
        break;
    case Progress:
        setProperty("handleType", "None");
        break;
    default:
        setProperty("handleType", "Normal");
        break;
    }
}

QSlider *DCCSlider::qtSlider()
{
    return DSlider::slider();
}

void DCCSlider::setRange(int min, int max)
{
    setMinimum(min);
    setMaximum(max);
}

void DCCSlider::setTickPosition(QSlider::TickPosition tick)
{
    tickPosition = tick;
}

void DCCSlider::setTickInterval(int ti)
{
    DSlider::slider()->setTickInterval(ti);
}

void DCCSlider::setSliderPosition(int Position)
{
    DSlider::slider()->setSliderPosition(Position);
}

void DCCSlider::setAnnotations(const QStringList &annotations)
{
    switch (tickPosition) {
    case QSlider::TicksLeft:
        setLeftTicks(annotations);
        break;
    case QSlider::TicksRight:
        setRightTicks(annotations);
        break;
    default:
        break;
    }
}

void DCCSlider::setOrientation(Qt::Orientation orientation)
{
    Q_UNUSED(orientation)
}

void DCCSlider::wheelEvent(QWheelEvent *e)
{
    e->ignore();
}

} // namespace widgets
} // namespace def
