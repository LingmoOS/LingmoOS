// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QColor>
#include <QDebug>
#include <QFont>
#include <QPen>
#include <QPainter>
#include <QRect>
#include <QtMath>

#include "progresscirclewidget.h"


ProgressCircleWidget::ProgressCircleWidget(QWidget *parent)
    : QWidget(parent)
    , m_animTimer(new QTimer(this))
{
    initUI();
    connect(m_animTimer, &QTimer::timeout, [this] {
        m_degree += kDegreePerSec * kRefreshInterval / 1000.;
        this->update();
    });
}

void ProgressCircleWidget::initUI()
{
    m_value = 0;
    m_animTimer->setInterval(kRefreshInterval);
}

void ProgressCircleWidget::setValue(qint64 v)
{
    if (v != m_value) {
        m_value = v;
        update();
    }
}

void ProgressCircleWidget::start() {
    m_degree = 0.;
    m_animTimer->start();
}

void ProgressCircleWidget::stop() {
    m_degree = 0.;
    m_animTimer->stop();
}

void ProgressCircleWidget::paintEvent(QPaintEvent *event)
{
    QRect rectValue;
    QPainter painter(this);
    painter.setFont(QFont("SourceHanSansSC", 42, QFont::Bold));
    painter.setRenderHint(QPainter::Antialiasing);

    // Setting center circle material
    painter.drawPixmap(4, 4, kBackgroundPixmap);

    // Setting outer arc
    // Setting coordination system to the correct position, and do rotation.
    painter.translate(kArcR, kArcR);
    painter.rotate(m_degree);
    painter.translate(-kArcR, -kArcR);

    painter.drawPixmap(0, 0, kArcPixmap);

    // Recovering the original coordination system.
    painter.translate(kArcR, kArcR);
    painter.rotate(-m_degree);
    painter.translate(-kArcR, -kArcR);

    painter.setPen(QColor("#FF001A2E"));
    // Setting different center effects for different percentage values.
    if (m_value >= 10)
        painter.drawText(48, 100, QString::number(m_value));
    else
        painter.drawText(64, 100, QString::number(m_value));

    // Draw "%"
    painter.setFont(QFont("SourceHanSansSC", 10, QFont::Normal));
    painter.drawText(114, 42, QString("%"));
}
