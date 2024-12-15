// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "black_widget.h"

#include <QPainter>

BlackWidget::BlackWidget(QWidget *parent)
    : QWidget(parent)
    , m_cursor(cursor())
{
    setObjectName(QStringLiteral("BlackWidget"));
    setAccessibleName(QStringLiteral("BlackWidget"));
}

void BlackWidget::setBlackMode(const bool isBlackMode)
{
    setVisible(isBlackMode);
#ifndef QT_DEBUG
    setCursor(isBlackMode ? Qt::BlankCursor : m_cursor);
#endif
}

void BlackWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect rect(QPoint(0, 0), QSize(size() * devicePixelRatioF()));
    painter.fillRect(rect, Qt::black);

    QWidget::paintEvent(event);
}

void BlackWidget::showEvent(QShowEvent *event)
{
    raise();
    QWidget::showEvent(event);
}
