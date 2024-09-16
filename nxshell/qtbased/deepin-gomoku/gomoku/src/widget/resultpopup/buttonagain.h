// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUTTONAGAIN_H
#define BUTTONAGAIN_H

#include <DWidget>

DWIDGET_USE_NAMESPACE

class Buttonagain : public DWidget
{
    Q_OBJECT
public:
    explicit Buttonagain(QWidget *parent = nullptr);

    void setResult(bool result);

signals:
    void signalButtonAnginClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap currentPixmap; //当前图片
    QPixmap againNormal; //再来一局正常状态
    QPixmap againHover; //再来一局hover状态
    QPixmap againPress; //再来一局press状态
    QPixmap failAgainPress; //再来一局press状态-fail
    bool mResult; //对局结果
    bool buttonPressed = false; //按钮按下标识
};

#endif // BUTTONAGAIN_H
