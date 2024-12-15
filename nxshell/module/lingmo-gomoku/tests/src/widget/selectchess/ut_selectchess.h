// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_SELECTCHESS_H
#define UT_SELECTCHESS_H

#include <QObject>
#include <QDebug>
#include "gtest/gtest.h"
#include "selectchess.h"

class UT_SelectChess : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_SelectChess = new Selectchess(false);
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_SelectChess;
        qInfo() << "TearDown" << endl;
    }
    Selectchess *m_SelectChess;
};

#endif // UT_SELECTCHESS_H
