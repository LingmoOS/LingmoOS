// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHESSSELECTED_H
#define CHESSSELECTED_H

#include <DWidget>

DWIDGET_USE_NAMESPACE

class Chessselected : public DWidget
{
public:
    Chessselected(int chessColor, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap chessWhite; //白色棋子
    QPixmap chessBlack; //黑色棋子
    int mChessColor; //棋子颜色
};

#endif // CHESSSELECTED_H
