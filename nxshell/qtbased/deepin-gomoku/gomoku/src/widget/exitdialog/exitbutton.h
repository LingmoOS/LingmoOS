// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXITBUTTON_H
#define EXITBUTTON_H

#include <QWidget>
#include <QPixmap>
#include <DWidget>

DWIDGET_USE_NAMESPACE
class ExitButton : public QWidget
{
Q_OBJECT
public:
     explicit ExitButton(QWidget *parent = nullptr);

private:
     void mouseReleaseEvent(QMouseEvent *event);
     void enterEvent(QEvent *event);
     void leaveEvent(QEvent *event);
     void paintEvent(QPaintEvent *event);
     void mousePressEvent(QMouseEvent *event);

private:
     QPixmap buttonNormal;
     QPixmap buttonHover;
     QPixmap buttonPress;
     QPixmap currentPixmap;
     bool buttonPressed= false; //按钮按下标识

signals:
     void signalButtonOKClicked();
};

#endif // EXITBUTTON_H
