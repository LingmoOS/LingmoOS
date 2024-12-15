/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2023 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPLITBAR_H
#define SPLITBAR_H

#include <QMouseEvent>
#include <QWidget>
#include <QPainter>
#include "window.h"
#include <QGraphicsOpacityEffect>

namespace KWin {

class SplitBar : public QWidget
{
    Q_OBJECT

public:
    SplitBar(QString);
    ~SplitBar();

    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent*e);
    void mouseReleaseEvent(QMouseEvent* e);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void paintEvent(QPaintEvent *event);

Q_SIGNALS:
    void splitbarPosChanged(QString, QPointF, Window *, bool);

public Q_SLOTS:
    void slotUpdateState(QString &, Window *);

private:
    QString                 m_screenName;
    QGraphicsOpacityEffect  *m_opacityEffect;
    QRectF                  m_screenRect;
    Window                  *m_window = nullptr;
};
}
#endif