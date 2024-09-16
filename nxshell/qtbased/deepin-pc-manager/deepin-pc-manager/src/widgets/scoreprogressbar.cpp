// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scoreprogressbar.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QPainter>
#include <QPropertyAnimation>

// 单个矩形条宽
const int SingleRectangleWidth = 60;
// 单个矩形条高
const qreal SingleRectangleHeight = 11.0;
// 整体最小宽
const qreal MiniWidth = SingleRectangleWidth * 3 + 20.0;
// 背景颜色
const QColor BackgroundColor(229, 229, 229);

ScoreProgressBar::ScoreProgressBar(QWidget *parent)
    : QWidget(parent)
    , m_valueAnimation(nullptr)
{
    setContentsMargins(0, 0, 0, 0);
    setBackgroundRole(QPalette::Window);

    m_currentValue = 0;
    m_value = 100;
    m_color = BackgroundColor;

    // 进度条数值动画
    m_valueAnimation = new QPropertyAnimation(this, "currentValue", this);
}

ScoreProgressBar::~ScoreProgressBar() { }

// 设置当前显示值
void ScoreProgressBar::setCurrentValue(int value)
{
    m_currentValue = value;
    update();
}

// 获取当前显示值
int ScoreProgressBar::currentValue() const
{
    return m_currentValue;
}

void ScoreProgressBar::setValue(int value)
{
    m_value = value;
    // 计算动画间隔
    int duration = VALUE_CHANGE_INTERVAL_MS * qAbs(m_value - m_currentValue);
    m_valueAnimation->setDuration(duration);
    // 起始值
    m_valueAnimation->setStartValue(m_currentValue);
    // 终止值
    m_valueAnimation->setEndValue(m_value);
    m_valueAnimation->start();
}

void ScoreProgressBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    setFixedSize(size());
}

QSize ScoreProgressBar::minimumSizeHint() const
{
    return QSize(MiniWidth, SingleRectangleHeight);
}

void ScoreProgressBar::paintEvent(QPaintEvent *)
{
    int scaleWidth = this->width();
    int scaleHeight = this->height();
    if (MiniWidth > scaleWidth)
        scaleWidth = MiniWidth; // 放大比例不能小于1
    if (SingleRectangleHeight > scaleHeight)
        scaleHeight = SingleRectangleHeight;

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::TextAntialiasing); // 设置渲染特性：反锯齿
    painter.scale(scaleWidth / MiniWidth, scaleHeight / SingleRectangleHeight);
    painter.save();

    // 三个圆角矩形尺寸
    QRect rect1(1, 1, SingleRectangleWidth - 1, SingleRectangleHeight - 1);
    QRect rect2(1 + SingleRectangleWidth + 10,
                1,
                SingleRectangleWidth - 1,
                SingleRectangleHeight - 1);

    // 绘制当前进度
    painter.setBrush(m_color);
    painter.setPen(Qt::NoPen);

    double percent = m_currentValue / 100.0;
    if (0.33 >= percent) {
        int width = int(SingleRectangleWidth * 3 * percent);
        QRect rect(1, 1, width, SingleRectangleHeight - 1);
        painter.drawRoundedRect(rect, 2, 2);
    } else if (0.67 >= percent) {
        painter.drawRoundedRect(rect1, 2, 2);
        int width = int(SingleRectangleWidth * 3 * (percent - 0.33) - 1);
        QRect rect(1 + SingleRectangleWidth + 10, 1, width, SingleRectangleHeight - 1);
        painter.drawRoundedRect(rect, 2, 2);
    } else {
        painter.drawRoundedRect(rect1, 2, 2);
        painter.drawRoundedRect(rect2, 2, 2);
        int width = int(SingleRectangleWidth * 3 * (percent - 0.66) - 1);
        QRect rect(1 + SingleRectangleWidth * 2 + 10 * 2, 1, width, SingleRectangleHeight - 1);
        painter.drawRoundedRect(rect, 2, 2);
    }

    painter.restore();
}
