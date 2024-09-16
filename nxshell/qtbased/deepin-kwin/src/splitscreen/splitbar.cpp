/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2023 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "splitbar.h"
#include "splitmanage.h"
#include "wayland_server.h"
#include "workspace.h"
#include <QtX11Extras/QX11Info>
#include <X11/extensions/shape.h>
#include <QPainterPath>

namespace KWin {

SplitBar::SplitBar(QString screenName)
    : QWidget()
    , m_screenName(screenName)
{
    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowDoesNotAcceptFocus);
    if (waylandServer()) {
        setWindowFlags(Qt::FramelessWindowHint);
    }
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowTitle(screenName);

    setGeometry(0, 0, 1, 1);
    setWindowOpacity(0);
    show();
    setProperty("__kwin_splitbar", true);
}

SplitBar::~SplitBar()
{
}

void SplitBar::mousePressEvent(QMouseEvent* e)
{
}

void SplitBar::mouseMoveEvent(QMouseEvent*e)
{
    Q_EMIT splitbarPosChanged(m_screenName, e->screenPos(), m_window, false);
}

void SplitBar::mouseReleaseEvent(QMouseEvent* e)
{
    Q_EMIT splitbarPosChanged(m_screenName, QPointF(), m_window, true);
}

void SplitBar::enterEvent(QEvent *)
{
    setWindowOpacity(1);
    setCursor(Qt::SizeHorCursor);
    workspace()->setSplitBarStatus(1);
    Q_EMIT workspace()->splitBarCursorChanged();
}

void SplitBar::leaveEvent(QEvent *)
{
    workspace()->setSplitBarStatus(0);
    setWindowOpacity(0);
    setCursor(Qt::ArrowCursor);
}

void SplitBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter SplitBarPainter(this);
    QPen pen;
    pen.setColor(QColor("#BBBBBB"));
    pen.setWidth(1);
    SplitBarPainter.setRenderHint(QPainter::Antialiasing, true);
    SplitBarPainter.setBrush(QColor("#DFDFDF"));
    SplitBarPainter.setPen(pen);
    SplitBarPainter.drawRect(0, 0, width(), height());

    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, true);
    const qreal radius = 4;
    QRectF rect = QRect(6, event->rect().height() / 2 - 50, 9, 100);
    QPainterPath path;

    path.moveTo(rect.bottomRight() - QPointF(0, radius));
    path.lineTo(rect.topRight() + QPointF(0, radius));
    path.arcTo(QRectF(QPointF(rect.topRight() - QPointF(radius * 2, 0)), QSize(radius * 2, radius *2)), 0, 90);
    path.lineTo(rect.topLeft() + QPointF(radius, 0));
    path.arcTo(QRectF(QPointF(rect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
    path.lineTo(rect.bottomLeft() - QPointF(0, radius));
    path.arcTo(QRectF(QPointF(rect.bottomLeft() - QPointF(0, radius * 2)), QSize(radius * 2, radius * 2)), 180, 90);
    path.lineTo(rect.bottomLeft() + QPointF(radius, 0));
    path.arcTo(QRectF(QPointF(rect.bottomRight() - QPointF(radius * 2, radius * 2)), QSize(radius * 2, radius * 2)), 270, 90);
    painter.fillPath(path, QColor(Qt::gray));
    QWidget::paintEvent(event);
}

void SplitBar::slotUpdateState(QString &name, Window *w)
{
    if (m_screenName != name)
        return;
    m_window = w;
    if (w == nullptr) {
        setGeometry(0, 0, 1, 1);
        return;
    }

    QRectF geo = w->frameGeometry();
    if (w->quickTileMode() == int(QuickTileFlag::Left)) {
        setGeometry(geo.x() + geo.width() - 10, geo.y(), 20, geo.height());
    } else if (w->quickTileMode() == int(QuickTileFlag::Right)) {
        setGeometry(geo.x() -10, geo.y(), 20, geo.height());
    }
    show();
    update();
}

}
