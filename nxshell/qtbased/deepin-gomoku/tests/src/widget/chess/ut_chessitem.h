// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_CHESSITEM_H
#define UT_CHESSITEM_H

#include "gtest/gtest.h"
#include "chessitem.h"
#include <QObject>
#include <QDebug>

class UT_ChessItem : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_ChessItem = new ChessItem(1);
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_ChessItem;
        qInfo() << "TearDown" << endl;
    }
    ChessItem *m_ChessItem;
};

#endif // UT_CHESSITEM_H
