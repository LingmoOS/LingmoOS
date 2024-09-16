// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARTIFICIALINTELLIGENCE_H
#define ARTIFICIALINTELLIGENCE_H

#include "constants.h"
#include "calculatescore.h"


class ArtificialIntelligence
{
public:
    ArtificialIntelligence();

    //根据当前棋局形式，返回计算出的最佳位置
    static Position getPosition(const ChessState chessState, const int color);

};

#endif // ARTIFICIALINTELLIGENCE_H
