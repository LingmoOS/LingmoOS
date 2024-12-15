// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COUNTDOWNTOOLTIP_H
#define COUNTDOWNTOOLTIP_H

#include <DWidget>
#include <QTimer>

DWIDGET_USE_NAMESPACE

class CountdownTooltip : public DWidget
{
    Q_OBJECT

    static const int NUMBER_PADDING_Y;

public:
    explicit CountdownTooltip(DWidget *parent = 0);
    void start();

signals:
    void finished();

public slots:
    void update();

protected:
    void paintEvent(QPaintEvent *event);

    /**
     * 2021.6.24新增
     * @brief getTooltipBackground 先获取模糊背景图
     * @return
     */
    QPixmap getTooltipBackground();
    /**
     * 2021.6.24新增
     * @brief paintRect  画模糊背景
     * @param painter
     * @param blurPixmap  矩形框位置的模糊图形
     */
    void paintRect(QPainter &painter,QPixmap &blurPixmap);


private:
    QPixmap countdown1Img;
    QPixmap countdown2Img;
    QPixmap countdown3Img;

    int showCountdownCounter;

    QTimer *showCountdownTimer;
    QString text;


};

#endif // COUNTDOWNTOOLTIP_H
