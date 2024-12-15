// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PUB_H
#define PUB_H


#include <QVector>
#include <utility>
/**
 * @brief ChessState 棋子状态数组
 */
typedef QVector<QVector<int>> ChessState;

/**
 * @brief Position 定义位置数据传递形式
 */
typedef  std::pair<int, int> Position;


#endif // PUB_H
