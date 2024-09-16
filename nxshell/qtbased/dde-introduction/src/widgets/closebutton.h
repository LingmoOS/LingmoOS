// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLOSEBUTTON_H
#define CLOSEBUTTON_H

#include <QPushButton>
#include <QPainter>
#include <DGuiApplicationHelper>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainterPath>

DGUI_USE_NAMESPACE

//关闭按钮类
class CloseButton : public QPushButton
{
    Q_OBJECT

public:
    explicit CloseButton(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    bool beFocused;

signals:
    void closeMainWindow();
};

#endif // CLOSEBUTTON_H
