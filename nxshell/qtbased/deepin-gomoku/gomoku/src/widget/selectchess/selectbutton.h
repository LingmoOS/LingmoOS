// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SELECTBUTTON_H
#define SELECTBUTTON_H

#include <DWidget>

#include <QPushButton>

DWIDGET_USE_NAMESPACE

class Selectbutton : public QPushButton
{
public:
    explicit Selectbutton(QWidget *parent = nullptr);

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap selectNormal; //选择按钮正常状态
    QPixmap selectHover; //选择按钮hover状态
    QPixmap selectPress; //选择按钮press状态
    QPixmap currentStatus; //目前的状态
    bool pressStatus = false; //鼠标点击状态
};

#endif // SELECTBUTTON_H
