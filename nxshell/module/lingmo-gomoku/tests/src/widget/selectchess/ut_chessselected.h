// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_CHESSSELECTED_H
#define UT_CHESSSELECTED_H

#include "gtest/gtest.h"
#include "chessselected.h"
#include <QDebug>

class UT_ChessSelected : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_ChessSelected = new Chessselected(1);
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_ChessSelected;
        qInfo() << "TearDown" << endl;
    }
    Chessselected *m_ChessSelected;
};

#endif // UT_CHESSSELECTED_H
