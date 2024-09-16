// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "monitorindicator.h"

#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QX11Info>

#include <X11/extensions/shape.h>
#include <X11/Xregion.h>

MonitorIndicator::MonitorIndicator(QWidget *parent)
    : QFrame(parent)
{
    setAccessibleName("MonitorIndicator");
    setWindowFlags(Qt::SplashScreen | Qt::X11BypassWindowManagerHint);
    setStyleSheet("background-color: #2ca7f8;");
}

void MonitorIndicator::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);

    XRectangle rectangle;
    rectangle.x = 0;
    rectangle.y = 0;
    rectangle.width = static_cast<ushort>(e->size().width());
    rectangle.height = static_cast<ushort>(e->size().height());

    // need to restore the cut area, if not,cut out will be repeated.
    XShapeCombineRectangles(QX11Info::display(), winId(), ShapeBounding, 0, 0, &rectangle, 1, ShapeSet, YXBanded);

    rectangle.x = 10;
    rectangle.y = 10;
    rectangle.width = static_cast<ushort>(e->size().width()) - 20;
    rectangle.height = static_cast<ushort>(e->size().height()) - 20;

    XShapeCombineRectangles(QX11Info::display(), winId(), ShapeBounding, 0, 0, &rectangle, 1, ShapeSubtract, YXBanded);
}
