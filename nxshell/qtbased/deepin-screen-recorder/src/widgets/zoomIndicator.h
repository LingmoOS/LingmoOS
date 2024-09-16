// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZOOMINDICATOR_H
#define ZOOMINDICATOR_H

#include "zoomIndicatorGL.h"
#include <DLabel>
#include <DWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QOpenGLWidget>

DWIDGET_USE_NAMESPACE

class ZoomIndicator : public DLabel
{
    Q_OBJECT
public:
    explicit ZoomIndicator(DWidget *parent = 0);
    ~ZoomIndicator();

    void showMagnifier(QPoint pos);
    void hideMagnifier();

    void setCursorPos(QPoint pos);
protected:
    void paintEvent(QPaintEvent *);

private:
    QRect m_globalRect;
    QRect m_centerRect;
    QBrush m_lastCenterPosBrush;

    ZoomIndicatorGL *m_zoomIndicatorGL = nullptr;
    QPoint m_cursorPos;

    /**
     * @brief 是否打开多任务视图界面。
     * 在某些hw机型上，打开多任务视图界面后，
     * 通过opengl绘制的widget，在执行move函数卡顿，因此做了规避。
     */
    bool m_isOpenWM = false;
};

#endif // MAGNIFIER_H
