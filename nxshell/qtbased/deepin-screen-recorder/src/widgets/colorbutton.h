// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <DPushButton>
#include <QPaintEvent>
#include <DWidget>

DWIDGET_USE_NAMESPACE

class ColorButton : public DPushButton
{
    Q_OBJECT
public:
    ColorButton(QColor bgColor, DWidget *parent = 0);
    ~ColorButton();

    void setColorBtnChecked();

public slots:
    //QColor getColor();

signals:
    void updatePaintColor(QColor paintColor);

//protected:
//    void paintEvent(QPaintEvent *);

private:
    QColor m_bgColor;
};

#endif // COLORBUTTON_H
