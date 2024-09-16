// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLOSEPOPUP_H
#define CLOSEPOPUP_H

#include <DWidget>

DWIDGET_USE_NAMESPACE

class Closepopup: public DWidget
{
    Q_OBJECT
public:
    explicit Closepopup(QWidget *parent = nullptr);

signals:
    void signalCloseClicked();

    // QWidget interface
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
private:
    QPixmap buttonNormal;
    QPixmap buttonHover;
    QPixmap buttonPress;
    QPixmap currentPixmap;
};

#endif // CLOSEPOPUP_H
