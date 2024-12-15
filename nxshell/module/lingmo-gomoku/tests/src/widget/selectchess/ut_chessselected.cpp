// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_chessselected.h"
#include <QPaintEvent>

TEST_F(UT_ChessSelected, UT_ChessSelected_paintEvent1)
{
    QRect rect;
    QPaintEvent event(rect);
    m_ChessSelected->mChessColor = 1;
    m_ChessSelected->paintEvent(&event);
}

TEST_F(UT_ChessSelected, UT_ChessSelected_paintEvent2)
{
    QRect rect;
    QPaintEvent event(rect);
    m_ChessSelected->mChessColor = 2;
    m_ChessSelected->paintEvent(&event);
}
