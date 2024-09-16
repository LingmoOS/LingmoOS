// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCOREBAR_H
#define SCOREBAR_H

#include "window/namespace.h"

#include <QWidget>

DEF_NAMESPACE_BEGIN
DEF_ANTIVIRUS_NAMESPACE_BEGIN

class ScoreBar : public QWidget
{
    Q_OBJECT
public:
    explicit ScoreBar(QWidget *parent = nullptr);
    ~ScoreBar();

protected:
    void paintEvent(QPaintEvent *);
    void drawCircle(QPainter *painter, int radius);
    void drawArc(QPainter *painter, int radius);
    void drawPolo(QPainter *painter, int radius);

private:
    int m_maxValue; // 最大值
    int m_minValue; // 最小值
    int m_value;

    int m_nullPosition; // 起始角度
    int m_lineWidth;    // 线条宽度

    bool m_showFree;        // 是否显示未使用进度
    bool m_showSmallCircle; // 是否显示小圆
    bool m_clockWise;       // 顺时针

    QColor m_usedColor;   // 已使用百分比颜色
    QColor m_freeColor;   // 未使用百分比颜色
    QColor m_circleColor; // 圆颜色

public:
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

public Q_SLOTS:
    // 设置目标值
    void setValue(int value);

    // 设置已使用百分比颜色
    void setUsedColor(const QColor &usedColor);
    // 设置未使用百分比颜色
    void setFreeColor(const QColor &freeColor);
    // 设置圆颜色
    void setCircleColor(const QColor &circleColor);
};

DEF_ANTIVIRUS_NAMESPACE_END
DEF_NAMESPACE_END

#endif // SCOREBAR_H
