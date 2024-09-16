// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chessselected.h"

#include <DHiDPIHelper>

#include <QPainter>
#include <QPainterPath>

Chessselected::Chessselected(int chessColor, QWidget *parent)
    : DWidget(parent)
    , chessWhite(DHiDPIHelper::loadNxPixmap(":/resources/white_chess.svg"))
    , chessBlack(DHiDPIHelper::loadNxPixmap(":/resources/black_chess.svg"))
    , mChessColor(chessColor)
{
    //设置大小
    setFixedSize(44, 44);
}

/**
 * @brief Chessselected::paintEvent 绘图事件
 * @param event
 */
void Chessselected::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();
    painter.setPen(Qt::NoPen);
    if (mChessColor == 1) {
        painter.drawPixmap(this->rect(), chessBlack);
    } else if (mChessColor == 2) {
        painter.drawPixmap(this->rect(), chessWhite);
    }
    painter.restore();
    DWidget::paintEvent(event);
}
