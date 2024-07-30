/*
    SPDX-FileCopyrightText: 1997 Michael Roth <mroth@wirlweb.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kseparator.h"

class KSeparatorPrivate
{
};

KSeparator::KSeparator(QWidget *parent, Qt::WindowFlags f)
    : QFrame(parent, f)
{
    setLineWidth(1);
    setMidLineWidth(0);
    setOrientation(Qt::Horizontal);
}

KSeparator::KSeparator(Qt::Orientation orientation, QWidget *parent, Qt::WindowFlags f)
    : QFrame(parent, f)
{
    setLineWidth(1);
    setMidLineWidth(0);
    setOrientation(orientation);
}

KSeparator::~KSeparator() = default;

void KSeparator::setOrientation(Qt::Orientation orientation)
{
    if (orientation == Qt::Vertical) {
        setFrameShape(QFrame::VLine);
        setFrameShadow(QFrame::Sunken);
        setMinimumSize(2, 0);
    } else {
        setFrameShape(QFrame::HLine);
        setFrameShadow(QFrame::Sunken);
        setMinimumSize(0, 2);
    }
    updateGeometry();
}

Qt::Orientation KSeparator::orientation() const
{
    return ((frameStyle() & VLine) == VLine) ? Qt::Vertical : Qt::Horizontal;
}

#include "moc_kseparator.cpp"
