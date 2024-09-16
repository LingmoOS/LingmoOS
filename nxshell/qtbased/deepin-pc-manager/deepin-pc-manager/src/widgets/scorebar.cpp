// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scorebar.h"

#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE

ScoreBar::ScoreBar(QWidget *parent)
    : QWidget(parent)
{
    m_minValue = 0;
    m_maxValue = 100;
    m_value = 50;

    m_nullPosition = 90;
    m_lineWidth = 7;

    m_showFree = false;
    m_showSmallCircle = false;
    m_clockWise = true;

    m_usedColor = QColor(100, 184, 255);
    m_freeColor = QColor(100, 100, 100);
    m_circleColor = QColor(70, 70, 70);

    QHBoxLayout *centralLayout = new QHBoxLayout;
    centralLayout->setAlignment(Qt::AlignCenter);
    setLayout(centralLayout);
}

ScoreBar::~ScoreBar() { }

void ScoreBar::paintEvent(QPaintEvent *)
{
    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);
    int radius = 99;

    // 绘制准备
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::TextAntialiasing); // 设置渲染特性：反锯齿
    painter.translate(width / 2.0, height / 2.0);
    painter.scale(side / 200.0, side / 200.0);

    // 绘制中心圆
    drawCircle(&painter, radius);

    // 绘制圆弧边框
    drawArc(&painter, radius);

    // 绘制水池进度
    drawPolo(&painter, radius);
}

void ScoreBar::drawCircle(QPainter *painter, int radius)
{
    radius = radius - m_lineWidth * 3 / 2;

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_circleColor);
    painter->drawEllipse(-radius, -radius, radius * 2, radius * 2); // 原点为左上角
    painter->restore();
}

void ScoreBar::drawArc(QPainter *painter, int radius)
{
    radius = radius - m_lineWidth;
    painter->save();
    painter->setBrush(Qt::NoBrush);

    QPen pen = painter->pen();
    pen.setWidthF(m_lineWidth);

    // 设置线条风格
    pen.setCapStyle(Qt::RoundCap);

    double arcLength = 360.0 / (m_maxValue - m_minValue) * m_value; // °
    QRect rect(-radius, -radius, radius * 2, radius * 2);

    // 圆弧进度框增长方向（顺时针或逆时针）
    if (!m_clockWise)
        arcLength = -arcLength;

    // 绘制剩余圆弧进度
    if (m_showFree) {
        pen.setColor(m_freeColor);
        painter->setPen(pen);
        painter->drawArc(rect, (m_nullPosition - arcLength) * 16, -(360 - arcLength) * 16);
    }

    // 绘制当前进度
    pen.setColor(m_usedColor);
    painter->setPen(pen);
    painter->drawArc(rect, m_nullPosition * 16, -arcLength * 16);

    // 绘制当前进度圆弧两端小圆
    if (m_showSmallCircle) {
        int offset = radius - m_lineWidth + 1;
        radius = m_lineWidth / 2 - 1;
        painter->rotate(-m_nullPosition - 90);

        QRect circleRect(-radius, radius + offset, radius * 2, radius * 2);
        painter->rotate(arcLength);
        painter->drawEllipse(circleRect);
    }

    painter->restore();
}

void ScoreBar::drawPolo(QPainter *painter, int radius)
{
    radius = radius - m_lineWidth * 5 / 2;
    painter->save();

    // 计算当前值所占百分比对应高度
    double poloHeight = (double)radius / (m_maxValue - m_minValue) * m_value;

    // 大圆路径
    QPainterPath bigPath;
    bigPath.addEllipse(-radius, -radius, radius * 2, radius * 2);

    // 底部水池所占矩形区域
    QPainterPath smallPath;
    smallPath.addRect(-radius, radius - poloHeight * 2, radius * 2, poloHeight * 2);

    // 提取两个路径重合部分
    QPainterPath path;
    path = bigPath.intersected(smallPath);

    painter->setPen(m_usedColor);
    painter->setBrush(m_usedColor);
    painter->drawPath(path);

    painter->restore();
}

QSize ScoreBar::sizeHint() const
{
    return QSize(200, 200);
}

QSize ScoreBar::minimumSizeHint() const
{
    return QSize(10, 10);
}

void ScoreBar::setValue(int value)
{
    // 值小于最小值或者值大于最大值或者值和当前值一致则无需处理
    if (value < m_minValue || value > m_maxValue || value == m_value) {
        return;
    }

    m_value = value;
    update();
}

void ScoreBar::setUsedColor(const QColor &usedColor)
{
    if (m_usedColor != usedColor) {
        m_usedColor = usedColor;
        update();
    }
}

void ScoreBar::setFreeColor(const QColor &freeColor)
{
    if (m_freeColor != freeColor) {
        m_freeColor = freeColor;
        update();
    }
}

void ScoreBar::setCircleColor(const QColor &circleColor)
{
    if (m_circleColor != circleColor) {
        m_circleColor = circleColor;
        update();
    }
}
