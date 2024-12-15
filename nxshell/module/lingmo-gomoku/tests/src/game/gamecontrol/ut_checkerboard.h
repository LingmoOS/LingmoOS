// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_CHECKERBOARD_H
#define UT_CHECKERBOARD_H

#include "gtest/gtest.h"
#include "checkerboard.h"
#include <QDebug>

class UT_CheckerBoard : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_CheckerBoard = new Checkerboard();
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_CheckerBoard;
        qInfo() << "TearDown" << endl;
    }
    Checkerboard *m_CheckerBoard;
};

#endif // UT_CHECKERBOARD_H
