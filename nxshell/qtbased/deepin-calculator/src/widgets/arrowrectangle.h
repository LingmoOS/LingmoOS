// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARROWRECTANGLE_H
#define ARROWRECTANGLE_H

#include <DArrowRectangle>
#include <QWidget>
DWIDGET_USE_NAMESPACE

class ArrowRectangle : public DArrowRectangle
{
    Q_OBJECT
public:
    explicit ArrowRectangle(ArrowDirection direction, FloatMode floatMode, QWidget *parent = nullptr);
    ~ArrowRectangle();
    bool eventFilter(QObject *obj, QEvent *event);
    void focusInEvent(QFocusEvent *event);
    void keyPressEvent(QKeyEvent *event);
//    void mouseMoveEvent(QMouseEvent *event);

public slots:
    void mouseMoveToClearFocus();
signals:
    void hidearrowrectangle(bool isesc = false);//判断是否为esc退出
};

#endif // ARROWRECTANGLE_H
