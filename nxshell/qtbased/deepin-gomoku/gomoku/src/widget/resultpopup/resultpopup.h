// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESULTPOPUP_H
#define RESULTPOPUP_H

#include "resultinfo.h"
#include "buttonrest.h"
#include "buttonagain.h"

#include <DDialog>

DWIDGET_USE_NAMESPACE
class Resultpopup : public DWidget
{
    Q_OBJECT
public:
    Resultpopup(bool compositing, QWidget *parent = nullptr);

    void popupShow();
    void setHasWin(bool win);
    void popupClose();

public slots:
    void slotCompositingChanged(bool compositing);

signals:
    void signalGameAgain();
    void signalHaveRest();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void initUI();
    void initBackgroundPix();

private:
    bool hasWin = false; //是否赢
    bool compositingStatus = false; //是否开始特效窗口
    QPixmap winPixmap; //赢的背景图片
    QPixmap failPixmap; //输的背景图片
    Resultinfo *resultInfo = nullptr; //结果信息
    Buttonrest *buttonRest = nullptr; //休息一下按钮
    Buttonagain *buttonAgain = nullptr; //再来一局按钮
};

#endif // RESULTPOPUP_H
