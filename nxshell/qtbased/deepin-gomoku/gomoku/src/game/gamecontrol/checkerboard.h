// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHECKERBOARD_H
#define CHECKERBOARD_H

#include "constants.h"
#include "pub.h"

#include <QObject>

/**
 * @brief The Checkerboard class 棋盘类 管理棋盘中的数组状态
 */
class Checkerboard : public QObject
{
    Q_OBJECT
public:
    explicit Checkerboard(QObject *parent = nullptr);

    void insertChess(Chess chess);

    const ChessState & getChessState();

    void initCheckerboard();

    void resetCheckerboard();

private:
    ChessState chessState;

signals:


public slots:
};

#endif // CHECKERBOARD_H
