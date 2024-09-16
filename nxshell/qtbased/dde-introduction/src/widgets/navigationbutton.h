// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NAVIGATIONBUTTON_H
#define NAVIGATIONBUTTON_H

#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QPainter>
#include <QKeyEvent>

#include <DGuiApplicationHelper>
#include <DPalette>
#include <DLabel>
#include <DPushButton>
#include <DWidget>
#include <QPainterPath>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

//导航按钮类
class NavigationButton : public DPushButton
{
    Q_OBJECT
public:
    explicit NavigationButton(QString text, DWidget* parent = nullptr);

    void initButton();

    bool needFrame;//需要画框标志位

protected:
    // QSize setHint();
    void paintEvent(QPaintEvent *event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void mousePressEvent(QMouseEvent *event);

signals:
    void widthChanged(int);

private:
    DLabel* m_label;
    bool isEnter;
};

#endif  // NAVIGATIONBUTTON_H
